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

#ifndef BCM5722D_REGISTERS_H
#define BCM5722D_REGISTERS_H

#define BIT(x)    (1 << (x))

#define DEVICEID_BCM5722                  0x165A
#define DEVICEID_BCM5754                  0x167A
#define DEVICEID_BCM5754M                 0x1672
#define DEVICEID_BCM5755                  0x167B
#define DEVICEID_BCM5755M                 0x1673
#define DEVICEID_BCM5787                  0x169B
#define DEVICEID_BCM5787M                 0x1693
#define DEVICEID_BCM5906                  0x1712
#define DEVICEID_BCM5906M                 0x1713
#define DEVICEID_BCM57788                 0x1691

#define PHYID_MODEL_5754_5787             0x0E
#define PHYID_MODEL_5755                  0x0C
#define PHYID_MODEL_5906                  0x04

#define ASICREV_A                         0x0A /* BCM5755/M */
#define ASICREV_B                         0x0B /* BCM5754/M, BCM5787/M */
#define ASICREV_C                         0x0C /* BCM5906/M */

#define REVLEVEL_A0                       0x00
#define REVLEVEL_A1                       0x01
#define REVLEVEL_A2                       0x02

// Ring Control Block
#define RCB_HOSTADDR_HI                   0x00000000
#define RCB_HOSTADDR_LO                   0x00000004
#define RCB_MAXLEN_FLAGS                  0x00000008
#define RCB_MAXLEN_SHIFT                  16
#define RCB_FLAGS_RING_DISABLED           0x00000002
#define RCB_NICADDR                       0x0000000C

#define T3_MAGIC_NUMBER                   0x4B657654
#define DRV_WOL_SIGNATURE                 0x474C0000

// Memory Window Start Address
#define MEMWIN_START                      0x00008000

// Memory Map
#define SRAM_TX_RING_RCB                  0x00000100
#define SRAM_RXR_RING_RCB                 0x00000200
#define SRAM_RXR_RING_RCB_NEXT            0x00000010
#define SRAM_STATUS_BLOCK                 0x00000300
#define SRAM_TX_RING                      0x00004000
#define SRAM_RXP_RING                     0x00006000
#define SRAM_SOFT_GENCOMM                 0x00000B50
#define SRAM_WOL_MAILBOX                  0x00000D30

// Standard Mode
#define MEMORY_WINDOW_START               0x00008000
#define MEMORY_WINDOW_END                 0x0000FFFF

#pragma mark -
#pragma mark PCI Configuration Registers

enum {
  kPCIPMCapabilities = 0x4A,
  kPCIPMCS = 0x4C
};

#pragma mark -
#pragma mark Broadcom Vendor-Specific Capabilities

// Miscellaneous Host Control Register
#define BPCI_MISCHOSTCTL                  0x00000068
#define BPCI_MISCHOSTCTL_CLEARINTA        BIT(0)
#define BPCI_MISCHOSTCTL_MASKIRQOUT       BIT(1)
#define BPCI_MISCHOSTCTL_WORDSWAP         BIT(3)
#define BPCI_MISCHOSTCTL_STATERW          BIT(4)
#define BPCI_MISCHOSTCTL_CLOCKCTL         BIT(5)
#define BPCI_MISCHOSTCTL_INDACC           BIT(7)
#define BPCI_MISCHOSTCTL_TAGSTATMODE      BIT(9)
#define BPCI_MISCHOSTCTL_ASICREV_MASK     0xFFFF0000
#define BPCI_MISCHOSTCTL_ASICREV_SHFT     16

// DMA Read/Write Control Register
#define BPCI_DMARWCTL                     0x0000006C
#define BPCI_DMARWCTL_WMARK_MASK          0x00003800
#define BPCI_DMARWCTL_WMARK_32            0
#define BPCI_DMARWCTL_WMARK_64            BIT(19)
#define BPCI_DMARWCTL_WMARK_96            BIT(20)
#define BPCI_DMARWCTL_WMARK_128           BIT(19) | BIT(20)
#define BPCI_DMARWCTL_WMARK_160           BIT(21)
#define BPCI_DMARWCTL_WMRAK_192           BIT(19) | BIT(21)
#define BPCI_DMARWCTL_WMARK_224           BIT(20) | BIT(21)
#define BPCI_DMARWCTL_WMARK_256           BIT(19) | BIT(20) | BIT(21)

// PCI State Register
#define BPCI_STATE                        0x00000070
#define BPCI_STATE_EEPROM                 BIT(5)
#define BPCI_STATE_EEPROMRETRY            BIT(6)

// Memory Window Base Address Register
#define BPCI_MEMWINBASEADDR               0x0000007C

// Memory Window Data Register
#define BPCI_MEMWINDATA                   0x00000084


#pragma mark -
#pragma mark PCIe Capabilities

// Device Capabilities Register
#define PCIECAP_CAPS                      0x000000D4
#define PCIECAP_CAPS_MAXPLOAD_MASK        0x00000007
#define PCIECAP_CAPS_MAXPLOAD_128         0
#define PCIECAP_CAPS_MAXPLOAD_256         BIT(0)
#define PCIECAP_CAPS_MAXPLOAD_512         BIT(1)
#define PCIECAP_CAPS_MAXPLOAD_1024        BIT(0) | BIT(1)
#define PCIECAP_CAPS_MAXPLOAD_2048        BIT(2)
#define PCIECAP_CAPS_MAXPLOAD_4096        BIT(0) | BIT(2)

// Device Control Register
#define PCIECAP_DEVCTL                    0x000000D8
#define PCIECAP_DEVCTL_RELAXORD           BIT(4)
#define PCIECAP_DEVCTL_MAXPLOAD_MASK      BIT(5) | BIT(6) | BIT(7)
#define PCIECAP_DEVCTL_MAXPLOAD_128       0
#define PCIECAP_DEVCTL_NOSNOOP            BIT(11)

// Device Status Register
#define PCIECAP_DEVSTAT                   0x000000DA
#define PCIECAP_DEVSTAT_CED               BIT(0)
#define PCIECAP_DEVSTAT_NFED              BIT(1)
#define PCIECAP_DEVSTAT_FED               BIT(2)
#define PCIECAP_DEVSTAT_URD               BIT(3)
#define PCIECAP_DEVSTAT_APD               BIT(4)
#define PCIECAP_DEVSTAT_TP                BIT(5)


#pragma mark -
#pragma mark High-Priority Mailboxes

// Interrupt Mailbox 0
#define HPMBX_IRQ0_HI                     0x00000200
#define HPMBX_IRQ0_LO                     0x00000204

// Receive BD Standard Producer Ring Index Register
#define HPMBX_RXPIDX_HI                   0x00000268
#define HPMBX_RXPIDX_LO                   0x0000026C

// Receive BD Return Ring 0 Consumer Index Register
#define HPMBX_RXR1IDX_HI                  0x00000280
#define HPMBX_RXR1IDX_LO                  0x00000284

// Receive BD Return Ring 1 Consumer Index Register
#define HPMBX_RXR2IDX_HI                  0x00000288
#define HPMBX_RXR2IDX_LO                  0x0000028C

// Receive BD Return Ring 2 Consumer Index Register
#define HPMBX_RXR3IDX_HI                  0x00000290
#define HPMBX_RXR3IDX_LO                  0x00000294

// Receive BD Return Ring 3 Consumer Index Register
#define HPMBX_RXR4IDX_HI                  0x00000298
#define HPMBX_RXR4IDX_LO                  0x0000029C

// Send BD Ring Host Producer Index Register
#define HPMBX_TXPIDX_HI                   0x00000300
#define HPMBX_TXPIDX_LO                   0x00000304


#pragma mark Ethernet MAC Control Registers

// Ethernet MAC Mode Register
#define EMAC_MODE                         0x00000400
#define EMAC_MODE_HALFDUPLEX              BIT(1)
#define EMAC_MODE_PORTMODE_MASK           0x0000000C
#define EMAC_MODE_PORTMODE_NONE           0
#define EMAC_MODE_PORTMODE_MII            BIT(2)
#define EMAC_MODE_PORTMODE_GMII           BIT(3)
#define EMAC_MODE_PORTMODE_TBI            BIT(2) | BIT(3)
#define EMAC_MODE_RXSTATSENA              BIT(11)
#define EMAC_MODE_RXSTATSCLR              BIT(12)
#define EMAC_MODE_TXSTATSENA              BIT(14)
#define EMAC_MODE_TXSTATSCLR              BIT(15)
#define EMAC_MODE_MAGICPACKETDETECT       BIT(18)
#define EMAC_MODE_ACPIPOWERON             BIT(19)
#define EMAC_MODE_TDE                     BIT(21)
#define EMAC_MODE_RDE                     BIT(22)
#define EMAC_MODE_FHDE                    BIT(23)

// Ethernet MAC Status Register
#define EMAC_STATUS                       0x00000404
#define EMAC_STATUS_CONFIGCHGD            BIT(3)
#define EMAC_STATUS_SYNCCHGD              BIT(4)
#define EMAC_STATUS_LNKSTATECHGD          BIT(12)
#define EMAC_STATUS_MICOMPLETION          BIT(22)

// Ethernet MAC Event Enable Register
#define EMAC_EVENT                        0x00000408
#define EMAC_EVENT_LNKSTATECHGD           BIT(12)
#define EMAC_EVENT_MICOMPLETION           BIT(22)
#define EMAC_EVENT_MIINTERRUPT            BIT(23)

// LED Control Register
#define EMAC_LEDCTL                       0x0000040C
#define EMAC_LEDCTL_OVERRIDELINK          BIT(0)
#define EMAC_LEDCTL_1000MBPS              BIT(1)
#define EMAC_LEDCTL_100MBPS               BIT(2)
#define EMAC_LEDCTL_10MBPS                BIT(3)
#define EMAC_LEDCTL_OVERRIDETRAFFIC       BIT(4)
#define EMAC_LEDCTL_BLINKTRAFFIC          BIT(5)
#define EMAC_LEDCTL_TRAFFIC               BIT(6)
#define EMAC_LEDCTL_1000MBPSSTAT          BIT(7)
#define EMAC_LEDCTL_100MBPSSTAT           BIT(8)
#define EMAC_LEDCTL_10MBPSSTAT            BIT(9)
#define EMAC_LEDCTL_TRAFFICSTAT           BIT(10)
#define EMAC_LEDCTL_MODE_MASK             BIT(11) | BIT(12)
#define EMAC_LEDCTL_MODE_MAC              0
#define EMAC_LEDCTL_MODE_PHY1             BIT(11)
#define EMAC_LEDCTL_MODE_PHY2             BIT(12)
#define EMAC_LEDCTL_MODE_COMBO            BIT(11) | BIT(12)
#define EMAC_LEDCTL_MACMODE               BIT(13)
#define EMAC_LEDCTL_SHAREDMODE            BIT(14)
#define EMAC_LEDCTL_COMBOMODE             BIT(15)
#define EMAC_LEDCTL_BLINKPERIOD_MASK      0x7FF80000
#define EMAC_LEDCTL_OVERRIDEBLINK         BIT(31)

// Ethernet MAC Address Registers
#define EMAC_MACADDR0_HI                  0x00000410
#define EMAC_MACADDR0_LO                  0x00000414
#define EMAC_MACADDR1_HI                  0x00000418
#define EMAC_MACADDR1_LO                  0x0000041C
#define EMAC_MACADDR2_HI                  0x00000420
#define EMAC_MACADDR2_LO                  0x00000424
#define EMAC_MACADDR3_HI                  0x00000428
#define EMAC_MACADDR3_LO                  0x0000042C

// Ethernet Transmit Random Backoff Register
#define EMAC_TXRANDBACKOFF                0x00000438

// Receive MTU Size Register
#define EMAC_RXMTUSIZE                    0x0000043C

// MI Communication Register
#define EMAC_MICOMM                       0x0000044C
#define EMAC_MICOMM_DATA                  0x0000FFFF
#define EMAC_MICOMM_REG(x)                ((x & 0x1F) << 16)
#define EMAC_MICOMM_PHY(x)                ((x & 0x1F) << 21)
#define EMAC_MICOMM_CMD_WRITE             BIT(26)
#define EMAC_MICOMM_CMD_READ              BIT(27)
#define EMAC_MICOMM_START                 BIT(29)
#define EMAC_MICOMM_BUSY                  BIT(29)

// MI Status Register
#define EMAC_MISTAT                       0x00000450
#define EMAC_MISTAT_LINKSTAT              BIT(0)

// MI Mode Register
#define EMAC_MIMODE                       0x00000454
#define EMAC_MIMODE_PORTPOLL              BIT(4)
#define EMAC_MIMODE_CLOCK_66MHZ           0x000C0000

// Transmit MAC Mode Register
#define EMAC_TXMODE                       0x0000045C
#define EMAC_TXMODE_ENABLE                BIT(1)
#define EMAC_TXMODE_FLOWCTL               BIT(4)
#define EMAC_TXMODE_LOCKUPFIX             BIT(8)

// Transmit MAC Length Register
#define EMAC_TXLEN                        0x00000464

// Receive MAC Mode Register
#define EMAC_RXMODE                       0x00000468
#define EMAC_RXMODE_RESET                 BIT(0)
#define EMAC_RXMODE_ENABLE                BIT(1)
#define EMAC_RXMODE_FLOWCTL               BIT(2)
#define EMAC_RXMODE_ACCEPTRUNTS           BIT(6)
#define EMAC_RXMODE_PROMISCMODE           BIT(8)
#define EMAC_RXMODE_NOCRCCHECK            BIT(9)
#define EMAC_RXMODE_IPV6PARSING           BIT(24)

// Receive MAC Status Register
#define EMAC_RXSTAT                       0x0000046C

// MAC Hash Register
#define EMAC_HASH_0                       0x00000470
#define EMAC_HASH_1                       0x00000474
#define EMAC_HASH_2                       0x00000478
#define EMAC_HASH_3                       0x0000047C

// Receive Rules Configuration Register
#define EMAC_RXRULESCONF                  0x00000500
#define EMAC_RXRULESCONF_DEF01            0x00000008

// Low Watermark Maximum Receive Frames Register
#define EMAC_LOWATMAXRXFRAMES             0x00000504


#pragma mark -
#pragma mark Statistics Registers

// Transmit MAC Statistics Counters
#define TXSTAT_IFHCOUTOCTETS              0x00000800
#define TXSTAT_ETHERSTATSCOLLISIONS       0x00000808
#define TXSTAT_OUTXONSENT                 0x0000080C
#define TXSTAT_OUTXOFFSENT                0x00000810
#define TXSTAT_DOT3INTERNALMACTXERR       0x00000818
#define TXSTAT_DOT3SINGLECOLLFRAMES       0x0000081C
#define TXSTAT_DOT3MULTICOLLFRAMES        0x00000820
#define TXSTAT_DOT3DEFERREDTX             0x00000824
#define TXSTAT_DOT3EXCESSIVECOLL          0x0000082C
#define TXSTAT_DOT3LATECOLL               0x00000830
#define TXSTAT_IFHCOUTUCASTPKTS           0x0000086C
#define TXSTAT_IFHCOUTMCASTPKTS           0x00000870
#define TXSTAT_IFHCOUTBCASTPKTS           0x00000874
#define TXSTAT_DOT3CARRIERSENSEERR        0x00000878
#define TXSTAT_IFOUTDISCARDS              0x0000087C

// Receive MAC Statistics Counters
#define RXSTAT_IFHCINOCTETS               0x00000880
#define RXSTAT_ETHERSTATSFRAGMENTS        0x00000888
#define RXSTAT_IFHCINUCASTPKTS            0x0000088C
#define RXSTAT_IFHCINMCASTPKTS            0x00000890
#define RXSTAT_IFHCINBCASTPKTS            0x00000894
#define RXSTAT_DOT3FCSERR                 0x00000898
#define RXSTAT_DOT3ALIGNMENTERR           0x0000089C
#define RXSTAT_XONPAUSEFRAMESRCVD         0x000008A0
#define RXSTAT_XOFFPAUSEFRAMESRCVD        0x000008A4
#define RXSTAT_MACCTLFRAMESRCVD           0x000008A8
#define RXSTAT_XOFFSTATEENTERED           0x000008AC
#define RXSTAT_DOT3FRAMESTOOLONGS         0x000008B0
#define RXSTAT_ETHERSTATSJABBERS          0x000008B4
#define RXSTAT_ETHERSTATSUNDERSZPKTS      0x000008B8


#pragma mark -
#pragma mark Send Data Initiator Control Registers

// Send Data Initiator Mode Register
#define TXDI_MODE                         0x00000C00
#define TXDI_MODE_ENABLE                  BIT(1)

// Send Data Initiator Statistics Control Registers
#define TXDI_STATSCTL                     0x00000C08
#define TXDI_STATSCTL_ENABLE              BIT(0)
#define TXDI_STATSCTL_FASTUPDATE          BIT(1)

// Send Data Initiator Statistics Enable Mask Register
#define TXDI_STATSENABLE                  0x00000C0C

// ISO Packet Transmit Support Register
#define TXDI_TXISOPKT                     0x00000C20


#pragma mark -
#pragma mark Send Data Completion Control Registers

// Send Data Completion Mode Register
#define TXDC_MODE                         0x00001000
#define TXDC_MODE_ENABLE                  BIT(1)


#pragma mark -
#pragma mark Send BD Ring Selector Control Registers

// Send BD Ring Selector Mode Register
#define TXBDS_MODE                        0x00001400
#define TXBDS_MODE_ENABLE                 BIT(1)
#define TXBDS_MODE_ATTN                   BIT(2)


#pragma mark -
#pragma mark Send BD Initiator Control Registers

// Send BD Initiator Mode Register
#define TXBDI_MODE                        0x00001800
#define TXBDI_MODE_ENABLE                 BIT(1)
#define TXBDI_MODE_ATTN                   BIT(2)


#pragma mark -
#pragma mark Send BD Completion Control Registers

// Send BD Completion Mode Register
#define TXBDC_MODE                        0x00001C00
#define TXBDC_MODE_ENABLE                 BIT(1)
#define TXBDC_MODE_ATTN                   BIT(2)


#pragma mark -
#pragma mark Receive List Placement Control Registers

// Receive List Placement Mode Register
#define RXLP_MODE                         0x00002000
#define RXLP_MODE_ENABLE                  BIT(1)

// Receive List Placement Configuration Register
#define RXLP_CONF                         0x00002010

// Receive List Placement Statistics Control Register
#define RXLP_STATSCTL                     0x00002014
#define RXLP_STATSCTL_ENABLE              BIT(0)

// Receive List Placement Statistics Enable Mask Register
#define RXLP_STATSENABLE                  0x00002018
#define RXLP_STATSENABLE_DACKFIX          BIT(18)


#pragma mark -
#pragma mark Receive Data and Receive BD Initiator Control Registers

// Receive Data and Receive BD Initiator Mode Register
#define RXDBDI_MODE                       0x00002400
#define RXDBDI_MODE_ENABLE                BIT(1)
#define RXDBDI_MODE_ILLEGALRTNRINGSZ      BIT(4)

// Standard Receive BD Ring RCB Register
#define RXDBDI_RXPRCB_HOSTADDR_HI         0x00002450
#define RXDBDI_RXPRCB_HOSTADDR_LO         0x00002454
#define RXDBDI_RXPRCB_MAXLEN              0x00002458
#define RXDBDI_RXPRCB_NICADDR             0x0000245C


#pragma mark -
#pragma mark Receive Data Completion Control Registers

// Receive Data Completion Mode Register
#define RXDC_MODE                         0x00002800
#define RXDC_MODE_ENABLE                  BIT(1)
#define RXDC_MODE_ATTN                    BIT(2)


#pragma mark -
#pragma mark Receive BD Initiator Control Registers

// Receive BD Initiator Mode
#define RXBDI_MODE                        0x00002C00
#define RXBDI_MODE_ENABLE                 BIT(1)
#define RXBDI_MODE_ATTN                   BIT(2)

// Standard Receive BD Producer Ring Replenish Threshold Register
#define RXBDI_RXBDPRINGTHOLD              0x00002C18


#pragma mark
#pragma mark Receive BD Completion Control Registers

// Receive BD Completion Mode Register
#define RXBDC_MODE                        0x00003000
#define RXBDC_MODE_ENABLE                 BIT(1)
#define RXBDC_MODE_ATTN                   BIT(2)


#pragma mark -
#pragma mark Host Coalescing Control Registers

// Host Coalescing Mode Register
#define HCC_MODE                          0x00003C00
#define HCC_MODE_ENABLE                   BIT(1)
#define HCC_MODE_COALNOW                  BIT(3)
#define HCC_MODE_STATBLKSIZE_32           BIT(8)
#define HCC_MODE_CLEARTICKSONRX           BIT(9)
#define HCC_MODE_CLEARTICKSONTX           BIT(10)

// Receive Coalescing Ticks Register
#define HCC_RXCOALTICKS                   0x00003C08

// Send Coalescing Ticks Register
#define HCC_TXCOALTICKS                   0x00003C0C

// Receive Max Coalesced BD Count
#define HCC_RXMAXCOALBDCNT                0x00003C10
#define HCC_TXMAXCOALBDCNT                0x00003C14
#define HCC_RXMAXCOALBDCNTIRQ             0x00003C20
#define HCC_TXMAXCOALBDCNTIRQ             0x00003C24

// Status Block Host Address Register
#define HCC_STATUSBLOCKHOSTADDR_HI        0x00003C38
#define HCC_STATUSBLOCKHOSTADDR_LO        0x00003C3C

// Flow Attention Register
#define HCC_FLOWATTN                      0x00003C48


#pragma mark -
#pragma mark Memory Arbiter Registers

// Memory Arbiter Mode Register
#define MAR_MODE                          0x00004000
#define MAR_MODE_ENABLE                   BIT(1)


#pragma mark -
#pragma mark Buffer Manager Control Registers

// Buffer Manager Mode Register
#define BMC_MODE                          0x00004400
#define BMC_MODE_ENABLE                   BIT(1)
#define BMC_MODE_ATTN                     BIT(2)

// Buffer Manager Status Register
#define BMC_STAT                          0x00004404

// Read DMA Mbuf Low Watermark Register
#define BMC_RDMAMBUFLOWAT                 0x00004410

// MAC Rx Mbuf Low Watermark Register
#define BMC_RXMBUFLOWAT                   0x00004414

// Mbuf High Watermark Register
#define BMC_MBUFHIWAT                     0x00004418

// DMA Descriptor Pool Low Watermark Register
#define BMC_DMADESCRLOWAT                 0x00004434

// DMA Descriptor Pool High Watermark Register
#define BMC_DMADESCRHIWAT                 0x00004438


#pragma mark -
#pragma mark Read DMA Control Registers

// Read DMA Mode Register
#define RDMA_MODE                         0x00004800
#define RDMA_MODE_ENABLE                  BIT(1)
#define RDMA_MODE_TGTABORTATTN            BIT(2)
#define RDMA_MODE_MSTABORTATTN            BIT(3)
#define RDMA_MODE_PARABORTATTN            BIT(4)
#define RDMA_MODE_HOSTADDROFLOWATTN       BIT(5)
#define RDMA_MODE_FIFOORUNATTN            BIT(6)
#define RDMA_MODE_FIFOURUNATTN            BIT(7)
#define RDMA_MODE_FIFOOREADATTN           BIT(8)
#define RDMA_MODE_WLONGERLENATTN          BIT(9)
#define RDMA_MODE_BURSTLEN_LONG           BIT(16) | BIT(17)

// Read DMA Status Register
#define RDMA_STATUS                       0x00004804


#pragma mark -
#pragma mark Write DMA Control Registers

// Write DMA Mode Register
#define WDMA_MODE                         0x00004C00
#define WDMA_MODE_ENABLE                  BIT(1)
#define WDMA_MODE_TGTABORTATTN            BIT(2)
#define WDMA_MODE_MSTABORTATTN            BIT(3)
#define WDMA_MODE_PARABORTATTN            BIT(4)
#define WDMA_MODE_HOSTADDROFLOWATTN       BIT(5)
#define WDMA_MODE_FIFOORUNATTN            BIT(6)
#define WDMA_MODE_FIFOURUNATTN            BIT(7)
#define WDMA_MODE_FIFOOWRITEATTN          BIT(8)
#define WDMA_MODE_WLONGERLENATTN          BIT(9)
#define WDMA_MODE_STATTAGFIX              BIT(29)

// Write DMA Status Register
#define WDMA_STATUS                       0x00004C04


#pragma mark -
#pragma mark RX RISC Registers

// RX RISC State Register
#define RXRISC_STATE                      0x00005004


#pragma mark -
#pragma mark Virtual CPU Registers

// VCPU Status Register
#define VCPU_STATUS                       0x00005100
#define VCPU_STATUS_INITDONE              BIT(26)
#define VCPU_STATUS_DRIVERRST             BIT(27)

// Device Configuration Shadow Register
#define VCPU_CONFSHADOW                   0x00005104
#define VCPU_CONFSHADOW_WOL               BIT(0)
#define VCPU_CONFSHADOW_MAGICPKT          BIT(2)
#define VCPU_CONFSHADOW_ASPM_DBNC         BIT(12)


#pragma mark -
#pragma mark Low-Priority Mailboxes

// Interrupt Mailbox 0 Register
#define LPMBX_IRQ0_HI                     0x00005800
#define LPMBX_IRQ0_LO                     0x00005804


#pragma mark -
#pragma mark Flow-Through Queues

// FTQ Reset Register
#define FTQ_RESET                         0x00005C00


#pragma mark -
#pragma mark Message Signaled Interrupt Registers

// MSI Mode Register
#define MSI_MODE                          0x00006000
#define MSI_MODE_ENABLE                   BIT(1)

// MSI Status Register
#define MSI_STATUS                        0x00006004


#pragma mark -
#pragma mark General Control Registers

// Mode Control Register
#define GCR_MODECTL                       0x00006800
#define GCR_MODECTL_BSWAPNONFRAME         BIT(1)
#define GCR_MODECTL_WSWAPNONFRAME         BIT(2)
#define GCR_MODECTL_BSWAP                 BIT(4)
#define GCR_MODECTL_WSWAP                 BIT(5)
#define GCR_MODECTL_HOSTSTACKUP           BIT(16)
#define GCR_MODECTL_HOSTSENDBD            BIT(17)
#define GCR_MODECTL_NOTXPHDRCSUM          BIT(20)
#define GCR_MODECTL_NORXPHDRCSUM          BIT(23)
#define GCR_MODECTL_IRQONMACATTN          BIT(26)

// Miscellaneous Configuration Register
#define GCR_MISCCFG                       0x00006804
#define GCR_MISCCFG_CORECLOCKRST          BIT(0)
#define GCR_MISCCFG_TIMERPSCALERMASK      0x000000FE
#define GCR_MISCCFG_TIMERPSCALERSHFT      1
#define GCR_MISCCFG_PHYIDDQ               BIT(21)
#define GCR_MISCCFG_GPHYPWRDWN            BIT(26)
#define GCR_MISCCFG_DISABLEGRCRST         BIT(29)
#define GCR_MISCCFG_TP_66MHZ              (0x41 << GCR_MISCCFG_TIMERPSCALERSHFT)

// Miscellaneous Local Control Register
#define GCR_MISCLCLCTL                    0x00006808
#define GCR_MISCLCLCTL_IRQONATTN          BIT(3)
#define GCR_MISCLCLCTL_GPIO_UART_SEL      BIT(4)
#define GCR_MISCLCLCTL_GPIO_OUTE0         BIT(11)
#define GCR_MISCLCLCTL_GPIO_OUTE1         BIT(12)
#define GCR_MISCLCLCTL_GPIO_OUTE2         BIT(13)
#define GCR_MISCLCLCTL_GPIO_OUT0          BIT(14)
#define GCR_MISCLCLCTL_GPIO_OUT1          BIT(15)
#define GCR_MISCLCLCTL_GPIO_OUT2          BIT(16)
#define GCR_MISCLCLCTL_AUTOSEEPROMACC     BIT(24)


#pragma mark -
#pragma mark Wake-on-LAN Registers

// Miscellaneous CableSense Control Register
#define WOL_MISCCSCTL                     0x00006890
#define WOL_MISCCSCTL_VCPUHALT            BIT(22)

// Fast Boot Program Counter Register
#define GCR_FASTBOOT                      0x00006894


#pragma mark -
#pragma mark Non-Volatile Memory Interface Registers

// Software Arbitration Register
#define NVM_SOFTARB                       0x00007020
#define NVM_SOFTARB_REQ_SET1              BIT(1)
#define NVM_SOFTARB_REQ_CLR1              BIT(5)
#define NVM_SOFTARB_ARB_WON1              BIT(9)


#pragma mark -
#pragma mark PCIe Registers

// TLP Control Register
#define PCIE_TLPCTL                       0x00007C00
#define PCIE_TLPCTL_DATAFIFOPROTECT       BIT(25)
#define PCIE_TLPCTL_IRQMODEFIX            BIT(29)

// Transaction Control Register
#define PCIE_TRANSCTL                     0x00007C04
#define PCIE_TRANSCTL_LOMCONF             BIT(5)
#define PCIE_TRANSCTL_1SHOTMSI            BIT(29)

// PHY Test Control Register
#define PCIE_TSTCTL                       0x00007E2C
#define PCIE_TSTCTL_SCRAMBLER             BIT(5)
#define PCIE_TSTCTL_10MODE                BIT(6)


#pragma mark -
#pragma mark Transceiver Registers

// MII Control Register
#define PHY_MIICTL                        0x00
#define PHY_MIICTL_SPEED_10               0
#define PHY_MIICTL_SPEED_100              BIT(6)
#define PHY_MIICTL_SPEED_1000             BIT(13)
#define PHY_MIICTL_DUPLEX_FULL            BIT(8)
#define PHY_MIICTL_RESTARTAUTONEG         BIT(9)
#define PHY_MIICTL_AUTONEGENABLE          BIT(12)
#define PHY_MIICTL_LOOPBACK               BIT(14)
#define PHY_MIICTL_RESET                  BIT(15)

// MII Status Register
#define PHY_MIISTAT                       0x01
#define PHY_MIISTAT_LINKSTAT              BIT(2)
#define PHY_MIISTAT_AUTONEGCOMPLETE       BIT(5)

// PHY Identifier Register #1
#define PHY_ID1                           0x02

// PHY Identifier Register #2
#define PHY_ID2                           0x03

// Auto-Negotiation Advertisement Register
#define PHY_AUTONEGADVERT                 0x04
#define PHY_AUTONEGADVERT_SELECTMASK      0x001F
#define PHY_AUTONEGADVERT_802_3           BIT(0)
#define PHY_AUTONEGADVERT_10HD            BIT(5)
#define PHY_AUTONEGADVERT_10FD            BIT(6)
#define PHY_AUTONEGADVERT_100HD           BIT(7)
#define PHY_AUTONEGADVERT_100FD           BIT(8)
#define PHY_AUTONEGADVERT_PAUSECAP        BIT(10)
#define PHY_AUTONEGADVERT_ASYMPAUSE       BIT(11)

// Auto-Negotiation Link Partner Ability Register
#define PHY_AUTONEGPARTNER                0x05
#define PHY_AUTONEGPARTNER_10HD           BIT(5)
#define PHY_AUTONEGPARTNER_10FD           BIT(6)
#define PHY_AUTONEGPARTNER_100HD          BIT(7)
#define PHY_AUTONEGPARTNER_100FD          BIT(8)
#define PHY_AUTONEGPARTNER_PAUSECAP       BIT(10)
#define PHY_AUTONEGPARTNER_ASYMPAUSE      BIT(11)

// 1000BASE-T Control Register
#define PHY_1000BASETCTL                  0x09
#define PHY_1000BASETCTL_ADVERTHD         BIT(8)
#define PHY_1000BASETCTL_ADVERTFD         BIT(9)

// 1000BASE-T Status Register
#define PHY_1000BASETSTS                  0x0A
#define PHY_1000BASETSTS_PARTNERHD        BIT(10)
#define PHY_1000BASETSTS_PARTNERFD        BIT(11)

#define PHY_DSPRWPORT                     0x15

#define PHY_DSPADDR                       0x17

// Auxiliary Control Register
#define PHY_AUXCTL                        0x18

// Normal
#define PHY_AUXCTL_NORMAL                 0x0

// 10BASE-T
#define PHY_AUXCTL_10BASET                BIT(0)

// Power Control
#define PHY_AUXCTL_PWRCTL                 BIT(1)

// Miscellaneous Test 1
#define PHY_AUXCTL_MISCTEST1              BIT(2)

// Miscellaneous Control
#define PHY_AUXCTL_MISCCTL                BIT(0) | BIT(1) | BIT(2)
#define PHY_AUXCTL_MISCCTL_AUTOMDIX       BIT(9)
#define PHY_AUXCTL_MISCCTL_READ           BIT(12) | BIT(13) | BIT(14)
#define PHY_AUXCTL_MISCCTL_WRITE          BIT(15)

// Auxiliary Status Summary Register
#define PHY_AUXSTAT                       0x19
#define PHY_AUXSTAT_LINKSTAT              BIT(2)
#define PHY_AUXSTAT_SPDDPLXMASK           0x0700
#define PHY_AUXSTAT_10HD                  BIT(8)
#define PHY_AUXSTAT_10FD                  BIT(9)
#define PHY_AUXSTAT_100HD                 BIT(8) | BIT(9)
#define PHY_AUXSTAT_100T4                 BIT(10)
#define PHY_AUXSTAT_100FD                 BIT(8) | BIT(10)
#define PHY_AUXSTAT_1000HD                BIT(9) | BIT(10)
#define PHY_AUXSTAT_1000FD                BIT(8) | BIT(9) | BIT(10)
#define PHY_AUXSTAT_AUTONEGCOMPLETE       BIT(15)

// Interrupt Status Register
#define PHY_IRQSTAT                       0x1A

// Interrupt Mask Register
#define PHY_IRQMASK                       0x1B

#define PHY_TEST1                         0x1E

#define PHY5906_TEST                      0x1F
#define PHY5906_TEST_SHADOW               BIT(7)

#define PHY5906_MISCCTL                   0x10
#define PHY5906_MISCCTL_AUTOMDIX          BIT(14)

#define PHY5906_PTEST                     0x17


#pragma mark -
#pragma mark Flags

// MAC Flags
#define FLAG_DEVICE_IS_NIC                BIT(0)
#define FLAG_EEPROM_WRITE_PROTECT         BIT(1)
#define FLAG_ASPM_WORKAROUND              BIT(2)
#define FLAG_WOL_CAP                      BIT(3)
#define FLAG_WOL_ENABLE                   BIT(4)

// PHY Flags
#define PHYFLAG_FAST_ETHERNET             BIT(0)
#define PHYFLAG_LOW_POWER                 BIT(1)
#define PHYFLAG_ETHERNET_WIRESPEED        BIT(2)
#define PHYFLAG_WOL_SPEED_100             BIT(3)
#define PHYFLAG_BUG_ADJUST_TRIM           BIT(14)
#define PHYFLAG_BUG_JITTER                BIT(15)

#endif
