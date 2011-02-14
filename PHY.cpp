/*
 * BCM5722D
 * Copyright (C) 2010 Adlan Razalan <dev at adlan dot name dot my>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Common.h"
#include "PHY.h"

#pragma mark -
#pragma mark Initialization/Reset/Fixes

bool BCM5722D::probePHY()
{
  UInt16 id1;
  UInt16 id2;

  // Set default value
  phyFlags            = 0;
  autoNegotiate       = true;
  media.speed         = kLinkSpeedNone;
  media.duplex        = kLinkDuplexNone;
  media.flowControl   = kFlowControlSymmetric;

  readMII(PHY_ID1, &id1);
  readMII(PHY_ID2, &id2);

  /*
   * id1 = [OUI(3-18)]
   * id2 = [OUI(19-24)][Model(4-9)][Rev(0-3)]
   */
  phyID = ((id1 & 0xFFFF) << 10);
  phyID |= ((id2 & 0xFC00) << 16);
  phyID |= ((id2 & 0x03FF) << 0);

  if (GET_ASICREV(asicRevision) != ASICREV_C) {
    phyFlags |= PHYFLAG_ETHERNET_WIRESPEED;
  } else {
    phyFlags |= PHYFLAG_FAST_ETHERNET;
  }

  if (deviceID == DEVICEID_BCM5755M) {
    phyFlags |= PHYFLAG_BUG_ADJUST_TRIM;
  }

  phyFlags |= PHYFLAG_BUG_JITTER;

  return true;
} // probePHY()


bool BCM5722D::setupPHY(bool reset)
{
  acknowledgeInterrupt();

  if (reset) {
    resetPHY();
  }

  UInt16     miiStatus;
  UInt16     miiControl;
  LinkSpeed  currentSpeed   = kLinkSpeedNone;
  LinkDuplex currentDuplex  = kLinkDuplexNone;

  readMII(PHY_MIISTAT, &miiStatus);
  readMII(PHY_MIISTAT, &miiStatus);

  if (miiStatus & PHY_MIISTAT_LINKSTAT) {

    UInt16 auxStat;
    UInt16 local   = 0;
    UInt16 partner = 0;
    int    i;

    for (i = 0; i < 2000; i++) {

      readMII(PHY_AUXSTAT, &auxStat);

      if (auxStat & PHY_AUXSTAT_LINKSTAT) {
        break;
      }

      IODelay(10);

    }

    DebugLog("Auxillary status: %X", auxStat);

    resolveOperatingSpeedAndLinkDuplex(auxStat,
                                       &currentSpeed,
                                       &currentDuplex);

    media.speed = currentSpeed;
    media.duplex = currentDuplex;

    readMII(PHY_MIICTL, &miiControl);

    if (autoNegotiate) {

        readMII(PHY_AUTONEGADVERT, &local);
        readMII(PHY_AUTONEGPARTNER, &partner);

    }

    if (currentDuplex == kLinkDuplexFull) {
      setupFlowControl(local, partner);
    } else {
      media.flowControl = kFlowControlDisabled;
    }
  }

  media.state = ((miiStatus & PHY_MIISTAT_LINKSTAT)
                 ? kLinkStateUp : kLinkStateDown);

  configureMAC(currentSpeed, currentDuplex);

  writeCSR(EMAC_EVENT, EMAC_EVENT_LNKSTATECHGD);
  IODelay(40);

  return true;
} // setupPHY()


void BCM5722D::acknowledgeInterrupt()
{
  UInt16 miiStatus;

  writeCSR(EMAC_EVENT, 0);
  writeCSR(EMAC_STATUS, (EMAC_STATUS_CONFIGCHGD |
                         EMAC_STATUS_SYNCCHGD |
                         EMAC_STATUS_LNKSTATECHGD |
                         EMAC_STATUS_MICOMPLETION
                         ));
  IODelay(40);

  writeMII(PHY_AUXCTL, 0x02);

  readMII(PHY_IRQSTAT, &miiStatus);
  readMII(PHY_IRQSTAT, &miiStatus);

  if (!(phyFlags & PHYFLAG_FAST_ETHERNET)) {
    writeMII(PHY_IRQMASK, ~0);
  }
} // acknowledgeInterrupt()


void BCM5722D::configureMAC(LinkSpeed speed, LinkDuplex duplex) {

  macMode &= ~EMAC_MODE_PORTMODE_MASK;

  if (speed == kLinkSpeed10 || speed == kLinkSpeed100) {
    macMode |= EMAC_MODE_PORTMODE_MII;
  } else if (speed == kLinkSpeed1000) {
    macMode |= EMAC_MODE_PORTMODE_GMII;
  } else {

    if (phyFlags & PHYFLAG_FAST_ETHERNET) {
      macMode |= EMAC_MODE_PORTMODE_MII;
    } else {
      macMode |= EMAC_MODE_PORTMODE_GMII;
    }
  }

  macMode &= ~EMAC_MODE_HALFDUPLEX;

  if (duplex == kLinkDuplexHalf){
    macMode |= EMAC_MODE_HALFDUPLEX;
  }

  writeCSR(EMAC_MODE, macMode);
  IODelay(40);

  if (speed == kLinkSpeed1000 && duplex == kLinkDuplexHalf) {
    writeCSR(EMAC_TXLEN, 0x26FF);
  } else {
    writeCSR(EMAC_TXLEN, 0x2620);
  }
} // configureMAC()


IOReturn BCM5722D::resetPHY()
{
  IOReturn status;
  UInt16 mii;
  int i;

  if (GET_ASICREV(asicRevision) == ASICREV_C) {

    clearBit(GCR_MISCCFG, GCR_MISCCFG_PHYIDDQ);
    IODelay(40);

  }

  status = readMII(PHY_MIISTAT, &mii);
  status |= readMII(PHY_MIISTAT, &mii);

  if (status != kIOReturnSuccess) {
    return status;
  }

  status = writeMII(PHY_MIICTL, PHY_MIICTL_RESET);

  if (status != kIOReturnSuccess) {
    return status;
  }

  for (i = 0; i < 5000; i++) {

    if (readMII(PHY_MIICTL, &mii) == kIOReturnSuccess &&
        (mii & PHY_MIICTL_RESET) == 0) {

      IODelay(40);
      break;

    }

    IODelay(10);

  }

  if (i == 5000) {
    return kIOReturnBusy;
  }

  if (phyFlags & PHYFLAG_BUG_JITTER) {
    fixJitterBug();
  }

  if (phyFlags & PHYFLAG_BUG_ADJUST_TRIM) {
    fixAdjustTrim();
  }

  if (GET_ASICREV(asicRevision) == ASICREV_C) {
    writeMII(PHY5906_PTEST, 0x12);
  }

  enableAutoMDIX(true);
  enableEthernetAtWirespeed();

  return kIOReturnSuccess;
} // resetPHY()


void BCM5722D::enableLoopback()
{
  int i;
  UInt16 miiStatus;

  writeMII(PHY_MIICTL, PHY_MIICTL_LOOPBACK);

  for (i = 0; i < 1500; i++) {

    if (!(readMII(PHY_MIISTAT, &miiStatus) & PHY_MIISTAT_LINKSTAT)) {
      break;
    }

    IODelay(10);

  }
} // enableLoopback()


// #tg3
void BCM5722D::fixJitterBug()
{
  writeMII(PHY_AUXCTL, 0x0C00);
  writeMII(PHY_DSPADDR, 0x000A);
  writeMII(PHY_DSPRWPORT, 0x010B);
  writeMII(PHY_AUXCTL, 0x0400);
} // fixJitterBug()


// #tg3
void BCM5722D::fixAdjustTrim()
{
  writeMII(PHY_AUXCTL, 0x0C00);
  writeMII(PHY_DSPADDR, 0x000A);
  writeMII(PHY_DSPRWPORT, 0x110B);
  writeMII(PHY_TEST1, 0x0014);
  writeMII(PHY_AUXCTL, 0x0400);
} // fixAdjustTrim()


#pragma mark -
#pragma mark Medium


void BCM5722D::addNetworkMedium(UInt32 type, UInt32 speed, UInt32 index)
{
  IONetworkMedium *medium;

  medium = IONetworkMedium::medium(type, speed, 0, index);

  if (medium) {

    IONetworkMedium::addMedium(mediumDict, medium);
    medium->release();

  }
} // addNetworkMedium()


void BCM5722D::probeMediaCapability()
{
  int count;

  if (phyFlags & PHYFLAG_FAST_ETHERNET) {
    count = kMediumTypeCount - 2;
  } else {
    count = kMediumTypeCount;
  }

  mediumDict = OSDictionary::withCapacity(count);

  addNetworkMedium(kIOMediumEthernetAuto, 0, kMediumTypeIndexAuto);

  addNetworkMedium(kIOMediumEthernet10BaseT | kIOMediumOptionHalfDuplex,
                   10 * MBit, kMediumTypeIndex10HD);
  addNetworkMedium(kIOMediumEthernet10BaseT | kIOMediumOptionFullDuplex,
                   10 * MBit, kMediumTypeIndex10FD);

  addNetworkMedium(kIOMediumEthernet100BaseTX | kIOMediumOptionHalfDuplex,
                   100 * MBit, kMediumTypeIndex100HD);
  addNetworkMedium(kIOMediumEthernet100BaseTX | kIOMediumOptionFullDuplex,
                   100 * MBit, kMediumTypeIndex100FD);

  if (!(phyFlags & PHYFLAG_FAST_ETHERNET)) {

    addNetworkMedium(kIOMediumEthernet1000BaseT | kIOMediumOptionHalfDuplex,
                     1000 * MBit, kMediumTypeIndex1000HD);
    addNetworkMedium(kIOMediumEthernet1000BaseT | kIOMediumOptionFullDuplex,
                     1000 * MBit, kMediumTypeIndex1000FD);

  }
} // setupMediaCapability()


IOReturn BCM5722D::setMedium(const IONetworkMedium *medium)
{
  UInt32 type;
  LinkSpeed changeSpeed = kLinkSpeedNone;
  LinkDuplex changeDuplex = kLinkDuplexNone;

  type = medium->getType();

  changeDuplex = ((type & kIOMediumOptionFullDuplex)
                  ? kLinkDuplexFull : kLinkDuplexHalf);

  switch (IOMediumGetSubType(type) + kIOMediumEthernet) {

    case kIOMediumEthernetAuto:
      DebugLog("Change medium: kIOMediumEthernetAuto");
      changeSpeed = kLinkSpeedNegotiate;
      changeDuplex = kLinkDuplexNegotiate;
      break;

    case kIOMediumEthernet10BaseT:
      DebugLog("Change medium: kIOMediumEthernet10BaseT");
      changeSpeed = kLinkSpeed10;
      break;

    case kIOMediumEthernet100BaseTX:
      DebugLog("Change medium: kIOMediumEthernet100BaseT");
      changeSpeed = kLinkSpeed100;
      break;

    case kIOMediumEthernet1000BaseT:
      DebugLog("Change medium: kIOMediumEthernet1000BaseT");
      changeSpeed = kLinkSpeed1000;
      break;

  }

  DebugLog("Change medium: %s",
           (changeDuplex == kLinkDuplexFull ? "kLinkDuplexFull" :
            (changeDuplex == kLinkDuplexHalf ? "kLinkDuplexHalf" :
             "kLinkDuplexNegotiate")));

  if (autoNegotiate) {
    startAutoNegotiation(changeSpeed, changeDuplex);
  } else {
    forceLinkSpeedDuplex(changeSpeed, changeDuplex);
  }

  currentMediumIndex = medium->getIndex();

  return kIOReturnSuccess;
} // setMedium()


#pragma mark -
#pragma mark Link


inline UInt16 BCM5722D::resolvePauseAdvertisement(FlowControl flowControl)
{
  UInt16 advertise;

  switch (flowControl) {

    case kFlowControlRx:

      advertise = (PHY_AUTONEGADVERT_PAUSECAP |
                   PHY_AUTONEGADVERT_ASYMPAUSE
                   );

      break;

    case kFlowControlTx:

      advertise = PHY_AUTONEGADVERT_ASYMPAUSE;

      break;

    case kFlowControlSymmetric:

      advertise = PHY_AUTONEGADVERT_PAUSECAP;

      break;

  }

  return advertise;
} // resolvePauseAdvertisement()


void BCM5722D::setupFlowControl(UInt16 local, UInt16 partner)
{
  FlowControl flowControl = kFlowControlDisabled;

  if (media.duplex != kLinkDuplexFull) {
    goto setupFlowControlDone;
  }

  if (autoNegotiate) {

    if (local & partner & PHY_AUTONEGADVERT_PAUSECAP) {
      flowControl = kFlowControlSymmetric;
    } else if (local & partner & PHY_AUTONEGADVERT_ASYMPAUSE) {

      if (local & PHY_AUTONEGADVERT_PAUSECAP) {
        flowControl = kFlowControlRx;
      } else if (partner & PHY_AUTONEGADVERT_PAUSECAP) {
        flowControl = kFlowControlTx;
      }
    }
  } else {
    flowControl = media.flowControl;
  }

  DebugLog("flowControl: %d", flowControl);

setupFlowControlDone:

  txMode &= ~EMAC_TXMODE_FLOWCTL;
  rxMode &= ~EMAC_RXMODE_FLOWCTL;

  switch (flowControl) {
    case kFlowControlTx:
      txMode |= EMAC_TXMODE_FLOWCTL;
      break;
    case kFlowControlRx:
      rxMode |= EMAC_RXMODE_FLOWCTL;
      break;
    case kFlowControlSymmetric:
      txMode |= EMAC_TXMODE_FLOWCTL;
      rxMode |= EMAC_RXMODE_FLOWCTL;
      break;
  }

  writeCSR(EMAC_RXMODE, rxMode);
  writeCSR(EMAC_TXMODE, txMode);

  media.flowControl = flowControl;
} // setupFlowControl()


void BCM5722D::configureLinkAdvertisement(LinkSpeed linkSpeed,
                                          LinkDuplex linkDuplex)
{
  UInt16 advertiseFe = 0;
  UInt16 advertiseGe = 0;

  advertiseFe = PHY_AUTONEGADVERT_802_3;
  advertiseFe |= resolvePauseAdvertisement(media.flowControl);

  switch (linkSpeed) {

    case kLinkSpeedNegotiate:

      advertiseFe |= (PHY_AUTONEGADVERT_10FD |
                      PHY_AUTONEGADVERT_10HD |
                      PHY_AUTONEGADVERT_100FD |
                      PHY_AUTONEGADVERT_100HD
                      );


      if (!(phyFlags & PHYFLAG_FAST_ETHERNET)) {
        advertiseGe = (PHY_1000BASETCTL_ADVERTHD |
                       PHY_1000BASETCTL_ADVERTFD
                       );
      }

      break;

    case kLinkSpeed1000:

      if (linkDuplex == kLinkDuplexFull) {
        advertiseGe = PHY_1000BASETCTL_ADVERTFD;
      } else if (linkDuplex == kLinkDuplexHalf) {
        advertiseGe = PHY_1000BASETCTL_ADVERTHD;
      } else {
        advertiseGe = (PHY_1000BASETCTL_ADVERTFD |
                       PHY_1000BASETCTL_ADVERTHD
                       );
      }

      break;

    case kLinkSpeed100:

      if (linkDuplex == kLinkDuplexFull) {
        advertiseFe |= PHY_AUTONEGADVERT_100FD;
      } else if (linkDuplex == kLinkDuplexHalf) {
        advertiseFe |= PHY_AUTONEGADVERT_100HD;
      } else {
        advertiseFe |= (PHY_AUTONEGADVERT_100FD |
                        PHY_AUTONEGADVERT_100HD
                        );
      }

      break;

    case kLinkSpeed10:

      if (linkDuplex == kLinkDuplexFull) {
        advertiseFe |= PHY_AUTONEGADVERT_10FD;
      } else if (linkDuplex == kLinkDuplexHalf) {
        advertiseFe |= PHY_AUTONEGADVERT_10HD;
      } else {
        advertiseFe |= (PHY_AUTONEGADVERT_10FD |
                        PHY_AUTONEGADVERT_10HD
                        );
      }

      break;

  }

  DebugLog("advertiseFe: %X", advertiseFe);
  DebugLog("advertiseGe: %X", advertiseGe);

  writeMII(PHY_AUTONEGADVERT, advertiseFe);

  if (!(phyFlags & PHYFLAG_FAST_ETHERNET)) {
    writeMII(PHY_1000BASETCTL, advertiseGe);
  }
} // configureLinkAdvertisement()


bool BCM5722D::startAutoNegotiation(LinkSpeed changeSpeed,
                                    LinkDuplex changeDuplex)
{
  if (changeSpeed != kLinkSpeedNegotiate) {
    Log("Advertising with limited capability, speed: %d MBps, %s duplex",
        changeSpeed, (changeDuplex == kLinkDuplexFull ? "full" : "half"));
  }

  configureLinkAdvertisement(changeSpeed, changeDuplex);

  UInt16 adv;
  readMII(PHY_AUTONEGADVERT, &adv);
  DebugLog("Adv reg: %X", adv);

  writeMII(PHY_MIICTL, (PHY_MIICTL_AUTONEGENABLE |
                        PHY_MIICTL_RESTARTAUTONEG));

  return true;
} // startAutoNegotiation()


bool BCM5722D::forceLinkSpeedDuplex(LinkSpeed changeSpeed,
                                    LinkDuplex changeDuplex)
{
  Log("Forcing link speed: %d MBps, %s duplex",
      changeSpeed, (changeDuplex == kLinkDuplexFull ? "full" : "half"));

  UInt16 miiCtl;

  switch (changeSpeed) {

    case kLinkSpeed10:
      miiCtl = PHY_MIICTL_SPEED_10;

      break;

    case kLinkSpeed100:
      miiCtl = PHY_MIICTL_SPEED_100;

      break;

    case kLinkSpeed1000:
      miiCtl = (PHY_MIICTL_SPEED_1000 |
                PHY_MIICTL_AUTONEGENABLE |
                PHY_MIICTL_RESTARTAUTONEG
                );

      configureLinkAdvertisement(changeSpeed, changeDuplex);

      autoNegotiate = true;

      break;

  }

  if (changeDuplex == kLinkDuplexFull) {
    miiCtl |= PHY_MIICTL_DUPLEX_FULL;
  }

  enableLoopback();

  writeMII(PHY_MIICTL, miiCtl);

  configureMAC(changeSpeed, changeDuplex);

  return true;
} // forceLinkSpeedDuplex()


void BCM5722D::resolveOperatingSpeedAndLinkDuplex(UInt16 status,
                                                  LinkSpeed *speed,
                                                  LinkDuplex *duplex)
{
  switch (status & PHY_AUXSTAT_SPDDPLXMASK) {

    case PHY_AUXSTAT_10HD:

      *speed = kLinkSpeed10;
      *duplex = kLinkDuplexHalf;

      break;

    case PHY_AUXSTAT_10FD:

      *speed = kLinkSpeed10;
      *duplex = kLinkDuplexFull;

      break;

    case PHY_AUXSTAT_100HD:

      *speed = kLinkSpeed100;
      *duplex = kLinkDuplexHalf;

      break;

    case PHY_AUXSTAT_100FD:

      *speed = kLinkSpeed100;
      *duplex = kLinkDuplexFull;

      break;

    case PHY_AUXSTAT_1000HD:

      *speed = kLinkSpeed1000;
      *duplex = kLinkDuplexHalf;

      break;

    case PHY_AUXSTAT_1000FD:

      *speed = kLinkSpeed1000;
      *duplex = kLinkDuplexFull;

      break;

    default:

      *speed = kLinkSpeedNone;
      *duplex = kLinkDuplexNone;

      break;

  }
} // resolveOperatingSpeedAndLinkDuplex()


// #tg3
void BCM5722D::enableAutoMDIX(bool active)
{
  UInt16 phy;

  if (phyFlags & PHYFLAG_FAST_ETHERNET) {

    UInt16 ephy;

    if (readMII(PHY5906_TEST, &ephy) == kIOReturnSuccess) {

      writeMII(PHY5906_TEST, ephy | PHY5906_TEST_SHADOW);

      if (readMII(PHY5906_MISCCTL, &phy) == kIOReturnSuccess) {

        if (active) {
          phy |= PHY5906_MISCCTL_AUTOMDIX;
        } else {
          phy &= ~PHY5906_MISCCTL_AUTOMDIX;
        }

        writeMII(PHY5906_TEST, ephy);

      }

      writeMII(PHY5906_TEST, ephy);

    }
  } else {

    phy = (PHY_AUXCTL_MISCCTL | PHY_AUXCTL_MISCCTL_READ);

    if (writeMII(PHY_AUXCTL, phy) == kIOReturnSuccess &&
        readMII(PHY_AUXCTL, &phy) == kIOReturnSuccess) {

      if (active) {
        phy |= PHY_AUXCTL_MISCCTL_AUTOMDIX;
      } else {
        phy &= ~PHY_AUXCTL_MISCCTL_AUTOMDIX;
      }

      phy |= PHY_AUXCTL_MISCCTL_WRITE;
      writeMII(PHY_AUXCTL, phy);

    }
  }
} // enableAutoMDIX()


// #tg3 - tg3_phy_set_wirespeed
void BCM5722D::enableEthernetAtWirespeed()
{
  IOReturn status;
  UInt16 value;

  if (!(phyFlags & PHYFLAG_ETHERNET_WIRESPEED)) {
    return;
  }

  status = writeMII(PHY_AUXCTL, 0x7007);
  status &= readMII(PHY_AUXCTL, &value);

  if (status == kIOReturnSuccess) {
    writeMII(PHY_AUXCTL, (value | (1 << 15) | (1 << 4)));
  }
} // enableEthernetAtWirespeed()


inline void BCM5722D::reportLinkStatus()
{
  if (media.state == kLinkStateUp) {
    Log("Link up: %d MBps, %s duplex. Flow control: %s",
        media.speed,
        (media.duplex == kLinkDuplexFull ? "full" : "half"),
        (media.flowControl == kFlowControlDisabled ? "disabled" :
         (media.flowControl == kFlowControlSymmetric ? "symmetric" :
          (media.flowControl == kFlowControlTx ? "Tx" : "Rx"))));
  } else {
    Log("Link down");
  }
} // reportLinkStatus()


#pragma mark -
#pragma mark Interrupt handling


void BCM5722D::serviceLinkInterrupt()
{
  statusBlock->statusWord &= ~STATUS_WORD_LNKCHGD;

  setupPHY();

  if (media.state == kLinkStateUp) {
    setLinkStatus(kIONetworkLinkValid | kIONetworkLinkActive,
                  getSelectedMedium(), media.speed * MBit);
  } else {
    setLinkStatus(kIONetworkLinkValid, 0);
  }

  reportLinkStatus();
} // serviceLinkInterrupt()


#pragma mark -
#pragma mark Reader/Writer methods


IOReturn BCM5722D::writeMII(UInt8 reg, UInt16 value)
{
  UInt32 miMode;
  UInt32 miComm;
  int i;

  miMode = readCSR(EMAC_MIMODE);

  if (miMode & EMAC_MIMODE_PORTPOLL) {

    writeCSR(EMAC_MIMODE, miMode & ~EMAC_MIMODE_PORTPOLL);
    IODelay(40);

  }

  writeCSR(EMAC_MICOMM, (EMAC_MICOMM_CMD_WRITE |
                         EMAC_MICOMM_START |
                         EMAC_MICOMM_PHY(1) |
                         EMAC_MICOMM_REG(reg) |
                         value
                         ));

  for (i = 0; i < 5000; i++) {

    IODelay(10);

    if (!(readCSR(EMAC_MICOMM) & EMAC_MICOMM_BUSY)) {

      IODelay(5);
      miComm = readCSR(EMAC_MICOMM);
      break;

    }

  }

  if (miMode & EMAC_MIMODE_PORTPOLL) {

    writeCSR(EMAC_MIMODE, miMode);
    IODelay(40);

  }

  if (i == 5000) {
    return kIOReturnBusy;
  }

  return kIOReturnSuccess;
} // writeMII()


IOReturn BCM5722D::readMII(UInt8 reg, UInt16 *value)
{
  UInt32 miMode;
  UInt32 miComm;
  int i;

  miMode = readCSR(EMAC_MIMODE);

  if (miMode & EMAC_MIMODE_PORTPOLL) {

    writeCSR(EMAC_MIMODE, miMode & ~EMAC_MIMODE_PORTPOLL);
    IODelay(40);

  }

  writeCSR(EMAC_MICOMM, (EMAC_MICOMM_CMD_READ |
                         EMAC_MICOMM_START |
                         EMAC_MICOMM_PHY(1) |
                         EMAC_MICOMM_REG(reg)
                         ));

  for (i = 0; i < 5000; i++) {

    IODelay(10);

    if (!(readCSR(EMAC_MICOMM) & EMAC_MICOMM_BUSY)) {

      IODelay(5);
      miComm = readCSR(EMAC_MICOMM);
      break;

    }

  }

  if (miMode & EMAC_MIMODE_PORTPOLL) {

    writeCSR(EMAC_MIMODE, miMode);
    IODelay(40);

  }

  if (i == 5000) {

    *value = 0;

    return kIOReturnBusy;

  }

  *value = miComm & EMAC_MICOMM_DATA;

  return kIOReturnSuccess;
} // readMII()

