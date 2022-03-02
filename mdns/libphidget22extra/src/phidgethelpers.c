#include "phidget22extra/phidgethelpers.h"

const char * CCONV
getErrorStr(PhidgetReturnCode code) {
	PhidgetReturnCode res;
	const char *str;

	res = Phidget_getErrorDescription(code, &str);
	if (res != EPHIDGET_OK)
		return ("<unknown return code>");
	return (str);
}

int CCONV
getDeviceVersion(PhidgetHandle phid) {
	int ver;

	if (Phidget_getDeviceVersion(phid, &ver) != 0)
		return (-1);
	return (ver);
}

int CCONV
getDeviceSerialNumber(PhidgetHandle phid) {
	int sn;

	if (Phidget_getDeviceSerialNumber(phid, &sn) != 0)
		return (-1);
	return (sn);
}

const char * CCONV
getDeviceName(PhidgetHandle phid) {
	const char *res;

	if (Phidget_getDeviceName(phid, &res) != 0)
		return ("<invalid phidget>");
	return (res);
}

const char * CCONV
getDeviceClassName(PhidgetHandle phid) {
	const char *res;

	if (Phidget_getDeviceClassName(phid, &res) != 0)
		return ("<invalid phidget>");
	return (res);
}

const char * CCONV
getChannelClassName(PhidgetHandle phid) {
	const char *res;

	if (Phidget_getChannelClassName(phid, &res) != 0)
		return ("<invalid phidget>");
	return (res);
}

const char * CCONV
getDeviceLabel(PhidgetHandle phid) {
	const char *res;

	if (Phidget_getDeviceLabel(phid, &res) != 0)
		return ("<invalid phidget>");
	return (res);
}

Phidget_DeviceID CCONV
getDeviceID(PhidgetHandle phid) {
	Phidget_DeviceID did;

	if (Phidget_getDeviceID(phid, &did) != 0)
		return (PHIDID_NOTHING);
	return (did);
}

Phidget_DeviceClass CCONV
getDeviceClass(PhidgetHandle phid) {
	Phidget_DeviceClass dc;

	if (Phidget_getDeviceClass(phid, &dc) != 0)
		return (PHIDCLASS_NOTHING);
	return (dc);
}

Phidget_ChannelClass CCONV
getChannelClass(PhidgetHandle phid) {
	Phidget_ChannelClass dc;

	if (Phidget_getChannelClass(phid, &dc) != 0)
		return (PHIDCHCLASS_NOTHING);
	return (dc);
}

int CCONV
isChannel(PhidgetHandle phid) {
	int res;

	if (Phidget_getIsChannel(phid, &res) != 0)
		return (0);
	return (res);
}

int CCONV
getChannel(PhidgetHandle phid) {
	int res;

	if (Phidget_getChannel(phid, &res) != 0)
		return (-1);
	return (res);
}

int CCONV
isRemote(PhidgetHandle phid) {
	int res;

	if (Phidget_getIsRemote(phid, &res) != 0)
		return (-1);
	return (res);
}

int CCONV
isHubPortDevice(PhidgetHandle phid) {
	int res;

	if (Phidget_getIsHubPortDevice(phid, &res) != 0)
		return (-1);
	return (res);
}

int CCONV
getHubPort(PhidgetHandle phid) {
	int res;

	if (Phidget_getHubPort(phid, &res) != 0)
		return (-1);
	return (res);
}