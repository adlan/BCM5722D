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

#ifndef BCM5722D_DEFINES_H
#define BCM5722D_DEFINES_H

#include "Register.h"
#include "HW.h"
#include "BCM5722D.h"


#ifdef _DEBUG

#define DebugLog(fmt, args...) \
    IOLog("BCM5722D (%s:%d): "fmt"\n", __FUNCTION__, __LINE__, ## args)
#define LogReg(reg) \
    DebugLog(#reg": 0x%08X", readCSR(reg))

#else

  #define DebugLog(fmt, args...)
  #define LogReg(reg)

#endif

#define Log(fmt, args...) \
    IOLog("%s: "fmt"\n", getName(), ## args)

#define GET_ASICREV(x)    ((x) >> 12)
#define GET_REVID(x)      ((x) & 0xF)
#define INCREMENT(x, y)   (x) = (x + 1) & (y - 1)
#define HOSTADDRESS_HI(x) ((UInt64)(x) >> 32)
#define HOSTADDRESS_LO(x) ((UInt64)(x) & 0xFFFFFFFFULL)

#endif
