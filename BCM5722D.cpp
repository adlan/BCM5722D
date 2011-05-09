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

#define super IOEthernetController

OSDefineMetaClassAndStructors(BCM5722D, IOEthernetController);

static const struct SupportedDevice
{
  UInt16 id;
  const char *model;
} devices[] = {
  { DEVICEID_BCM5722,  "BCM5722 NetXtreme Server Gigabit Ethernet" },
  { DEVICEID_BCM5754,  "BCM5754 NetXtreme Gigabit Ethernet" },
  { DEVICEID_BCM5754M, "BCM5754M NetXtreme Gigabit Ethernet" },
  { DEVICEID_BCM5755,  "BCM5755 NetXtreme Gigabit Ethernet" },
  { DEVICEID_BCM5755M, "BCM5755M NetXtreme Gigabit Ethernet" },
  { DEVICEID_BCM5787,  "BCM5787 NetLink (TM) Gigabit Ethernet" },
  { DEVICEID_BCM5787M, "BCM5787M NetLink (TM) Gigabit Ethernet" },
  { DEVICEID_BCM5906,  "BCM5906 NetLink (TM) Fast Ethernet" },
  { DEVICEID_BCM5906M, "BCM5906M NetLink (TM) Fast Ethernet" },
  { 0, NULL }
};


#pragma mark -
#pragma mark - IOService methods


bool BCM5722D::start(IOService *provider)
{
  IOLog("BCM5722D (Build date/time: %s %s)\n", __DATE__, __TIME__);

  bool success = false;
  bool started = false;

  do {

    if (!super::start(provider)) {
      break;
    }

    started = true;

    pciNub = OSDynamicCast(IOPCIDevice, provider);

    if (!pciNub) {
      break;
    }

    pciNub->retain();

    if (!pciNub->open(this)) {
      break;
    }

    csrMap = pciNub->mapDeviceMemoryWithRegister(kIOPCIConfigBaseAddress0);

    if (!csrMap) {
      break;
    }

    csrBase = (volatile void *)csrMap->getVirtualAddress();

    if (!setupDriver(provider)) {
      break;
    }

    if (!allocateDriverMemory()) {
      break;
    }

    initializePCIConfig();

    prepareDriver();

    if (!probePHY()) {
      break;
    }

    probeMediaCapability();

    if(!publishMediumDictionary(mediumDict)) {

      Log("Failed to publish medium dictionary");
      break;

    }

    success = true;

  } while (false);

  if (pciNub) {
    pciNub->close(this);
  }

  do {

    if (!success) {
      break;
    }

    success = false;

    if (!attachInterface((IONetworkInterface **)&netIface, false)) {
      break;
    }

    netIface->registerService();

    success = true;

  } while (false);

  if (started && !success) {
    super::stop(provider);
  } else {
    Log("Loaded successfully");
  }

  return success;
} // start()


void BCM5722D::stop(IOService *provider)
{
  if (interruptSource) {

    interruptSource->disable();
    workLoop->removeEventSource(interruptSource);

  }

  if (timerSource) {

    timerSource->cancelTimeout();
    workLoop->removeEventSource(timerSource);

  }

  super::stop(provider);
} // stop()


void BCM5722D::free(void)
{
#define RELEASE(x) do { if (x) { (x)->release(); (x) = 0; } } while(0)

  freeDescriptorMemory(&txMD);
  freeDescriptorMemory(&rxMD);
  freeDescriptorMemory(&rxReturnMD);

  RELEASE(txCursor);
  RELEASE(rxCursor);
  RELEASE(mediumDict);
  RELEASE(interruptSource);
  RELEASE(timerSource);
  RELEASE(workLoop);
  RELEASE(netIface);
  RELEASE(csrMap);
  RELEASE(pciNub);

  super::free();
} // free()


#pragma mark -
#pragma mark IOEthernetController methods


IOReturn BCM5722D::getHardwareAddress(IOEthernetAddress *address)
{
  memcpy(address, &ethAddress, kIOEthernetAddressSize);

  return kIOReturnSuccess;
} // getHardwareAddress()


IOReturn BCM5722D::setHardwareAddress(const IOEthernetAddress *address)
{
  memcpy(&ethAddress, address, kIOEthernetAddressSize);

  configureMACAddress();

  return kIOReturnSuccess;
} // setHardwareAddress()


IOReturn BCM5722D::setMulticastMode(bool active)
{
  return kIOReturnSuccess;
} // setMulticastMode()


IOReturn BCM5722D::setMulticastList(IOEthernetAddress *addrs, UInt32 count)
{
  UInt32 hashReg[4] = {0};
  UInt32 regIndex;
  UInt32 bitPosition;
  UInt32 crc;
  int i;

  if (count) {

    for (i = 0; i < count; i++) {

      crc = computeEthernetCRC((const UInt8 *)(addrs + i), 6);
      bitPosition = ~crc & 0x7F;
      regIndex = (bitPosition & 0x60) >> 5;
      bitPosition &= 0x1F;
      hashReg[regIndex] |= (1 << bitPosition);

    }

    writeCSR(EMAC_HASH_0, hashReg[0]);
    writeCSR(EMAC_HASH_1, hashReg[1]);
    writeCSR(EMAC_HASH_2, hashReg[2]);
    writeCSR(EMAC_HASH_3, hashReg[3]);

  }

  return kIOReturnSuccess;
} // setMulticastList()


IOReturn BCM5722D::setPromiscuousMode(bool active)
{
  if (active) {
    rxMode |= EMAC_RXMODE_PROMISCMODE;
  } else {
    rxMode &= ~EMAC_RXMODE_PROMISCMODE;
  }

  promiscuousModeEnabled = active;

  writeCSR(EMAC_RXMODE, rxMode);

  return kIOReturnSuccess;
} // setPromiscuousMode()


#pragma mark -
#pragma mark - IONetworkController methods


IOReturn BCM5722D::enable(IONetworkInterface *iface)
{
  bool success = false;

  do {

    if (adapterEnabled) {
      break;
    }

    if (!pciNub->open(this)) {
      break;
    }

    stopAdapter();

    resetPHY();

    resetAdapter();

    initializeAdapter();

    configureMACAddress();

    if (!initRxRing()) {
      break;
    }

    if (!initTxRing()) {
      break;
    }

    clearStatistics();

    setBit(PCIE_TRANSCTL, PCIE_TRANSCTL_1SHOTMSI);

    enableInterrupts(true);

    timerSource->setTimeoutMS(kWatchDogTimeout);

    selectMedium(getSelectedMedium());

    transmitQueue->setCapacity(txQueueLength);
    transmitQueue->start();

    adapterEnabled = true;
    success = true;

  } while (false);

  return (success ? kIOReturnSuccess : kIOReturnIOError);
} // enable()


IOReturn BCM5722D::disable(IONetworkInterface *iface)
{
  bool success = false;

  do {

    if (!adapterEnabled) {
      break;
    }

    transmitQueue->stop();
    transmitQueue->setCapacity(0);
    transmitQueue->flush();

    timerSource->cancelTimeout();

    enableInterrupts(false);

    setLinkStatus(kIONetworkLinkValid, 0);

    stopAdapter();

    resetAdapter();

    freeRxRing();

    freeTxRing();

    pciNub->close(this);

    adapterEnabled = false;
    success = true;

  } while (false);

  return (success ? kIOReturnSuccess : kIOReturnIOError);
} // disable()


bool BCM5722D::createWorkLoop()
{
  workLoop = IOWorkLoop::workLoop();

  return (workLoop != 0);
} // createWorkLoop()


IOWorkLoop* BCM5722D::getWorkLoop() const
{
  return workLoop;
} // getWorkLoop()


IOOutputQueue* BCM5722D::createOutputQueue()
{
  return IOBasicOutputQueue::withTarget(this, txQueueLength);
} // createOutputQueue()


IOReturn BCM5722D::getChecksumSupport(UInt32 *checksumMask,
                                      UInt32 checksumFamily,
                                      bool isOutput)
{
  *checksumMask = 0;

  if (checksumFamily == kChecksumFamilyTCPIP) {

    *checksumMask = (kChecksumIP | kChecksumTCP | kChecksumUDP);

    return kIOReturnSuccess;

  }

  return kIOReturnUnsupported;
} // getChecksumSupport()


bool BCM5722D::configureInterface(IONetworkInterface *iface)
{
  IONetworkData *data;

  if (!super::configureInterface(iface)) {
    return false;
  }

  data = iface->getNetworkData(kIONetworkStatsKey);

  if (!data || !(networkStats = (IONetworkStats *)data->getBuffer())) {
    return false;
  }

  data = iface->getNetworkData(kIOEthernetStatsKey);

  if (!data || !(ethernetStats = (IOEthernetStats *)data->getBuffer())) {
    return false;
  }

  return true;
} // configureInterface()


IOReturn BCM5722D::getMaxPacketSize(UInt32 *maxSize) const
{
  *maxSize = kIOEthernetMaxPacketSize + 4;

  return kIOReturnSuccess;
} // getMaxPacketSize()


#pragma mark -
#pragma mark Identity


const OSString* BCM5722D::newModelString() const
{
  for (int i = 0; devices[i].id; i++) {
    if (devices[i].id == deviceID) {
			Log("Model: %s", devices[i].model);
      return OSString::withCString(devices[i].model);
    }
  }

  return OSString::withCString("Unknown Model");
} // newModelString()


const OSString* BCM5722D::newRevisionString() const
{
  char revision[3] = { 0 };

  snprintf(revision, sizeof(revision), "%X", GET_REVID(asicRevision));

  return OSString::withCString(revision);
} // newRevisionString()


const OSString* BCM5722D::newVendorString() const
{
  return OSString::withCString("Broadcom");
}


#pragma mark -
#pragma mark Medium


IOReturn BCM5722D::selectMedium(const IONetworkMedium *medium)
{
  bool success = false;

  if (OSDynamicCast(IONetworkMedium, medium) == 0) {
    medium = IONetworkMedium::getMediumWithType(mediumDict,
                                                kIOMediumEthernetAuto);
  }

  if (setMedium(medium) == kIOReturnSuccess) {

    setSelectedMedium(medium);
    success = true;

  }

  return (success ? kIOReturnSuccess : kIOReturnIOError);
} // selectMedium()


#pragma mark -
#pragma mark Work


UInt32 BCM5722D::outputPacket(mbuf_t m, void *param)
{
  if (txFreeSlot == 0) {
    return kIOReturnOutputStall;
  }

  IOPhysicalSegment   segments[kTxMaxSegmentCount];
  UInt32              segmentCount;
  IOReturn            status;

  segmentCount =
      txCursor->getPhysicalSegmentsWithCoalesce(m,
                                                segments,
                                                kTxMaxSegmentCount);

  if (segmentCount == 0) {

    freePacket(m);
    BUMP_NETSTAT(outputErrors);

    return kIOReturnOutputDropped;

  }

  if (segmentCount <= txFreeSlot) {

    UInt32 bdChecksumFlags;
    UInt16 bdFlags = 0;
    UInt16 bdVlanTag = 0;

    getChecksumDemand(m, kChecksumFamilyTCPIP, &bdChecksumFlags);

    if (bdChecksumFlags & kChecksumIP) {
      bdFlags |= TXBDFLAG_IP_CHECKSUM;
    }

    if (bdChecksumFlags & (kChecksumTCP | kChecksumUDP)) {
      bdFlags |= TXBDFLAG_TCP_UDP_CHECKSUM;
    }

    if (getVlanTagDemand(m, (UInt32 *)&bdVlanTag)) {
      bdFlags |= TXBDFLAG_VLAN_TAG;
    }

    int producer = txProducerIdx;
    int lastSegment;

    for (int i = 0; i < segmentCount; i++) {

      txBD[producer].addressHigh = HOSTADDRESS_HI(segments[i].location);
      txBD[producer].addressLow = HOSTADDRESS_LO(segments[i].location);
      txBD[producer].length = segments[i].length;
      txBD[producer].flags = bdFlags;
      txBD[producer].vlanTag = bdVlanTag;
      txBD[producer].mss = 0; // used for large send offload

      lastSegment = producer;
      INCREMENT(producer, kTxBDCount);

    }

    txBD[lastSegment].flags |= TXBDFLAG_PACKET_END;
    txPacketArray[txProducerIdx] = m;

    writeMailbox(HPMBX_TXPIDX_LO, producer);

    txFreeSlot -= segmentCount;
    txProducerIdx = producer;
    BUMP_NETSTAT(outputPackets);

    status = kIOReturnOutputSuccess;

  } else {

    DebugLog("%u -> %u segments, %u free, consumer index: %u",
             mbuf_pkthdr_len(m),
             segmentCount,
             txFreeSlot,
             txLocalConsumerIdx);

    status = kIOReturnOutputStall;

  }

  return status;
} // outputPacket()
