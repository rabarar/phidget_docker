#include "phidget22extra/projectsupport.h"

const char * CCONV
getPhidgetExtraVersion(void) {

	return (PHIDGETEXTRA_VERSION);
}

#if defined(_WINDOWS)

PhidgetReturnCode CCONV UTF16toUTF8(char *in, int inBytes, char *out); // from Phidget22

const char * CCONV
getComputerName(char *ubuf, size_t len, const char *def) {
	int err;
	DWORD slen;

	slen = (DWORD)len;
	err = GetComputerNameA(ubuf, &slen);
	if (err == 0)
		mos_strlcpy(ubuf, def, len);

	return (ubuf);
}
#elif defined(_LINUX) || defined(_FREEBSD)
const char *
getComputerName(char *ubuf, size_t len, const char *def) {
	int err;

	err = gethostname(ubuf, len);
	if (err != 0)
		mos_strlcpy(ubuf, def, len);

	return (ubuf);
}
#elif defined(_MACOSX)

#include <SystemConfiguration/SCDynamicStoreCopySpecific.h>

const char *
getComputerName(char *ubuf, size_t len, const char *def) {
	CFStringRef		hostname;

	hostname = SCDynamicStoreCopyComputerName(NULL, NULL);
	if (hostname) {
		CFStringGetCString(hostname, ubuf, len, kCFStringEncodingUTF8);
		CFRelease(hostname);
		if (ubuf[0] == '\0')
			mos_strlcpy(ubuf, def, len);
	} else {
		mos_strlcpy(ubuf, def, len);
	}
	return (ubuf);
}
#endif /* _WINDOWS */

Phidget_LogLevel CCONV
getLogLevel(const char *level) {

	if (mos_strcasecmp(level, "verbose") == 0)
		return (PHIDGET_LOG_VERBOSE);
	if (mos_strcasecmp(level, "debug") == 0)
		return (PHIDGET_LOG_DEBUG);
	if (mos_strcasecmp(level, "info") == 0)
		return (PHIDGET_LOG_INFO);
	if (mos_strncasecmp(level, "warn", 4) == 0)
		return (PHIDGET_LOG_WARNING);
	if (mos_strncasecmp(level, "err", 3) == 0)
		return (PHIDGET_LOG_ERROR);
	if (mos_strncasecmp(level, "crit", 4) == 0)
		return (PHIDGET_LOG_CRITICAL);

	return (PHIDGET_LOG_INFO);
}

PhidgetReturnCode CCONV
setChannelProjectMatchS(PhidgetHandle ch, const char *match) {
	ProjectMatch pm;
	PhidgetReturnCode res;

	res = parseProjectMatch(match, &pm);
	if (res != EPHIDGET_OK)
		return (res);

	return (setChannelProjectMatch(ch, &pm));
}

PhidgetReturnCode CCONV
setChannelProjectMatch(PhidgetHandle ch, ProjectMatchHandle pm) {
	PhidgetReturnCode res;

	if (pm->serialnumber != -1) {
		res = Phidget_setDeviceSerialNumber(ch, pm->serialnumber);
		if (res != EPHIDGET_OK)
			return (res);
	}
	if (pm->hubport != -1) {
		res = Phidget_setHubPort(ch, pm->hubport);
		if (res != EPHIDGET_OK)
			return (res);
	}
	if (pm->hubportmode) {
		res = Phidget_setIsHubPortDevice(ch, 1);
		if (res != EPHIDGET_OK)
			return (res);
	}
	if (pm->channel != -1) {
		res = Phidget_setChannel(ch, pm->channel);
		if (res != EPHIDGET_OK)
			return (res);
	}
	if (pm->label != NULL) {
		res = Phidget_setDeviceLabel(ch, pm->label);
		if (res != EPHIDGET_OK)
			return (res);
	}
	if (pm->remote == 1) {
		res = Phidget_setIsRemote(ch, 1);
		if (res != EPHIDGET_OK)
			return (res);
	} else if (pm->remote == 0) {
		res = Phidget_setIsRemote(ch, 0);
		if (res != EPHIDGET_OK)
			return (res);
	}
	if (pm->local == 1) {
		res = Phidget_setIsLocal(ch, 1);
		if (res != EPHIDGET_OK)
			return (res);
	} else if (pm->local == 0) {
		res = Phidget_setIsLocal(ch, 0);
		if (res != EPHIDGET_OK)
			return (res);
	}
	if (pm->servername != 0) {
		res = Phidget_setServerName(ch, pm->servername);
		if (res != EPHIDGET_OK)
			return (res);
	}

	return (EPHIDGET_OK);
}

void CCONV
initProjectMatch(ProjectMatchHandle pm) {

	pm->serialnumber = -1;
	pm->hubport = -1;
	pm->hubportmode = 0;
	pm->channel = -1;
	pm->label = NULL;
	pm->servername = NULL;
	pm->remote = -1;
	pm->local = -1;
}

PhidgetReturnCode CCONV
parseProjectMatch(const char *match, ProjectMatchHandle pm) {
	const char *c, *s;
	char val[32];
	int args[3];
	int off;
	int res;

	off = 0;
	args[0] = -1;
	args[1] = -1;
	args[2] = -1;

	initProjectMatch(pm);

	for (s = c = match; *c; c++) {
		switch (*c) {
		case '/':
			if (s == c) {
				s = c + 1;
			}
			else {
				if (c - s > (int)sizeof(val) - 1)
					return (1);
				memset(val, 0, sizeof(val));
				mos_strncpy(val, s, c - s);
				res = mos_strto32(val, 0, &args[off]);
				s = c + 1;
				if (res != 0)
					return (EPHIDGET_INVALIDARG);
			}
			off++;
			if (off > 2)
				return (EPHIDGET_INVALIDARG);
			break;
		}
	}

	if (s != c) {
		if (c - s > (int)sizeof(val) - 1)
			return (EPHIDGET_INVALIDARG);
		memset(val, 0, sizeof(val));
		mos_strncpy(val, s, c - s);
		res = mos_strto32(val, 0, &args[off]);
		if (res != 0)
			return (EPHIDGET_INVALIDARG);
	}

	pm->serialnumber = args[0];
	pm->hubport = args[1];
	pm->channel = args[2];

	if (pm->hubport >= HUB_PORT_MAX) {
		pm->hubportmode = 1;
		pm->hubport -= HUB_PORT_MAX;
		if (pm->hubport >= HUB_PORT_MAX)
			return (EPHIDGET_INVALIDARG);
	}

	return (EPHIDGET_OK);
}
