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

enum
{
  kPowerStateOff = 0,
  kPowerStateOn,
  kPowerStateCount
};


#pragma mark -
#pragma mark IONetworkController Methods


IOReturn BCM5722D::getPacketFilters(const OSSymbol *group,
                                    UInt32 *filters) const
{
  if ((group == gIOEthernetWakeOnLANFilterGroup) &&
      magicPacketSupported) {

    *filters = kIOEthernetWakeOnMagicPacket;

    return kIOReturnSuccess;

  }

  return IOEthernetController::getPacketFilters(group, filters);
} // getPacketFilters()


IOReturn BCM5722D::setWakeOnMagicPacket(bool active)
{
  wakeOnLanEnabled = active;

  return kIOReturnSuccess;
} // setWakeOnMagicPacket()


IOReturn BCM5722D::registerWithPolicyMaker(IOService *policyMaker)
{
  static IOPMPowerState devicePowerState[kPowerStateCount] = {

    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 1, kIOPMDeviceUsable, kIOPMPowerOn, kIOPMPowerOn, 0, 0, 0, 0, 0, 0,
      0, 0 }

  };

  currentPowerState = kPowerStateOn;

  return policyMaker->registerPowerDriver(this,
                                          devicePowerState,
                                          kPowerStateCount);
} // registerWithPolicyMaker()


IOReturn BCM5722D::setPowerState(unsigned long powerStateOrdinal,
                                 IOService *policyMaker)
{
  if (powerStateOrdinal == currentPowerState) {
    return IOPMAckImplied;
  }

  DebugLog("Changing power state from %lu to %lu",
           currentPowerState, powerStateOrdinal);
  
  switch (powerStateOrdinal) {

    case kPowerStateOn:

      pciNub->configWrite16(kPCIPMCS, kPCIPMCSPMEStatus);
      IOSleep(10);

      break;

    case kPowerStateOff:

      IOOptionBits state = (wakeOnLanEnabled ?
                            kPCIPMCPMESupportFromD3Cold :
                            kPCIPMCD3Support);
      
      if (wakeOnLanEnabled) {
        prepareForWakeOnLanMode();
      }

      pciNub->hasPCIPowerManagement(state);

      break;
  }

  currentPowerState = powerStateOrdinal;

  return IOPMAckImplied;
} // setPowerState()


void BCM5722D::prepareForWakeOnLanMode()
{
  // not implemented
} // prepareForWakeOnLanMode()
