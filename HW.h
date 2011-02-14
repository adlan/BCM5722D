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

#ifndef BCM5722D_HW_H
#define BCM5722D_HW_H

#include <IOKit/IOTypes.h>

// Transmit buffer descriptor

struct BTxBufferDescriptor
{
  UInt32 addressHigh;
  UInt32 addressLow;
  UInt16 flags;
  UInt16 length;
  UInt16 vlanTag;
  UInt16 mss;
};

// Transmit BD Flags
#define TXBDFLAG_TCP_UDP_CHECKSUM         BIT(0)
#define TXBDFLAG_IP_CHECKSUM              BIT(1)
#define TXBDFLAG_PACKET_END               BIT(2)
#define TXBDFLAG_VLAN_TAG                 BIT(6)
#define TXBDFLAG_COAL_NOW                 BIT(7)
#define TXBDFLAG_CPU_PRE_DMA              BIT(8)
#define TXBDFLAG_CPU_POST_DMA             BIT(9)
#define TXBDFLAG_DONT_GEN_CRC             BIT(15)


// Receive buffer descriptor

struct BRxBufferDescriptor
{
  UInt32 addressHigh;
  UInt32 addressLow;
  UInt16 length;
  UInt16 index;
  UInt16 flags;
  UInt16 type;
  UInt16 tcpUDPCksum;
  UInt16 ipCksum;
  UInt16 vlanTag;
  UInt16 errorFlags;
  UInt32 rssHash;
  UInt32 opaque;
};

// Receive BD Flags
#define RXBDFLAG_PACKET_END               BIT(2)
#define RXBDFLAG_RSS_HASH_VALID           BIT(3)
#define RXBDFLAG_VLAN_TAG                 BIT(6)
#define RXBDFLAG_FRAME_HAS_ERROR          BIT(10)
#define RXBDFLAG_IP_CHECKSUM              BIT(12)
#define RXBDFLAG_TCP_UDP_CHECKSUM         BIT(13)
#define RXBDFLAG_TCP_UDP_IS_TCP           BIT(14)
#define RXBDFLAG_IP_VERSION               BIT(15)

// Receive BD Flags Errors
#define RXBDERROR_BAD_CRC                 BIT(0)
#define RXBDERROR_COLL_DETECT             BIT(1)
#define RXBDERROR_LINK_LOST               BIT(2)
#define RXBDERROR_PHY_DECODE_ERR          BIT(3)
#define RXBDERROR_ODD_NIBBLE_RX_MII       BIT(4)
#define RXBDERROR_MAC_ABORT               BIT(5)
#define RXBDERROR_LEN_LESS_64             BIT(6)
#define RXBDERROR_TRUNC_NO_RES            BIT(7)
#define RXBDERROR_GIANT_PKT_RCVD          BIT(8)


// Status block

struct BStatusBlock
{
  UInt32 statusWord;
  UInt32 statusTag;
  UInt16 rxReturn2ProducerIdx;
  UInt16 rxConsumerIdx;
  UInt16 rxReturn4ProducerIdx;
  UInt16 rxReturn3ProducerIdx;
  UInt16 rxReturn1ProducerIdx;
  UInt16 txConsumerIdx;
};

// Status Block Status
#define STATUS_WORD_UPDATED               0x00000001
#define STATUS_WORD_LNKCHGD               0x00000002
#define STATUS_WORD_ERROR                 0x00000004

#endif
