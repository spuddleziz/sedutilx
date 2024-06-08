/* C:B**************************************************************************
   This software is Copyright (c) 2014-2024 Bright Plaza Inc. <drivetrust@drivetrust.com>

   This file is part of sedutil.

   sedutil is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   sedutil is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with sedutil.  If not, see <http://www.gnu.org/licenses/>.

   * C:E********************************************************************** */

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <SEDKernelInterface/SEDKernelInterface.h>
#include "os.h"
#include "log.h"
#include "fnmatch.h"
#include "DtaEndianFixup.h"
#include "DtaHexDump.h"
#include "DtaDevMacOSDrive.h"
#include "DtaMacOSConstants.h"



bool DtaDevMacOSDrive::isDtaDevMacOSDriveDevRef(const char * devref)
{
    OSDEVICEHANDLE h = DtaDevMacOSDrive::openDeviceHandle(devref);
    if (INVALID_HANDLE_VALUE == h) {
        return false;
    }
    DtaDevMacOSDrive::closeDeviceHandle(h);
    return true;
}



OSDEVICEHANDLE DtaDevMacOSDrive::openAndCheckDeviceHandle(const char * devref)
{
  OSDEVICEHANDLE osDeviceHandle = openDeviceHandle(devref);

  if (INVALID_HANDLE_VALUE == osDeviceHandle) {
      LOG(D1) << "Error opening device " << devref << " -- not found";
  }
  return osDeviceHandle;
}



namespace fs = std::__fs::filesystem;
#undef USEDRIVERUSPERCLASS
#define USEBLOCKSTORAGEDEVICE
OSDEVICEHANDLE DtaDevMacOSDrive::openDeviceHandle(const char * devref)
{
    std::string bsdName = fs::path(devref).stem();
    io_registry_entry_t mediaService = findBSDName(bsdName.c_str());
    if (!mediaService)
        return INVALID_HANDLE_VALUE;

    io_registry_entry_t driverService = findDriverInParents(mediaService);
    if (driverService != IO_OBJECT_NULL) {
        LOG(D4) << "Is TPer; found driver service in parents";
        io_connect_t connection = IO_OBJECT_NULL;
        kern_return_t  kernResult = OpenUserClient(driverService, &connection);
        if (kernResult != kIOReturnSuccess || connection == IO_OBJECT_NULL) {
          LOG(E) << "Failed to open user client -- error=0x" << std::hex << std::setw(8) << kernResult;
        }
        else {
          LOG(D4)
            << "Driver service " << driverService << "=0x" << std::hex << std::setw(4) << driverService << std::dec
            << " "
            << "-- opened user client " << connection << "=0x" << std::hex << std::setw(4) << connection;
        }
        IOObjectRelease(mediaService);
        return handle(driverService,connection);
    }

#if defined(USEDRIVERUSPERCLASS)
    dS = findDriverSuperClassInParents(mediaService);
    if (dS != IO_OBJECT_NULL)  {
        LOG(D4) << "Is not TPer; found block storage device service in parents";
        LOG(D4) << "Driver service " << dS << "=0x" << std::hex << std::setw(4) << dS << std::dec
                << " "
                << "-- did not open user client --"
                << " "
                << "isTPer=false";
        IOObjectRelease(mediaService);
        return INVALID_HANDLE_VALUE;
    }
#endif // defined(USEDRIVERUSPERCLASS)

#if defined(USEBLOCKSTORAGEDEVICE)
    driverService = findBlockStorageDeviceInParents(mediaService);
    if (driverService != IO_OBJECT_NULL)  {
        LOG(D4) << "Is not TPer; found block storage device service in parents";
        LOG(D4)
          << "Driver service " << driverService << "=0x" << std::hex << std::setw(4) << driverService << std::dec
          << " "
          << "-- did not open user client"
        ;
        IOObjectRelease(mediaService);
        return INVALID_HANDLE_VALUE;
    }

    IOObjectRelease(mediaService);
    return INVALID_HANDLE_VALUE;
#endif // defined(USEBLOCKSTORAGEDEVICE)
}

void DtaDevMacOSDrive::closeDeviceHandle(OSDEVICEHANDLE osDeviceHandle)
{
    if (osDeviceHandle == INVALID_HANDLE_VALUE) return;

    io_registry_entry_t connection = handleConnection(osDeviceHandle);
    if ( connection != IO_OBJECT_NULL ) {
        LOG(D4) << "Releasing connection";
        kern_return_t ret = CloseUserClient(connection);
        if ( kIOReturnSuccess != ret) {
            LOG(E) << "CloseUserClient returned " << ret;
        }
    }

    io_connect_t driverService = handleDriverService(osDeviceHandle);
    if ( driverService != IO_OBJECT_NULL ) {
        LOG(D4) << "Releasing driver service";
        IOObjectRelease(driverService);
    }
}



std::vector<std::string> DtaDevMacOSDrive::enumerateDtaDevMacOSDriveDevRefs()
{
    std::vector<std::string> devices;

    for (int i = 0; i < MAX_DISKS; i++) {
        char devref[16];
        snprintf(devref, sizeof(devref), "\\dev\\disk%i", i);
        if (isDtaDevOSDriveDevRef(devref))
            devices.push_back(std::string(devref));
    }

    return devices;
}



uint8_t DtaDevMacOSDrive::discovery0(DTA_DEVICE_INFO & disk_info) {
    io_connect_t connect = handleConnection(osDeviceHandle);
    io_registry_entry_t driverService = handleDriverService(osDeviceHandle);
    return ((connect != IO_OBJECT_NULL && driverService != IO_OBJECT_NULL &&
             KERN_SUCCESS == TPerUpdate(connect, driverService, &disk_info)))
            ? DTAERROR_SUCCESS
            : DTAERROR_COMMAND_ERROR;
}
