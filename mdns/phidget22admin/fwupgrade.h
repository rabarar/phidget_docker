#ifndef PHIDGET22_FWUPGRADE_H
#define PHIDGET22_FWUPGRADE_H

#include "phidget22.h"
#ifdef _WIN32
#define PHIDGET22_API __declspec(dllimport)
#else
#define PHIDGET22_API
#endif

PHIDGET22_API const char * CCONV Phidget_strerror(PhidgetReturnCode);

PHIDGET22_API PhidgetReturnCode CCONV Phidget_getDeviceFirmwareUpgradeString (PhidgetHandle deviceOrChannel, const char **buffer);
PHIDGET22_API PhidgetReturnCode CCONV Phidget_getDeviceVINTID(PhidgetHandle deviceOrChannel, uint32_t *VINTID);
PHIDGET22_API PhidgetReturnCode CCONV Phidget_reboot (PhidgetHandle phid);
PHIDGET22_API PhidgetReturnCode CCONV Phidget_rebootFirmwareUpgrade (PhidgetHandle phid, uint32_t upgradeTimeout);

typedef struct _PhidgetFirmwareUpgrade *PhidgetFirmwareUpgradeHandle;
PHIDGET22_API PhidgetReturnCode CCONV PhidgetFirmwareUpgrade_create(PhidgetFirmwareUpgradeHandle *ch);
PHIDGET22_API PhidgetReturnCode CCONV PhidgetFirmwareUpgrade_delete(PhidgetFirmwareUpgradeHandle *ch);
PHIDGET22_API PhidgetReturnCode CCONV PhidgetFirmwareUpgrade_sendFirmware(PhidgetFirmwareUpgradeHandle ch, const uint8_t *data, size_t dataLen);
PHIDGET22_API PhidgetReturnCode CCONV PhidgetFirmwareUpgrade_getActualDeviceID(PhidgetFirmwareUpgradeHandle ch, Phidget_DeviceID *actualDeviceID);
PHIDGET22_API PhidgetReturnCode CCONV PhidgetFirmwareUpgrade_getActualDeviceName(PhidgetFirmwareUpgradeHandle ch, const char **actualDeviceName);
PHIDGET22_API PhidgetReturnCode CCONV PhidgetFirmwareUpgrade_getActualDeviceSKU(PhidgetFirmwareUpgradeHandle ch, const char **actualDeviceSKU);
PHIDGET22_API PhidgetReturnCode CCONV PhidgetFirmwareUpgrade_getActualDeviceVersion(PhidgetFirmwareUpgradeHandle ch, int *actualDeviceVersion);
PHIDGET22_API PhidgetReturnCode CCONV PhidgetFirmwareUpgrade_getActualDeviceVINTID(PhidgetFirmwareUpgradeHandle ch, uint32_t *actualDeviceVINTID);
PHIDGET22_API PhidgetReturnCode CCONV PhidgetFirmwareUpgrade_getProgress(PhidgetFirmwareUpgradeHandle ch, double *progress);

typedef void (CCONV *PhidgetFirmwareUpgrade_OnProgressChangeCallback)(PhidgetFirmwareUpgradeHandle ch, void *ctx, double progress);
PHIDGET22_API PhidgetReturnCode CCONV PhidgetFirmwareUpgrade_setOnProgressChangeHandler(PhidgetFirmwareUpgradeHandle ch, PhidgetFirmwareUpgrade_OnProgressChangeCallback fptr, void *ctx);

#endif
