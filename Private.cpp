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

#define TX_RING_SIZE      (kTxBDCount * (sizeof(BTxBufferDescriptor)))
#define RX_RING_SIZE      (kRxBDCount * (sizeof(BRxBufferDescriptor)))
#define STATUS_BLOCK_SIZE (sizeof(BStatusBlock))

#pragma mark -
#pragma mark Initialization


bool BCM5722D::resetAdapter()
{
  UInt32 reset;
  UInt32 value;
  UInt16 pciCommand;

  enableInterrupts(false);

  lockNVRAM();

  writeCSR(MAR_MODE, MAR_MODE_ENABLE);

  pciNub->configWrite32(BPCI_MISCHOSTCTL, pciMiscHostControl);

  pciCommand = pciNub->configRead16(kIOPCIConfigCommand);

  if (GET_ASICREV(asicRevision) != ASICREV_C) {
    writeCSR(GCR_FASTBOOT, 0);
  }

  writeMemoryIndirect(SRAM_SOFT_GENCOMM, T3_MAGIC_NUMBER);

  reset = (GCR_MISCCFG_CORECLOCKRST |
           GCR_MISCCFG_GPHYPWRDWN |
           GCR_MISCCFG_TP_66MHZ
           );

  // #tg3 - Force PCIe 1.0a
  if (readCSR(PCIE_TSTCTL) == (PCIE_TSTCTL_10MODE |
                               PCIE_TSTCTL_SCRAMBLER)) {
    writeCSR(PCIE_TSTCTL, PCIE_TSTCTL_SCRAMBLER);
  }

  /* Causes machine lock when this bit is not set before chip reset */
  writeCSR(GCR_MISCCFG, GCR_MISCCFG_DISABLEGRCRST);
  reset |= GCR_MISCCFG_DISABLEGRCRST;

  if (GET_ASICREV(asicRevision) == ASICREV_C) {

    setBit(VCPU_STATUS, VCPU_STATUS_DRIVERRST);
    clearBit(WOL_MISCCSCTL, WOL_MISCCSCTL_VCPUHALT);

  }

  writeCSR(GCR_MISCCFG, reset);

  // Wait 100ms for PCIe systems
  IOSleep(100);

  enableInterrupts(false);

  pciNub->configWrite16(kIOPCIConfigCommand, pciCommand);

  UInt16 pcieDevCtl;

  pcieDevCtl = pciNub->configRead16(PCIECAP_DEVCTL);
  pcieDevCtl &= ~(PCIECAP_DEVCTL_RELAXORD |     // Disable relaxed ordering
                  PCIECAP_DEVCTL_NOSNOOP |      // Disable no snoop
                  PCIECAP_DEVCTL_MAXPLOAD_MASK  // Max payload: 128
                  );

  pciNub->configWrite16(PCIECAP_DEVCTL, pcieDevCtl);

  pciNub->configWrite16(PCIECAP_DEVSTAT, (PCIECAP_DEVSTAT_CED |
                                          PCIECAP_DEVSTAT_NFED |
                                          PCIECAP_DEVSTAT_FED |
                                          PCIECAP_DEVSTAT_URD
                                          ));

  writeCSR(MAR_MODE, MAR_MODE_ENABLE);

  pciNub->configWrite32(BPCI_MISCHOSTCTL, pciMiscHostControl);

  writeCSR(BPCI_STATE, (BPCI_STATE_EEPROM |
                        BPCI_STATE_EEPROMRETRY
                        ));

  writeCSR(GCR_MODECTL, (GCR_MODECTL_WSWAPNONFRAME |
                         GCR_MODECTL_BSWAP |
                         GCR_MODECTL_WSWAP
                         ));

  writeCSR(EMAC_MODE, 0);
  IODelay(40);

  // Step 16
  // #tg3
  int i;

  if (GET_ASICREV(asicRevision) == ASICREV_C) {

    for (i = 0; i < 200; i++) {

      if (readCSR(VCPU_STATUS) & VCPU_STATUS_INITDONE) {
        break;
      }

      IODelay(100);

    }
  } else {

    for (i = 0; i < 100000; i++) {

      value = readMemoryIndirect(SRAM_SOFT_GENCOMM);

      if (value == ~T3_MAGIC_NUMBER) {
        break;
      }

      IODelay(10);

    }
  }

  DebugLog("%X to %X in %d iterations",
           T3_MAGIC_NUMBER, ~T3_MAGIC_NUMBER, i);

  writeCSR(PCIE_TLPCTL, (PCIE_TLPCTL_DATAFIFOPROTECT |
                         PCIE_TLPCTL_IRQMODEFIX
                         ));

  return true;
} // reset()


bool BCM5722D::initializeAdapter()
{
  UInt32 value;
  int i;

  writeCSR(BPCI_MEMWINBASEADDR, 0);

  // #tg3
  // 0x40180000
  writeCSR(BPCI_DMARWCTL, 0x76180000);

  // Step 22
  writeCSR(GCR_MODECTL, (GCR_MODECTL_WSWAPNONFRAME |
                         GCR_MODECTL_BSWAP |
                         GCR_MODECTL_WSWAP |
                         GCR_MODECTL_HOSTSENDBD |
                         GCR_MODECTL_HOSTSTACKUP |
                         GCR_MODECTL_NOTXPHDRCSUM |
                         GCR_MODECTL_IRQONMACATTN
                         ));

  // The core clock of the device runs at 66MHz
  value = readCSR(GCR_MISCCFG);
  value |= GCR_MISCCFG_TP_66MHZ;
  writeCSR(GCR_MISCCFG, value);

  // Step 28

  writeCSR(BMC_RDMAMBUFLOWAT, 0);

  // #tg3
  if (GET_ASICREV(asicRevision) == ASICREV_C) {

    writeCSR(BMC_RXMBUFLOWAT, 0x4);
    writeCSR(BMC_MBUFHIWAT, 0x10);

  } else {

    writeCSR(BMC_RXMBUFLOWAT, 0x10);
    writeCSR(BMC_MBUFHIWAT, 0x60);

  }

  writeCSR(EMAC_LOWATMAXRXFRAMES, 2);

  writeCSR(BMC_DMADESCRLOWAT, 5);
  writeCSR(BMC_DMADESCRHIWAT, 10);

  writeCSR(BMC_MODE, (BMC_MODE_ENABLE | BMC_MODE_ATTN));

  for (i = 0; i < 1000; i++) {

    if ((readCSR(BMC_MODE) & BMC_MODE_ENABLE)) {
      break;
    }

    IODelay(10);

  }

  if (i == 1000) {

    Log("Unable to start device buffer manager");
    return false;

  }

  writeCSR(FTQ_RESET, 0xFFFFFFFF);
  writeCSR(FTQ_RESET, 0x0);

  // Step 33
  writeCSR(RXDBDI_RXPRCB_HOSTADDR_HI, HOSTADDRESS_HI(rxAddress));
  writeCSR(RXDBDI_RXPRCB_HOSTADDR_LO, HOSTADDRESS_LO(rxAddress));
  writeCSR(RXDBDI_RXPRCB_MAXLEN, (0x200 << RCB_MAXLEN_SHIFT)); // 512
  writeCSR(RXDBDI_RXPRCB_NICADDR, SRAM_RXP_RING);

  value = 25;

  // #tg3
  if (GET_ASICREV(asicRevision) == ASICREV_C) {

    switch (GET_REVID(asicRevision)) {

      case REVLEVEL_A0:
      case REVLEVEL_A1:
      case REVLEVEL_A2:
        writeCSR(TXDI_TXISOPKT, readCSR(TXDI_TXISOPKT) & ~0x3 | 0x2);

      default:
        break;

    }

    // #tg3 - Calculated from TG3_RX_INTERNAL_RING_SZ_5906 / 2
    value = 16;

  }

  writeCSR(RXBDI_RXBDPRINGTHOLD, value);

  writeMailbox(HPMBX_TXPIDX_LO, 0);

  UInt32 address;

  address = MEMWIN_START + SRAM_TX_RING_RCB;
  writeCSR(address + RCB_HOSTADDR_HI, HOSTADDRESS_HI(txAddress));
  writeCSR(address + RCB_HOSTADDR_LO, HOSTADDRESS_LO(txAddress));
  writeCSR(address + RCB_MAXLEN_FLAGS, (kTxBDCount << RCB_MAXLEN_SHIFT));
  writeCSR(address + RCB_NICADDR, SRAM_TX_RING);

  // Step 37
  // BCM5755/M supports up to 4 receive return ring. Disable the other 3
  if (GET_ASICREV(asicRevision) == ASICREV_A) {

    value = SRAM_RXR_RING_RCB + SRAM_RXR_RING_RCB_NEXT * 4;

    for (address = SRAM_RXR_RING_RCB + SRAM_RXR_RING_RCB_NEXT;
         address < value;
         address += SRAM_RXR_RING_RCB_NEXT) {
      writeMemoryIndirect(address + RCB_MAXLEN_FLAGS,
                          RCB_FLAGS_RING_DISABLED);
    }
  }

  // Step 38
  address = MEMWIN_START + SRAM_RXR_RING_RCB;
  writeCSR(address + RCB_HOSTADDR_HI, HOSTADDRESS_HI(rxReturnAddress));
  writeCSR(address + RCB_HOSTADDR_LO, HOSTADDRESS_LO(rxReturnAddress));
  writeCSR(address + RCB_MAXLEN_FLAGS, (0x200 << RCB_MAXLEN_SHIFT));
  writeCSR(address + RCB_NICADDR, 0);

  writeMailbox(HPMBX_RXPIDX_LO, 0);

  // Step 42
  // Max MTU = kIOEthernetMaxPacketSize(1518) + Vlan Tag(4)
  writeCSR(EMAC_RXMTUSIZE, kIOEthernetMaxPacketSize + 4);

  // Configure transmit inter-packet gap
  writeCSR(EMAC_TXLEN, 0x2620);

  writeCSR(EMAC_RXRULESCONF, EMAC_RXRULESCONF_DEF01);

  // Step 45
  writeCSR(RXLP_CONF, 0x181);

  // Step 46
  // TODO: Verify DACK issue
  value = readCSR(RXLP_STATSENABLE);
  value &= ~RXLP_STATSENABLE_DACKFIX;
  writeCSR(RXLP_STATSENABLE, value);
  //writeCSR(RXLP_STATSENABLE, 0xFFFFFF);

  // Step 47
  writeCSR(RXLP_STATSCTL, RXLP_STATSCTL_ENABLE);

  writeCSR(TXDI_STATSENABLE, 0xFFFFFF);

  writeCSR(TXDI_STATSCTL, (TXDI_STATSCTL_ENABLE |
                           TXDI_STATSCTL_FASTUPDATE
                           ));

  // Step 50
  writeCSR(HCC_MODE, 0);

  for (i = 0; i < 2000; i++) {

    if (!(readCSR(HCC_MODE) & HCC_MODE_ENABLE)) {
      break;
    }

    IODelay(10);

  }

  writeCSR(HCC_RXCOALTICKS, 150);
  writeCSR(HCC_TXCOALTICKS, 150);

  // Step 53
  writeCSR(HCC_RXMAXCOALBDCNT, 10);
  writeCSR(HCC_TXMAXCOALBDCNT, 10);

  writeCSR(HCC_RXMAXCOALBDCNTIRQ, 0);
  writeCSR(HCC_TXMAXCOALBDCNTIRQ, 0);

  writeCSR(HCC_STATUSBLOCKHOSTADDR_HI, HOSTADDRESS_HI(statusBlockAddress));
  writeCSR(HCC_STATUSBLOCKHOSTADDR_LO, HOSTADDRESS_LO(statusBlockAddress));

  // Step 56
  writeCSR(HCC_MODE, (HCC_MODE_STATBLKSIZE_32 |
                      HCC_MODE_ENABLE
                      ));

  writeCSR(RXBDC_MODE, (RXBDC_MODE_ENABLE |
                        RXBDC_MODE_ATTN
                        ));

  writeCSR(RXLP_MODE, RXLP_MODE_ENABLE);

  writeCSR(EMAC_MODE, (macMode |
                       EMAC_MODE_RXSTATSCLR |
                       EMAC_MODE_TXSTATSCLR |
                       EMAC_MODE_PORTMODE_MII
                       ));
  IODelay(40);

  writeCSR(GCR_MISCLCLCTL, (GCR_MISCLCLCTL_IRQONATTN |
                            GCR_MISCLCLCTL_AUTOSEEPROMACC
                            ));
  IODelay(100);

  // Step 63
  value = (WDMA_MODE_ENABLE |
           WDMA_MODE_TGTABORTATTN |
           WDMA_MODE_MSTABORTATTN |
           WDMA_MODE_PARABORTATTN |
           WDMA_MODE_HOSTADDROFLOWATTN |
           WDMA_MODE_FIFOORUNATTN |
           WDMA_MODE_FIFOURUNATTN |
           WDMA_MODE_FIFOOWRITEATTN |
           WDMA_MODE_WLONGERLENATTN |
           WDMA_MODE_STATTAGFIX
           );

  writeCSR(WDMA_MODE, value);
  IODelay(40);

  value = (RDMA_MODE_ENABLE |
           RDMA_MODE_TGTABORTATTN |
           RDMA_MODE_MSTABORTATTN |
           RDMA_MODE_PARABORTATTN |
           RDMA_MODE_HOSTADDROFLOWATTN |
           RDMA_MODE_FIFOORUNATTN |
           RDMA_MODE_FIFOURUNATTN |
           RDMA_MODE_FIFOOREADATTN |
           RDMA_MODE_WLONGERLENATTN |
           RDMA_MODE_BURSTLEN_LONG
           );

  // TODO: Read DMA Mode, assert bit 27 when tso is implemented

  writeCSR(RDMA_MODE, value);
  IODelay(40);

  writeCSR(RXDC_MODE, (RXDC_MODE_ENABLE |
                       RXDC_MODE_ATTN
                       ));

  writeCSR(TXDC_MODE, TXDC_MODE_ENABLE);

  writeCSR(TXBDC_MODE, (TXBDC_MODE_ENABLE |
                        TXBDC_MODE_ATTN
                        ));

  writeCSR(RXBDI_MODE, (RXBDI_MODE_ENABLE |
                        RXBDI_MODE_ATTN
                        ));

  writeCSR(RXDBDI_MODE, (RXDBDI_MODE_ENABLE |
                         RXDBDI_MODE_ILLEGALRTNRINGSZ
                         ));

  writeCSR(TXDI_MODE, TXDI_MODE_ENABLE);

  writeCSR(TXBDI_MODE, (TXBDI_MODE_ENABLE |
                        TXBDI_MODE_ATTN
                        ));

  writeCSR(TXBDS_MODE, (TXBDS_MODE_ENABLE |
                        TXBDS_MODE_ATTN
                        ));

  writeCSR(EMAC_TXMODE, txMode);
  IODelay(40);

  writeCSR(EMAC_RXMODE, rxMode);
  IODelay(40);

  if (readCSR(EMAC_MIMODE) & EMAC_MIMODE_PORTPOLL) {
    writeCSR(EMAC_MIMODE, EMAC_MIMODE_CLOCK_66MHZ);
  }

  writeCSR(EMAC_LEDCTL, EMAC_LEDCTL_MODE_PHY1);

  writeCSR(EMAC_MISTAT, EMAC_MISTAT_LINKSTAT);

  writeCSR(EMAC_EVENT, EMAC_EVENT_LNKSTATECHGD);

  return true;
} // initializeAdapter()


void BCM5722D::prepareDriver()
{
  // Set default value
  txQueueLength = 1000;

  UInt32 value;

  value = readCSR(BPCI_MISCHOSTCTL);
  asicRevision = (value >> BPCI_MISCHOSTCTL_ASICREV_SHFT);

	deviceID = pciNub->configRead16(kIOPCIConfigDeviceID);

  // Set default register value
  macMode = (EMAC_MODE_TDE |
             EMAC_MODE_RDE |
             EMAC_MODE_FHDE |
             EMAC_MODE_RXSTATSENA |
             EMAC_MODE_TXSTATSENA
             );

  pciMiscHostControl =  (BPCI_MISCHOSTCTL_MASKIRQOUT |
                         BPCI_MISCHOSTCTL_WORDSWAP |
                         BPCI_MISCHOSTCTL_STATERW |
                         BPCI_MISCHOSTCTL_INDACC |
                         BPCI_MISCHOSTCTL_TAGSTATMODE
                         );

  txMode = (EMAC_TXMODE_ENABLE |
            EMAC_TXMODE_LOCKUPFIX
            );

  rxMode = (EMAC_RXMODE_ENABLE |
            EMAC_RXMODE_IPV6PARSING
            );

  resetAdapter();

  /* Only fetch address from MAC address registers */
  value = readCSR(EMAC_MACADDR0_HI);
  ethAddress.bytes[0] = (value >> 8) & 0xFF;
  ethAddress.bytes[1] = value & 0xFF;

  value = readCSR(EMAC_MACADDR0_LO);
  ethAddress.bytes[2] = (value >> 24) & 0xFF;
  ethAddress.bytes[3] = (value >> 16) & 0xFF;
  ethAddress.bytes[4] = (value >> 8) & 0xFF;
  ethAddress.bytes[5] = value & 0xFF;
} // prepareAdapter()


void BCM5722D::stopAdapter()
{
  static const struct StopSequence
  {
    UInt32 reg;
    UInt32 bit;
  } steps[] = {

    // Receive path shutdown
    {EMAC_RXMODE, EMAC_RXMODE_ENABLE},
    {RXBDI_MODE, RXBDI_MODE_ENABLE},
    {RXLP_MODE, RXLP_MODE_ENABLE},
    {RXDBDI_MODE, RXDBDI_MODE_ENABLE},
    {RXDC_MODE, RXDC_MODE_ENABLE},
    {RXBDC_MODE, RXBDC_MODE_ENABLE},

    // Transmit path shutdown
    {TXBDS_MODE, TXBDS_MODE_ENABLE},
    {TXBDI_MODE, TXBDI_MODE_ENABLE},
    {TXDI_MODE, TXDI_MODE_ENABLE},
    {RDMA_MODE, RDMA_MODE_ENABLE},
    {TXDC_MODE, TXDC_MODE_ENABLE},
    {TXBDC_MODE, TXBDC_MODE_ENABLE},
    {EMAC_MODE, EMAC_MODE_TDE},
    {EMAC_TXMODE, EMAC_TXMODE_ENABLE},

    // Memory related state machine shutdown
    {HCC_MODE, HCC_MODE_ENABLE},
    {WDMA_MODE, WDMA_MODE_ENABLE},
    {0, 0}

  };

  int i;
  int j;
  UInt32 value;

  for (i = 0; steps[i].reg != 0; i++) {

    clearBit(steps[i].reg, steps[i].bit);

    j = 4000;

    while (j != 0) {

      IODelay(10);

      value = readCSR(steps[i].reg);

      if ((value & steps[i].bit) == 0) {
        break;
      }

      j--;

    }

    if (j == 0) {
      DebugLog("0x%08X enable bit does not clear (0x%08X)",
               steps[i].reg,
               readCSR(steps[i].reg));
    }
  }

  writeCSR(FTQ_RESET, 0xFFFFFFFF);
  writeCSR(FTQ_RESET, 0);
} // stopAdapter()


void BCM5722D::initializePCIConfig()
{
  pciNub->setBusMasterEnable(true);
  pciNub->setMemoryEnable(true);

  UInt16 pmCapability;

  pmCapability = pciNub->configRead16(kPCIPMCapabilities);

  magicPacketSupported =
      (pmCapability & kPCIPMCPMESupportFromD3Cold) ? true : false;

  pciNub->configWrite16(kPCIPMCS, kPCIPMCSPMEStatus);

  IOSleep(10);
} // initializePCIConfig()


IOReturn BCM5722D::lockNVRAM()
{
  int i;

  writeCSR(NVM_SOFTARB, NVM_SOFTARB_REQ_SET1);

  for (i = 0; i < 1000; i++) {

    if (readCSR(NVM_SOFTARB) & NVM_SOFTARB_ARB_WON1) {
      break;
    }

    IODelay(10);

  }

  if (i == 1000) {

    writeCSR(NVM_SOFTARB, NVM_SOFTARB_REQ_CLR1);
    return kIOReturnBusy;

  }

  return kIOReturnSuccess;
} // lockNVRAM()


/*
 * Write MAC address to the MAC address register and configure
 * transmit random backoff seed.
 */
void BCM5722D::configureMACAddress()
{
  UInt32 value;

  value = (ethAddress.bytes[0] << 8 |
           ethAddress.bytes[1]);
  writeCSR(EMAC_MACADDR0_HI, value);

  value = (ethAddress.bytes[2] << 24 |
           ethAddress.bytes[3] << 16 |
           ethAddress.bytes[4] << 8 |
           ethAddress.bytes[5]);
  writeCSR(EMAC_MACADDR0_LO, value);

  UInt32 seed = (ethAddress.bytes[0] + ethAddress.bytes[1] +
                 ethAddress.bytes[2] + ethAddress.bytes[3] +
                 ethAddress.bytes[4] + ethAddress.bytes[4]) & 0x3FF;

  writeCSR(EMAC_TXRANDBACKOFF, seed);
} // configureMACAddress()


#pragma mark -
#pragma mark Memory/ring initialization


bool BCM5722D::allocateDescriptorMemory(IOBufferMemoryDescriptor **memory,
                                        IOByteCount size)
{
  *memory = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(kernel_task,
              kIOMemoryPhysicallyContiguous, size, 0xFFFFFFFFFFFFFFFFULL);

  if (*memory && (*memory)->prepare() != kIOReturnSuccess) {

    (*memory)->release();
    *memory = 0;

  }

  return (*memory != 0);
} // allocateDescriptorMemory()


void BCM5722D::freeDescriptorMemory(IOBufferMemoryDescriptor **memory)
{
  if (*memory) {

    (*memory)->complete();
    (*memory)->release();
    *memory = 0;

  }
} // freeDescriptorMemory()


bool BCM5722D::allocateDriverMemory()
{
  bool success;

  do {

    // Tx
    if (!allocateDescriptorMemory(&txMD, TX_RING_SIZE)) {

      Log("Unable to allocate descriptor memory for Tx use");
      break;

    }

    txBD = (BTxBufferDescriptor *)txMD->getBytesNoCopy();
    memset(txBD, 0, TX_RING_SIZE);

    txAddress = txMD->getPhysicalSegment(0, 0);

    txCursor = IOMbufNaturalMemoryCursor::withSpecification(kMaxPacketSize,
                                                            kTxMaxSegmentCount);

    // Rx
    if (!allocateDescriptorMemory(&rxMD, RX_RING_SIZE)) {

      Log("Unable to allocate descriptor memory for Rx use");
      break;

    }

    rxBD = (BRxBufferDescriptor *)rxMD->getBytesNoCopy();
    memset(rxBD, 0, RX_RING_SIZE);

    rxAddress = rxMD->getPhysicalSegment(0, 0);

    rxCursor = IOMbufNaturalMemoryCursor::withSpecification(kMaxPacketSize,
                                                            kRxMaxSegmentCount);

    // Rx Return
    if (!allocateDescriptorMemory(&rxReturnMD, RX_RING_SIZE)) {

      Log("Unable to allocate descriptor memory for Rx Return use");
      break;

    }

    rxReturnBD = (BRxBufferDescriptor *)rxReturnMD->getBytesNoCopy();
    memset(rxReturnBD, 0, RX_RING_SIZE);

    rxReturnAddress = rxReturnMD->getPhysicalSegment(0, 0);

    // Status Block
    if (!allocateDescriptorMemory(&statusBlockMD, STATUS_BLOCK_SIZE)) {

      IOLog("Unable to allocate descriptor memory for Status Block use");
      break;

    }

    statusBlock = (BStatusBlock *)statusBlockMD->getBytesNoCopy();
    memset(statusBlock, 0, STATUS_BLOCK_SIZE);

    statusBlockAddress = statusBlockMD->getPhysicalSegment(0, 0);

    success = true;

  } while (false);

  return success;
} // allocateDriverMemory()


bool BCM5722D::initTxRing()
{
  memset(txBD, 0, TX_RING_SIZE);

  writeMailbox(HPMBX_TXPIDX_LO, 0);

  txProducerIdx = txLocalConsumerIdx = 0;
  txFreeSlot = kTxBDCount;

  return true;
} // initTxRing()


void BCM5722D::freeTxRing()
{
  int i;

  for (i = 0; i < kTxBDCount; i++) {

    if (txPacketArray[i] != 0) {

      freePacket(txPacketArray[i]);
      txPacketArray[i] = 0;

    }
  }
} // freeTxRing()


bool BCM5722D::initRxRing()
{
  int                  i;
  bool                 success = true;

  memset(rxBD, 0, RX_RING_SIZE);

  for (i = 0; i < kRxBDCount; i++) {

    if (rxPacketArray[i]) {
      continue;
    }

    rxPacketArray[i] = allocatePacket(kIOEthernetMaxPacketSize + 4);

    if (!rxPacketArray[i]) {
      break;
    }

    if (!configureRxDescriptor(i)) {

      freePacket(rxPacketArray[i]);
      success = false;
      break;

    }
  }

  rxProducerIdx = rxReturnConsumerIdx = 0;

  writeMailbox(HPMBX_RXPIDX_LO, kRxBDCount - 1);

  return success;
} // initRings()


void BCM5722D::freeRxRing()
{
  int i;

  for (i = 0; i < kRxBDCount; i++) {

    if (rxPacketArray[i]) {

      freePacket(rxPacketArray[i]);
      rxPacketArray[i] = 0;

    }
  }
} // freeRings()


/* Updates bd on rx std ring */
bool BCM5722D::configureRxDescriptor(UInt16 index,
                                     BOption options)
{
  IOPhysicalSegment    segment;
  BRxBufferDescriptor *bd;
  UInt32               count;
  bool                 updated = false;

  bd = &rxBD[index];

  switch (options) {

    case kBDOptionReuse:

      bd->flags = RXBDFLAG_PACKET_END;
      bd->length = rxSegmentLength[index];

      updated = true;
      break;

    default:

      count = rxCursor->getPhysicalSegmentsWithCoalesce(rxPacketArray[index],
                                                        &segment,
                                                        kRxMaxSegmentCount);

      if (count) {

        bd->addressHigh = HOSTADDRESS_HI(segment.location);
        bd->addressLow = HOSTADDRESS_LO(segment.location);
        bd->flags = RXBDFLAG_PACKET_END;
        bd->length = segment.length;
        bd->index = index;

        rxSegmentLength[index] = segment.length;

        updated = true;

      }

      break;

  }

  return updated;
} // configureRxDescriptor()


#pragma mark -
#pragma mark Interrupt


bool BCM5722D::setupDriver(IOService *provider)
{
  transmitQueue = getOutputQueue();

  if (!transmitQueue) {
    return false;
  }

  IOWorkLoop *tWorkLoop = (IOWorkLoop *)getWorkLoop();

  if (!tWorkLoop) {
    return false;
  }

  interruptSource = IOInterruptEventSource::interruptEventSource(
      this, OSMemberFunctionCast(IOInterruptEventAction, this,
                                 &BCM5722D::interruptOccurred),
      provider, 1);

  if (!interruptSource ||
      (tWorkLoop->addEventSource(interruptSource) != kIOReturnSuccess)) {

    Log("Failed to register interrupt source");
    return false;

  }

  interruptSource->enable();

  timerSource = IOTimerEventSource::timerEventSource(
      this, OSMemberFunctionCast(IOTimerEventSource::Action, this,
                                 &BCM5722D::timeoutOccurred));

  if (!timerSource ||
      (tWorkLoop->addEventSource(timerSource) != kIOReturnSuccess)) {

    Log("Failed to register interrupt source");
    return false;

  }

  return true;
} // setupDriver()


void BCM5722D::enableInterrupts(bool active)
{
  if (active) {

    clearBit(BPCI_MISCHOSTCTL, BPCI_MISCHOSTCTL_MASKIRQOUT);
    setBit(BPCI_MISCHOSTCTL, BPCI_MISCHOSTCTL_CLEARINTA);
    writeMailbox(HPMBX_IRQ0_LO, 0);

    interruptEnabled = true;

  } else {

    setBit(BPCI_MISCHOSTCTL, BPCI_MISCHOSTCTL_MASKIRQOUT);
    writeMailbox(HPMBX_IRQ0_LO, 1);

    interruptEnabled = false;

  }
} // enableInterrupts()


void BCM5722D::interruptOccurred(IOInterruptEventSource *source, int count)
{
  UInt32 statusTag;

  statusTag = statusBlock->statusTag << 24;

  if (statusBlock->statusWord & STATUS_WORD_LNKCHGD) {
    serviceLinkInterrupt();
  }

  UInt16 producerIdx = statusBlock->rxReturn1ProducerIdx;

  if (producerIdx != rxReturnConsumerIdx) {
    serviceRxInterrupt(producerIdx);
    BUMP_RXSTAT(interrupts);
  }

  UInt16 consumerIdx = statusBlock->txConsumerIdx;

  if (consumerIdx != txLocalConsumerIdx) {
    serviceTxInterrupt(consumerIdx);
    BUMP_TXSTAT(interrupts);
    transmitQueue->service(IOBasicOutputQueue::kServiceAsync);
  }

  writeMailbox(HPMBX_IRQ0_LO, statusTag);

} // interruptOccurred()


void BCM5722D::timeoutOccurred(IOTimerEventSource *source)
{
  updateStatistics();

  timerSource->setTimeoutMS(kWatchDogTimeout);
} // timeoutOccurred()


void BCM5722D::serviceTxInterrupt(UInt16 consumerIdx)
{
  while (txLocalConsumerIdx != consumerIdx) {

    if (txPacketArray[txLocalConsumerIdx] != 0) {

      freePacket(txPacketArray[txLocalConsumerIdx]);
      txPacketArray[txLocalConsumerIdx] = 0;

    }

    txFreeSlot++;
    INCREMENT(txLocalConsumerIdx, kTxBDCount);

  }
} // serviceTxInterrupt()


/**
 * @param producerIdx, Rx return ring producer index
 */
void BCM5722D::serviceRxInterrupt(UInt16 producerIdx)
{
  BRxBufferDescriptor *bd;
  mbuf_t               inputPacket;
  bool                 replaced;
  UInt16               packetLength;
  UInt32               checksumValidMask = 0;

  while (rxReturnConsumerIdx != producerIdx) {

    bd = &rxReturnBD[rxReturnConsumerIdx];

    if (bd->flags & RXBDFLAG_FRAME_HAS_ERROR) {

      BUMP_NETSTAT(inputErrors);

      if (bd->errorFlags & RXBDERROR_COLL_DETECT)    BUMP_RXSTAT(collisionErrors);
      if (bd->errorFlags & RXBDERROR_PHY_DECODE_ERR) BUMP_RXSTAT(phyErrors);
      if (bd->errorFlags & RXBDERROR_TRUNC_NO_RES)   BUMP_RXSTAT(resourceErrors);
      if (bd->errorFlags & RXBDERROR_GIANT_PKT_RCVD) BUMP_RXSTAT(overruns);
      if (bd->errorFlags & RXBDERROR_MAC_ABORT)      BUMP_RXSTAT(resets);

      configureRxDescriptor(bd->index, kBDOptionReuse);

      goto serviceRxInterruptNext;

    }

    packetLength = bd->length - kIOEthernetCRCSize;

    inputPacket = replaceOrCopyPacket(&rxPacketArray[bd->index],
                                      packetLength,
                                      &replaced);

    if (inputPacket == 0) {

      DebugLog("replaceOrCopyPacket returns 0");
      configureRxDescriptor(bd->index, kBDOptionReuse);
      BUMP_NETSTAT(inputErrors);

      goto serviceRxInterruptNext;

    }

    if (replaced && !configureRxDescriptor(bd->index)) {

      freePacket(rxPacketArray[bd->index]);
      rxPacketArray[bd->index] = inputPacket;
      inputPacket = 0;
      BUMP_NETSTAT(inputErrors);

      goto serviceRxInterruptNext;

    }

    if (bd->flags & RXBDFLAG_IP_CHECKSUM) {
      checksumValidMask |= kChecksumIP;
    }

    if (bd->flags & RXBDFLAG_TCP_UDP_CHECKSUM) {
      checksumValidMask |= (kChecksumTCP | kChecksumUDP);
    }

    setChecksumResult(inputPacket,
                      kChecksumFamilyTCPIP,
                      (kChecksumIP | kChecksumTCP | kChecksumUDP),
                      checksumValidMask);

    if (bd->flags & RXBDFLAG_VLAN_TAG) {
      setVlanTag(inputPacket, bd->vlanTag);
    }

    netIface->inputPacket(inputPacket,
                          packetLength,
                          IONetworkInterface::kInputOptionQueuePacket);

    BUMP_NETSTAT(inputPackets);

serviceRxInterruptNext:

    INCREMENT(rxReturnConsumerIdx, kRxBDCount);
    INCREMENT(rxProducerIdx, kRxBDCount);

  }

  writeMailbox(HPMBX_RXR1IDX_LO, rxReturnConsumerIdx);
  writeMailbox(HPMBX_RXPIDX_LO, rxProducerIdx);

  netIface->flushInputQueue();
} // serviceRxInterrupt()


#pragma mark -
#pragma mark Statistics


void BCM5722D::clearStatistics()
{
  readCSR(RXSTAT_IFHCINOCTETS);
  readCSR(RXSTAT_ETHERSTATSFRAGMENTS);
  readCSR(RXSTAT_IFHCINUCASTPKTS);
  readCSR(RXSTAT_IFHCINMCASTPKTS);
  readCSR(RXSTAT_IFHCINBCASTPKTS);
  readCSR(RXSTAT_DOT3FCSERR);
  readCSR(RXSTAT_DOT3ALIGNMENTERR);
  readCSR(RXSTAT_XONPAUSEFRAMESRCVD);
  readCSR(RXSTAT_XOFFPAUSEFRAMESRCVD);
  readCSR(RXSTAT_MACCTLFRAMESRCVD);
  readCSR(RXSTAT_XOFFSTATEENTERED);
  readCSR(RXSTAT_DOT3FRAMESTOOLONGS);
  readCSR(RXSTAT_ETHERSTATSJABBERS);
  readCSR(RXSTAT_ETHERSTATSUNDERSZPKTS);

  readCSR(TXSTAT_IFHCOUTOCTETS);
  readCSR(TXSTAT_ETHERSTATSCOLLISIONS);
  readCSR(TXSTAT_OUTXONSENT);
  readCSR(TXSTAT_OUTXOFFSENT);
  readCSR(TXSTAT_DOT3INTERNALMACTXERR);
  readCSR(TXSTAT_DOT3SINGLECOLLFRAMES);
  readCSR(TXSTAT_DOT3MULTICOLLFRAMES);
  readCSR(TXSTAT_DOT3DEFERREDTX);
  readCSR(TXSTAT_DOT3EXCESSIVECOLL);
  readCSR(TXSTAT_DOT3LATECOLL);
  readCSR(TXSTAT_IFHCOUTUCASTPKTS);
  readCSR(TXSTAT_IFHCOUTMCASTPKTS);
  readCSR(TXSTAT_IFHCOUTBCASTPKTS);
  readCSR(TXSTAT_DOT3CARRIERSENSEERR);
  readCSR(TXSTAT_IFOUTDISCARDS);
} // clearStatistics()


#define UPDATE_STATISTICS(s, r) \
    do { \
      UInt32 value = readCSR((r)); \
      (s) += value; \
    } while (false)


// This function is called periodically
void BCM5722D::updateStatistics()
{
//  UPDATE_STATISTICS(RXSTAT_IFHCINOCTETS);
//  UPDATE_STATISTICS(RXSTAT_ETHERSTATSFRAGMENTS);
//  UPDATE_STATISTICS(RXSTAT_IFHCINUCASTPKTS);
//  UPDATE_STATISTICS(RXSTAT_IFHCINMCASTPKTS);
//  UPDATE_STATISTICS(RXSTAT_IFHCINBCASTPKTS);
  UPDATE_STATISTICS(ethernetStats->dot3StatsEntry.fcsErrors,
                    RXSTAT_DOT3FCSERR);
  UPDATE_STATISTICS(ethernetStats->dot3StatsEntry.alignmentErrors,
                    RXSTAT_DOT3ALIGNMENTERR);
//  UPDATE_STATISTICS(RXSTAT_XONPAUSEFRAMESRCVD);
//  UPDATE_STATISTICS(RXSTAT_XOFFPAUSEFRAMESRCVD);
//  UPDATE_STATISTICS(RXSTAT_MACCTLFRAMESRCVD);
//  UPDATE_STATISTICS(RXSTAT_XOFFSTATEENTERED);
  UPDATE_STATISTICS(ethernetStats->dot3StatsEntry.frameTooLongs,
                    RXSTAT_DOT3FRAMESTOOLONGS);
//  UPDATE_STATISTICS(RXSTAT_ETHERSTATSJABBERS);
  UPDATE_STATISTICS(ethernetStats->dot3RxExtraEntry.frameTooShorts,
                    RXSTAT_ETHERSTATSUNDERSZPKTS);

//  UPDATE_STATISTICS(TXSTAT_IFHCOUTOCTETS);
  UPDATE_STATISTICS(networkStats->collisions,
                    TXSTAT_ETHERSTATSCOLLISIONS);
//  UPDATE_STATISTICS(TXSTAT_OUTXONSENT);
//  UPDATE_STATISTICS(TXSTAT_OUTXOFFSENT);
  UPDATE_STATISTICS(ethernetStats->dot3StatsEntry.internalMacTransmitErrors,
                    TXSTAT_DOT3INTERNALMACTXERR);
  UPDATE_STATISTICS(ethernetStats->dot3StatsEntry.singleCollisionFrames,
                    TXSTAT_DOT3SINGLECOLLFRAMES);
  UPDATE_STATISTICS(ethernetStats->dot3StatsEntry.multipleCollisionFrames,
                    TXSTAT_DOT3MULTICOLLFRAMES);
  UPDATE_STATISTICS(ethernetStats->dot3StatsEntry.deferredTransmissions,
                    TXSTAT_DOT3DEFERREDTX);
  UPDATE_STATISTICS(ethernetStats->dot3StatsEntry.excessiveCollisions,
                    TXSTAT_DOT3EXCESSIVECOLL);
  UPDATE_STATISTICS(ethernetStats->dot3StatsEntry.lateCollisions,
                    TXSTAT_DOT3LATECOLL);
//  UPDATE_STATISTICS(TXSTAT_IFHCOUTUCASTPKTS);
//  UPDATE_STATISTICS(TXSTAT_IFHCOUTMCASTPKTS);
//  UPDATE_STATISTICS(TXSTAT_IFHCOUTBCASTPKTS);
  UPDATE_STATISTICS(ethernetStats->dot3StatsEntry.carrierSenseErrors,
                    TXSTAT_DOT3CARRIERSENSEERR);
//  UPDATE_STATISTICS(TXSTAT_IFOUTDISCARDS);

} // updateStatistics()


#pragma mark -
#pragma mark Reader/Writer methods


void BCM5722D::writeCSR(UInt32 offset, UInt32 value)
{
  OSWriteLittleInt32(csrBase, offset, value);
} // writeCSR()


UInt32 BCM5722D::readCSR(UInt32 offset)
{
  return OSReadLittleInt32(csrBase, offset);
} // readCSR()


void BCM5722D::clearBit(UInt32 offset, UInt32 bit)
{
  writeCSR(offset, readCSR(offset) & ~bit);
} // clearBit()


void BCM5722D::setBit(UInt32 offset, UInt32 bit)
{
  writeCSR(offset, readCSR(offset) | bit);
} // setBit()


void BCM5722D::writeMemoryIndirect(UInt32 offset, UInt32 value)
{
  writeCSR(BPCI_MEMWINBASEADDR, offset);
  writeCSR(BPCI_MEMWINDATA, value);
  writeCSR(BPCI_MEMWINBASEADDR, 0);
} // writeMemoryIndirect()


UInt32 BCM5722D::readMemoryIndirect(UInt32 offset)
{
  UInt32 value;

  writeCSR(BPCI_MEMWINBASEADDR, offset);
  value = readCSR(BPCI_MEMWINDATA);
  writeCSR(BPCI_MEMWINBASEADDR, 0);

  return value;
} // readMemoryIndirect()


/*
 * BCM5906 don't have high-priority mailboxes
 */
void BCM5722D::writeMailbox(UInt32 offset, UInt32 value)
{
  if (GET_ASICREV(asicRevision) == ASICREV_C) {
    offset += LPMBX_IRQ0_HI - HPMBX_IRQ0_HI;
  }

  writeCSR(offset, value);
} // writeMailbox()


UInt32 BCM5722D::readMailbox(UInt32 offset)
{
  if (GET_ASICREV(asicRevision) == ASICREV_C) {
    offset += LPMBX_IRQ0_HI - HPMBX_IRQ0_HI;
  }

  return readCSR(offset);
} // readMailbox()


#pragma mark -
#pragma mark Miscellaneous methods


UInt32 BCM5722D::computeEthernetCRC(const UInt8* address, int length)
{
  UInt32 remainder;
  UInt32 crc;
  int    i;
  int    j;

  crc = 0xFFFFFFFF;

  for (i = 0; i < length; i++) {

    crc ^= address[i];

    for (j = 0; j < 8; j++) {

      remainder = crc & 0x01;
      crc >>= 1;

      if (remainder) {
        crc ^= 0xEDB88320;
      }
    }
  }

  return ~crc;
} // computeEthernetCRC()
