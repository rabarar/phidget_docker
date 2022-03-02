#ifndef _PHIDGET_HELPERS_H_
#define _PHIDGET_HELPERS_H_

#include "phidget22extra_int.h"

PHIDAPI const char * CCONV getDeviceName(PhidgetHandle phid);
PHIDAPI int CCONV getDeviceVersion(PhidgetHandle phid);
PHIDAPI int CCONV getDeviceSerialNumber(PhidgetHandle phid);
PHIDAPI Phidget_DeviceClass CCONV getDeviceClass(PhidgetHandle phid);
PHIDAPI const char * CCONV getDeviceClassName(PhidgetHandle phid);
PHIDAPI Phidget_ChannelClass CCONV getChannelClass(PhidgetHandle phid);
PHIDAPI const char * CCONV getChannelClassName(PhidgetHandle phid);
PHIDAPI const char * CCONV getDeviceLabel(PhidgetHandle phid);
PHIDAPI Phidget_DeviceID CCONV getDeviceID(PhidgetHandle phid);
PHIDAPI int CCONV isChannel(PhidgetHandle phid);
PHIDAPI int CCONV getChannel(PhidgetHandle phid);
PHIDAPI int CCONV isRemote(PhidgetHandle phid);
PHIDAPI int CCONV isHubPortDevice(PhidgetHandle phid);
PHIDAPI int CCONV getHubPort(PhidgetHandle phid);
PHIDAPI const char * CCONV getErrorStr(PhidgetReturnCode);

#endif /* _PHIDGET_HELPERS_H_ */