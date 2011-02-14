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

#ifndef BCM5722D_PHY_H
#define BCM5722D_PHY_H

#define PHYID_OUI(id)     (((id) >> 10) & 0x0000FFFF)
#define PHYID_MODEL(id)   (((id) >> 4) & 0x0000003F)
#define PHYID_REV(id)     ((id) & 0x0000000F)

#define MBit 1000000

enum FlowControl
{
  kFlowControlDisabled = 0,
  kFlowControlRx,
  kFlowControlTx,
  kFlowControlSymmetric,
};


enum
{
  kMediumTypeIndexAuto = 0,
  kMediumTypeIndex10HD,
  kMediumTypeIndex10FD,
  kMediumTypeIndex100HD,
  kMediumTypeIndex100FD,
  kMediumTypeIndex1000HD,
  kMediumTypeIndex1000FD,
  kMediumTypeCount,
};


enum LinkDuplex
{
  kLinkDuplexFull,
  kLinkDuplexHalf,
  kLinkDuplexNegotiate,
  kLinkDuplexNone
};


enum LinkSpeed
{
  kLinkSpeedNone = 0,
  kLinkSpeed10 = 10,
  kLinkSpeed100 = 100,
  kLinkSpeed1000 = 1000,
  kLinkSpeedNegotiate
};


enum LinkState
{
  kLinkStateUp,
  kLinkStateDown
};


enum
{
  kAutoNegotiationDelay = 50,
  kAutoNegotiationTimeout = 10000
};


struct MediaStatus
{
  LinkDuplex  duplex;
  LinkSpeed   speed;
  LinkState   state;
  FlowControl flowControl;
};

#endif
