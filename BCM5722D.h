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

#ifndef BCM5722D_BCM5722D_H
#define BCM5722D_BCM5722D_H

#include <IOKit/pci/IOPCIDevice.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IONetworkStats.h>
#include <IOKit/network/IOEthernetStats.h>
#include <IOKit/network/IOMbufMemoryCursor.h>
#include <IOKit/network/IONetworkMedium.h>
#include <IOKit/network/IOBasicOutputQueue.h>

#include "Common.h"
#include "PHY.h"

#define BCM5722D my_name_adlan_BCM5722D

#define BUMP_NETSTAT(x) do { networkStats->x += 1; } while (0)
#define BUMP_ETHSTAT(x) do { ethernetStats->dot3StatsEntry.x += 1; } while (0)
#define BUMP_TXSTAT(x) do { ethernetStats->dot3TxExtraEntry.x += 1; } while (0)
#define BUMP_RXSTAT(x) do { ethernetStats->dot3RxExtraEntry.x += 1; } while (0)

enum
{
  kMaxPacketSize     = kIOEthernetMaxPacketSize +  4,
  kTxBDCount         = 512,
  kRxBDCount         = 512,
  kTxMaxSegmentCount = 384,  // 0.75 *ring size(512) = 384
  kRxMaxSegmentCount = 1,
  kWatchDogTimeout   = 5000  // ms
};

enum BOption
{
  kOptionDefault = 0,
  kBDOptionReuse              // configureRxDescriptor option
};

class BCM5722D : public IOEthernetController
{
  OSDeclareDefaultStructors(BCM5722D);

 private:
  IOPCIDevice            *pciNub;
  IOMemoryMap            *csrMap;
  IOWorkLoop             *workLoop;
  volatile void          *csrBase;
  IOInterruptEventSource *interruptSource;
  IOEthernetInterface    *netIface;
  IOTimerEventSource     *timerSource;
  IOEthernetAddress       ethAddress;
  IOOutputQueue          *transmitQueue;
  IONetworkStats         *networkStats;
  IOEthernetStats        *ethernetStats;
  OSDictionary           *mediumDict;

  IOBufferMemoryDescriptor  *txMD;
  BTxBufferDescriptor       *txBD;
  IOPhysicalAddress          txAddress;
  IOMbufNaturalMemoryCursor *txCursor;
  mbuf_t                     txPacketArray[kTxBDCount];

  IOBufferMemoryDescriptor  *rxMD;
  BRxBufferDescriptor       *rxBD;
  IOPhysicalAddress          rxAddress;
  IOMbufNaturalMemoryCursor *rxCursor;
  mbuf_t                     rxPacketArray[kRxBDCount];

  IOBufferMemoryDescriptor *rxReturnMD;
  BRxBufferDescriptor      *rxReturnBD;
  IOPhysicalAddress         rxReturnAddress;

  IOBufferMemoryDescriptor *statusBlockMD;
  BStatusBlock             *statusBlock;
  IOPhysicalAddress         statusBlockAddress;

  UInt16        asicRevision;
  UInt16        deviceID;
  UInt32        txQueueLength;
  bool          magicPacketSupported;
  bool          wakeOnLanEnabled;
  bool          adapterEnabled;
  bool          interruptEnabled;
  bool          promiscuousModeEnabled;
  unsigned long currentPowerState;

  UInt16 txProducerIdx;
  UInt16 txLocalConsumerIdx;
  UInt16 txFreeSlot;

  UInt16 rxProducerIdx;
  UInt16 rxReturnConsumerIdx;
  UInt16 rxSegmentLength[kRxBDCount];

  UInt32 rxMode;
  UInt32 txMode;
  UInt32 macMode;
  UInt32 pciMiscHostControl;

  bool        autoNegotiate;
  UInt8       currentMediumIndex;
  UInt32      phyID;
  UInt16      phyFlags;
  MediaStatus media;

 public:

#pragma mark -
#pragma mark IOService Methods

  virtual bool start(IOService *provider);
  virtual void stop(IOService *provider);
  virtual void free(void);

#pragma mark -
#pragma mark IONetworkController Methods

  virtual IOReturn        enable(IONetworkInterface *iface);
  virtual IOReturn        disable(IONetworkInterface *iface);
  virtual bool            createWorkLoop();
  virtual IOWorkLoop     *getWorkLoop(void) const;
  virtual IOOutputQueue  *createOutputQueue();
  virtual bool            configureInterface(IONetworkInterface *iface);
  virtual IOReturn        getChecksumSupport(UInt32 *checksumMask,
                                             UInt32 checksumFamily,
                                             bool isOutput);
  virtual IOReturn        getPacketFilters(const OSSymbol *group,
                                           UInt32 *filters) const;
  virtual IOReturn        getMaxPacketSize(UInt32 *maxSize) const;
  virtual IOReturn        registerWithPolicyMaker(IOService *policyMaker);
  virtual IOReturn        setPowerState(unsigned long powerStateOrdinal,
                                        IOService *policyMaker);
  virtual UInt32          outputPacket(mbuf_t m,
                                       void *param);
  virtual IOReturn        selectMedium(const IONetworkMedium *medium);
  virtual const OSString *newModelString() const;
  virtual const OSString *newRevisionString() const;
  virtual const OSString *newVendorString() const;

#pragma mark -
#pragma mark IOEthernetController Methods

  virtual IOReturn getHardwareAddress(IOEthernetAddress *address);
  virtual IOReturn setHardwareAddress(const IOEthernetAddress *address);
  virtual IOReturn setWakeOnMagicPacket(bool active);
  virtual IOReturn setMulticastMode(bool active);
  virtual IOReturn setMulticastList(IOEthernetAddress *addrs,
                                    UInt32 count);
  virtual IOReturn setPromiscuousMode(bool active);

#pragma mark -
#pragma mark MAC
#pragma mark -

#pragma mark -
#pragma mark Initialization/Reset

  bool     setupDriver(IOService *provider);
  bool     resetAdapter();
  void     initializePCIConfig();
  void     prepareDriver();
  bool     initializeAdapter();
  void     stopAdapter();
  IOReturn lockNVRAM();
  void     configureMACAddress();
  void     clearStatistics();
  void     updateStatistics();


#pragma mark -
#pragma mark Memory/Ring

  bool allocateDescriptorMemory(IOBufferMemoryDescriptor **memory,
                                IOByteCount size);
  void freeDescriptorMemory(IOBufferMemoryDescriptor **memory);
  bool allocateDriverMemory();
  bool configureRxDescriptor(UInt16 index, BOption options = kOptionDefault);
  bool initTxRing();
  void freeTxRing();
  bool initRxRing();
  void freeRxRing();


#pragma mark -
#pragma mark Interrupt Handling

  void enableInterrupts(bool active);
  void interruptOccurred(IOInterruptEventSource *source, int count);
  void timeoutOccurred(IOTimerEventSource *source);
  void serviceTxInterrupt(UInt16 consumerIdx);
  void serviceRxInterrupt(UInt16 producerIdx);


#pragma mark -
#pragma mark Reader/Writer

  void     writeCSR(UInt32 offset, UInt32 value);
  UInt32   readCSR(UInt32 offset);
  void     writeMemoryIndirect(UInt32 offset, UInt32 value);
  UInt32   readMemoryIndirect(UInt32 offset);
  void     writeMailbox(UInt32 offset, UInt32 value);
  UInt32   readMailbox(UInt32 offset);
  void     clearBit(UInt32 offset, UInt32 bit);
  void     setBit(UInt32 offset, UInt32 bit);


#pragma mark -
#pragma mark Helper

  UInt32 computeEthernetCRC(const UInt8 *address, int length);
  void   prepareForWakeOnLanMode();


#pragma mark -
#pragma mark PHY
#pragma mark -

#pragma mark -
#pragma mark Initialization/Reset/Fixes

  bool     probePHY();
  bool     setupPHY(bool reset = false);
  IOReturn resetPHY();
  void     acknowledgeInterrupt();
  void     configureMAC(LinkSpeed speed, LinkDuplex duplex);
  void     enableLoopback();
  void     fixJitterBug();
  void     fixAdjustTrim();


#pragma mark -
#pragma mark Medium

  void     addNetworkMedium(UInt32 type,
                            UInt32 speed,
                            UInt32 index);
  void     probeMediaCapability();
  IOReturn setMedium(const IONetworkMedium *medium);


#pragma mark -
#pragma mark Link

  UInt16   resolvePauseAdvertisement(FlowControl flowControl);
  void     setupFlowControl(UInt16 local, UInt16 partner);
  void     configureLinkAdvertisement(LinkSpeed linkSpeed,
                                      LinkDuplex linkDuplex);
  bool     startAutoNegotiation(LinkSpeed changeSpeed,
                                LinkDuplex changeDuplex);
  bool     forceLinkSpeedDuplex(LinkSpeed changeSpeed,
                                LinkDuplex changeDuplex);
  void     resolveOperatingSpeedAndLinkDuplex(UInt16 status,
                                              LinkSpeed *speed,
                                              LinkDuplex *duplex);
  void     enableAutoMDIX(bool active);
  void     enableEthernetAtWirespeed();
  void     reportLinkStatus();


#pragma mark -
#pragma mark Interrupt Handling

  void serviceLinkInterrupt();


#pragma mark -
#pragma mark Reader/Writer

  IOReturn writeMII(UInt8 reg, UInt16 value);
  IOReturn readMII(UInt8 reg, UInt16 *value);

};

#endif
