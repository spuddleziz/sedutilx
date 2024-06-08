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

#include <string>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFDictionary.h>
#include "DtaDevMacOSDrive.h"
#include <IOKit/storage/IOMedia.h>
#include <SEDKernelInterface/SEDKernelInterface.h>

/** Factory functions
 *
 * Static class members of DtaDevOSDrive that are passed through
 * to DtaDevMacOSDrive
 *
 */

bool DtaDevOSDrive::isDtaDevOSDriveDevRef(const char * devref) {
  return DtaDevMacOSDrive::isDtaDevMacOSDriveDevRef(devref);
}

std::vector<std::string> DtaDevOSDrive::enumerateDtaDevOSDriveDevRefs() {
  return DtaDevMacOSDrive::enumerateDtaDevMacOSDriveDevRefs();
}

DtaDevOSDrive * DtaDevOSDrive::getDtaDevOSDrive(const char * devref,
                                                DTA_DEVICE_INFO &device_info)
{
  return static_cast<DtaDevOSDrive *>(DtaDevMacOSDrive::getDtaDevMacOSDrive(devref, device_info));
}



// #include "DtaDevMacOSAta.h"
#include "DtaDevMacOSSata.h"
#include "DtaDevMacOSScsi.h"
// #include "DtaDevMacOSNvme.h"


/** Factory functions
 *
 * Static class members that support instantiation of subclass members
 * with the subclass switching logic localized here for easier maintenance.
 *
 */





typedef std::map<std::string, std::string>dictionary;

static inline void overlay(dictionary & dest, dictionary & source) {
    for (const auto& p : source) dest[p.first] = p.second;
}
/**
 * Converts a CFString to a UTF-8 std::string if possible.
 *
 * @param input A reference to the CFString to convert.
 * @return Returns a std::string containing the contents of CFString converted to UTF-8. Returns
 *  an empty string if the input reference is null or conversion is not possible.
 */
// Modified from https://gist.githubusercontent.com/peter-bloomfield/1b228e2bb654702b1e50ef7524121fb9/raw/934184166a8c3ff403dd5d7f8c0003810014f73d/cfStringToStdString.cpp per comments
static
std::string cfStringToStdString(CFStringRef input, bool & error)
{
    error = false;
    if (!input)
        return {};

    // Attempt to access the underlying buffer directly. This only works if no conversion or
    //  internal allocation is required.
    auto originalBuffer{ CFStringGetCStringPtr(input, kCFStringEncodingUTF8) };
    if (originalBuffer)
        return originalBuffer;

    // Copy the data out to a local buffer.
    CFIndex lengthInUtf16{ CFStringGetLength(input) };
    CFIndex maxLengthInUtf8{ CFStringGetMaximumSizeForEncoding(lengthInUtf16,
        kCFStringEncodingUTF8) + 1 }; // <-- leave room for null terminator
    std::vector<char> localBuffer((size_t)maxLengthInUtf8);

    if (CFStringGetCString(input, localBuffer.data(), maxLengthInUtf8, kCFStringEncodingUTF8))
        return localBuffer.data();

    error = true;
    return {};
}

#include "log.h"
#include "DtaDevMacOSDrive.h"
// Create a copy of the properties of this I/O registry entry
// Receiver owns this CFMutableDictionary instance if not NULL
static
void collectProperties(CFDictionaryRef cfproperties, dictionary * properties) {
   CFDictionaryApplyFunction(cfproperties,
                              [](const void *vkey, const void *vvalue, void * vproperties){
                              
                                  dictionary * properties = (dictionary *)vproperties;
                                  
                                  // Get the key --  should be a string
                                  std::string key, value="<\?\?\?>";
                                  CFTypeID keyTypeID = CFGetTypeID(vkey);
                                  if (CFStringGetTypeID() == keyTypeID) {
                                      bool error=false;
                                      key = cfStringToStdString(reinterpret_cast<CFStringRef>(vkey), error);
                                      if (error) {
                                          LOG(E) << "Failed to get key as string " << HEXON(sizeof(const void *)) << vkey;
                                          return;
                                      }
                                  } else {
                                     LOG(E) << "Unrecognized key type " << (CFTypeRef)vkey;
                                     return;
                                  };
                                  
                                  // Get the value -- could be a Bool, Dict, Data, String, or Number
                                  CFTypeID valueTypeID = CFGetTypeID(vvalue);
                                  if (CFStringGetTypeID() == valueTypeID) {
                                      // String
                                      bool error=false;
                                      value = cfStringToStdString(reinterpret_cast<CFStringRef>(vvalue), error);
                                      if (error) {
                                          LOG(E) << "Failed to get key as string " << HEXON(sizeof(const void *)) << vkey;
                                          return;
                                      }
                                  } else if (CFBooleanGetTypeID() == valueTypeID) {
                                      // Bool
                                      value = std::string(CFBooleanGetValue(reinterpret_cast<CFBooleanRef>(vvalue)) ? "true" : "false");
                                  } else if (CFNumberGetTypeID() == valueTypeID) {
                                      // Number
                                      if (CFNumberIsFloatType(reinterpret_cast<CFNumberRef>(vvalue))) {
                                          // Float
                                          double dvalue=0.0;
                                          bool error=CFNumberGetValue(reinterpret_cast<CFNumberRef>(vvalue), kCFNumberDoubleType, (void *)&dvalue);
                                          if (error) {
                                              LOG(E) << "Failed to get value as float " << HEXON(sizeof(vvalue)) << vvalue;
                                              return;
                                          }
                                          value = std::to_string(dvalue);
                                      } else {
                                          // Integer
                                          long long llvalue=0LL;
                                          bool error=CFNumberGetValue(reinterpret_cast<CFNumberRef>(vvalue), kCFNumberLongLongType, (void *)&llvalue);
                                          if (error) {
                                              LOG(E) << "Failed to get value as float " << HEXON(sizeof(vvalue)) << vvalue;
                                              return;
                                          }
                                          value = std::to_string(llvalue);
                                      }
                                  } else if (CFDataGetTypeID() == valueTypeID) {
                                      // Data
                                  } else if (CFDictionaryGetTypeID() == valueTypeID) {
                                      // Dict
                                      collectProperties(reinterpret_cast<CFDictionaryRef>(vvalue), properties);
                                      return;
                                  } else {
                                      // Unknown
                                      LOG(E) << "Failed to get value " << HEXON(sizeof(vvalue)) << vvalue << " with type ID "  << HEXON(sizeof(valueTypeID)) << valueTypeID;
                                      return;
                                  }
                                          
                                  (*properties)[key]=value;
                              },
                              (void *)properties);
}

static
dictionary * copyProperties(io_service_t service) {
    CFMutableDictionaryRef cfproperties = NULL;
    if (KERN_SUCCESS != IORegistryEntryCreateCFProperties(service,
                                                          &cfproperties,
                                                          CFAllocatorGetDefault(),
                                                          0)) {
        return NULL;
    }
    dictionary * properties = new dictionary;
    collectProperties(cfproperties, properties);
    return properties;
}


static
dictionary* getOSSpecificInformation(OSDEVICEHANDLE h, 
                                     const char* /* devref  *** TODO -- UNUSED *** */,
                                     InterfaceDeviceID& /* interfaceDeviceIdentification  *** TODO -- UNUSED *** */,
                                     DTA_DEVICE_INFO& device_info)
{
	dictionary* allProperties = new dictionary;
    
    io_service_t deviceEntry;
    dictionary * deviceProperties;

    io_service_t media;
    dictionary * mediaProperties;

    io_service_t tPer;
    dictionary * tPerProperties;

    deviceEntry=handleDriverService(h);
    if (IO_OBJECT_NULL == deviceEntry) {
        goto finishedWithDevice;
    }
    deviceProperties = copyProperties(deviceEntry);
    if (NULL == deviceProperties) {
        goto finishedWithDevice;
    }
    
    overlay(*allProperties, *deviceProperties);
    
    media = findServiceForClassInChildren(deviceEntry, kIOMediaClass);
    if (IO_OBJECT_NULL == media) {
        goto finishedWithMedia;
    }
    mediaProperties = copyProperties( media );
    if (NULL == mediaProperties ) {
        goto finishedWithMedia ;
    }
    
    overlay(*allProperties, *mediaProperties);

    tPer = findParent(deviceEntry);
    if (IOObjectConformsTo(tPer, kDriverClass)) {
        tPerProperties = copyProperties( tPer );
        overlay(*allProperties, *tPerProperties);
    }
    IOObjectRelease(tPer);



    device_info.devType = DEVICE_TYPE_OTHER;

//    // We get some information to fill in to the device information struct as
//    // defaults in case other efforts are fruitless
//	copyIfAvailable(VendorId, vendorID, "vendorID");
//	copyIfAvailable(ProductId, modelNum, "modelNum");
//	copyIfAvailable(ProductRevision, firmwareRev, "firmwareRev");
//	copyIfAvailable(SerialNumber, serialNum, "serialNum");
//
//	{
//		unsigned char* p = interfaceDeviceIdentification;
//#define copyIDField(field, length) do { memcpy(p, device_info.field, length); p += length; } while (0)
//		copyIDField(vendorID,    INQUIRY_VENDOR_IDENTIFICATION_Length );
//		copyIDField(modelNum,    INQUIRY_PRODUCT_IDENTIFICATION_Length);
//		copyIDField(firmwareRev, INQUIRY_PRODUCT_REVISION_LEVEL_Length);
//	}
//
//
//	switch (descriptor.BusType) {
//	case BusTypeAta:
//		LOG(/* TODO D4 */ D1) << devref << " descriptor.BusType = BusTypeAta (" << descriptor.BusType << ")";
//		property["busType"] = "ATA";
//		device_info.devType = DEVICE_TYPE_ATA;
//		break;
//
//	case BusTypeSata:
//		LOG(/* TODO D4 */ D1) << devref << " descriptor.BusType = BusTypeSata (" << descriptor.BusType << ")";
//		property["busType"] = "SATA";
//		device_info.devType = DEVICE_TYPE_ATA;
//		break;
//
//	case BusTypeUsb:
//		LOG(/* TODO D4 */ D1) << devref << " descriptor.BusType = BusTypeUsb (" << descriptor.BusType << ")";
//		property["busType"] = "USB";
//		device_info.devType = DEVICE_TYPE_USB;
//		break;
//
//	case BusTypeNvme:
//		LOG(/* TODO D4 */ D1) << devref << " descriptor.BusType = BusTypeNvme (" << descriptor.BusType << ")";
//		property["busType"] = "NVME";
//		device_info.devType = DEVICE_TYPE_NVME;
//		break;
//
//	case BusTypeRAID:
//		LOG(/* TODO D4 */ D1) << devref << " descriptor.BusType = BusTypeRAID (" << descriptor.BusType << ")";
//		property["busType"] = "RAID";
//		device_info.devType = DEVICE_TYPE_OTHER;
//		break;
//
//	case BusTypeSas:
//		LOG(/* TODO D4 */ D1) << devref << " descriptor.BusType = BusTypeSas (" << descriptor.BusType << ")";
//		property["busType"] = "SAS";
//		device_info.devType = DEVICE_TYPE_SAS;
//		break;
//
//	default:
//		LOG(/* TODO D4 */ D1) << devref << " has UNKNOWN descriptor.BusType " << descriptor.BusType << "?!";
//		property["busType"] = "UNKN";
//		device_info.devType = DEVICE_TYPE_OTHER;
//		break;
//	}
//

	// We can fill in the size (capacity) of the device regardless of its type
	//
//	if (DeviceIoControl(h,                 // handle to device
//		IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, // dwIoControlCode
//		NULL,                             // lpInBuffer
//		0,                                // nInBufferSize
//		&dg,             // output buffer
//		sizeof(dg),           // size of output buffer
//		&BytesReturned,        // number of bytes returned
//		NULL)) {
//		device_info.devSize = dg.DiskSize.QuadPart;
//		LOG(/* TODO D4 */ D1) << devref << " size = " << device_info.devSize;
//	}
//	else {
//		device_info.devSize = 0;
//		LOG(/* TODO D4 */ D1) << devref << " size is UNKNOWN";
//	}
//	char buffer[100];
//	snprintf(buffer, sizeof(buffer), "%llu", device_info.devSize);
//	property["size"] = std::string(buffer);


finishedWithMedia:
    IOObjectRelease(media);

finishedWithDevice:

    return allProperties;
}



DtaDevMacOSDrive * DtaDevMacOSDrive::getDtaDevMacOSDrive(const char * devref,
                                                         DTA_DEVICE_INFO &device_info)
{
  OSDEVICEHANDLE osDeviceHandle = DtaDevMacOSDrive::openDeviceHandle(devref);
  if (INVALID_HANDLE_VALUE == osDeviceHandle) {
      return NULL;
  }

  DtaDevMacOSDrive* drive = NULL;
  InterfaceDeviceID interfaceDeviceIdentification;
  memset(interfaceDeviceIdentification, 0, sizeof(interfaceDeviceIdentification));
  LOG(/* TODO D4 */ D1) << devref << " driveParameters:";
  dictionary * driveParameters = getOSSpecificInformation(osDeviceHandle, devref, interfaceDeviceIdentification, device_info);
  IFLOG(/* TODO D4 */ D1)
	for (const auto & pair : *driveParameters) {
	  LOG(/* TODO D4 */ D1) << pair.first << ":\"" << pair.second << "\"";
	}
  DtaDevMacOSDrive::closeDeviceHandle(osDeviceHandle);

  if (driveParameters == NULL) {
	  LOG(E) << "Failed to determine drive parameters for " << devref;
	  return NULL;
  }
 
  //if ( (drive = DtaDevMacOSNvme::getDtaDevMacOSNvme(devref, disk_info)) != NULL )
  //  return drive ;
  ////  LOG(D4) << "DtaDevMacOSNvme::getDtaDevMacOSNvme(\"" << devref <<  "\", disk_info) returned NULL";

#define trySubclass(variant) \
  if ((drive = DtaDevMacOS##variant::getDtaDevMacOS##variant(devref, device_info)) != NULL) \
  { \
	break; \
  } else

#define logSubclassFailed(variant) \
  LOG(/* TODO D4 */ D1) << "DtaDevMacOS" #variant "::getDtaDevMacOS" #variant "(\"" << devref << "\", disk_info) returned NULL";

#define skipSubclass(variant) \
  LOG(/* TODO D4 */ D1) << "DtaDevMacOS" #variant "::getDtaDevMacOS" #variant "(\"" << devref << "\", disk_info) unimplmented";


  // Create a subclass instance based on device_info.devType as determined by
  // getOSSpecificInformation.  Customizing code has device_info and
  // drive parameters available.
  //
switch (device_info.devType) {
  case DEVICE_TYPE_SCSI:  // SCSI
  case DEVICE_TYPE_SAS:   // SCSI
	  trySubclass(Scsi)
      break;

  case DEVICE_TYPE_USB:   // UAS SAT -- USB -> SCSI -> AT pass-through
//  case DEVICE_TYPE_SATA:  // synonym
	  if (!deviceNeedsSpecialAction(interfaceDeviceIdentification,avoidSlowSATATimeout)) {
	    trySubclass(Sata);
	  }
	  if (!deviceNeedsSpecialAction(interfaceDeviceIdentification,avoidSlowSASTimeout)) {
		  trySubclass(Scsi);
	  }
	  break;

  case DEVICE_TYPE_NVME:  // NVMe
	  // TODO: Just hack by using Scsi for now.  BlockStorageDevice?
	  if (deviceNeedsSpecialAction(interfaceDeviceIdentification, acceptPseudoDeviceImmediately)) {
		  trySubclass(Scsi);
		  break;
	  }

      skipSubclass(Nvme)   // TODO test
	  break;

  case DEVICE_TYPE_ATA:   // SATA / PATA
      skipSubclass(Ata)    // TODO
	  break;

  case DEVICE_TYPE_OTHER:
	  LOG(E) << "Unimplemented device type " << devref;
  default:
      break;
  }

  delete driveParameters;
  return drive ;
}
