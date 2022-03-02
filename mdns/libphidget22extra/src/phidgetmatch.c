#include "phidget22extra/phidgetloghelpers.h"
#include "phidget22extra/phidgethelpers.h"
#include "phidget22extra/phidgetconfig.h"

#include "phidget22extra/phidgetmatch.h"

static void CCONV
phidgetmatch_onattach(PhidgetHandle ch, void *ctx) {
	PhidgetReturnCode res;
	phidgetmatch_t *pm;

	pm = ctx;

	suploginfo("%"PRIphid, pm->handle);

	if (pm->datainterval != 0) {
		res = Phidget_setDataInterval(ch, pm->datainterval);
		if (res != EPHIDGET_OK)
			suploginfo("%s: failed to set data interval to %d: %s",
			  pm->alias, pm->datainterval, getErrorStr(res));
	}

	if (pm->onattach)
		pm->onattach(pm);
}

static void CCONV
phidgetmatch_ondetach(PhidgetHandle ch, void *ctx) {
	phidgetmatch_t *pm;

	pm = ctx;

	suploginfo("%"PRIphid, pm->handle);

	if (pm->ondetach)
		pm->ondetach(pm);
}

static void CCONV
phidgetmatch_onerror(PhidgetHandle ch, void *ctx, Phidget_ErrorEventCode code, const char *desc) {
	phidgetmatch_t *pm;

	pm = ctx;

	suploginfo("%"PRIphid, pm->handle);

	if (pm->onerror)
		pm->onerror(pm, code, desc);
}

PhidgetReturnCode CCONV
PhidgetMatch_find(phidgetmatch_t *phidgets, const char *alias, phidgetmatch_t **match) {
	phidgetmatch_t *pm;

	for (pm = phidgets; pm->cclass != 0; pm++) {
		if (mos_strcmp(pm->alias, alias) == 0) {
			*match = pm;
			return (EPHIDGET_OK);
		}
	}
	return (EPHIDGET_NOENT);
}

PhidgetReturnCode CCONV
PhidgetMatch_setChannel(phidgetmatch_t *phidgets, const char *alias, int ch) {
	PhidgetReturnCode res;
	phidgetmatch_t *pm;

	res = PhidgetMatch_find(phidgets, alias, &pm);
	if (res != EPHIDGET_OK)
		return (res);

	pm->match.channel = ch;
	return (EPHIDGET_OK);
}

PhidgetReturnCode CCONV
PhidgetMatch_setHubPort(phidgetmatch_t *phidgets, const char *alias, int port) {
	PhidgetReturnCode res;
	phidgetmatch_t *pm;

	res = PhidgetMatch_find(phidgets, alias, &pm);
	if (res != EPHIDGET_OK)
		return (res);

	pm->match.hubport = port;
	return (EPHIDGET_OK);
}

PhidgetReturnCode CCONV
PhidgetMatch_setDeviceSerialNumber(phidgetmatch_t *phidgets, const char *alias, int sn) {
	PhidgetReturnCode res;
	phidgetmatch_t *pm;

	res = PhidgetMatch_find(phidgets, alias, &pm);
	if (res != EPHIDGET_OK)
		return (res);

	pm->match.sn = sn;
	return (EPHIDGET_OK);
}

PhidgetReturnCode CCONV
PhidgetMatch_setDeviceLabel(phidgetmatch_t *phidgets, const char *alias, const char *label) {
	PhidgetReturnCode res;
	phidgetmatch_t *pm;

	res = PhidgetMatch_find(phidgets, alias, &pm);
	if (res != EPHIDGET_OK)
		return (res);

	pm->match.label = mos_strdup(label, NULL);	/* probably a leak */
	return (EPHIDGET_OK);
}

PhidgetReturnCode CCONV
PhidgetMatch_updateChannel(phidgetmatch_t *phidgets, const char *alias, const char *pconf) {
	PhidgetReturnCode res;
	phidgetmatch_t *pm;
	pconf_t *pc;
	int v;

	res = PhidgetMatch_find(phidgets, alias, &pm);
	if (res != EPHIDGET_OK)
		return (res);

	res = pconf_parsepcs(&pc, NULL, 0, pconf);
	if (res != EPHIDGET_OK)
		return (res);

	v = pconf_get32(pc, -1, "phid.sn");
	if (v != -1)
		pm->match.sn = v;

	v = pconf_get32(pc, -1, "phid.port");
	if (v != -1)
		pm->match.hubport = v;

	v = pconf_get32(pc, -1, "phid.ch");
	if (v != -1)
		pm->match.channel = v;

	pconf_release(&pc);

	return (EPHIDGET_OK);
}

PhidgetReturnCode CCONV
PhidgetMatch_openChannels(phidgetmatch_t *phidgets) {
	PhidgetReturnCode res;
	phidgetmatch_t *pm;

	for (pm = phidgets; pm->cclass != 0; pm++) {
		if (pm->alias == NULL)
			continue;

		res = createTypedPhidgetChannelHandle(&pm->handle, pm->cclass);
		if (res != EPHIDGET_OK) {
			suplogerr("%s: Failed to create channel handle for class: 0x%x", pm->alias, pm->cclass);
			continue;
		}
		suploginfo("%s: created handle", pm->alias);

		res = Phidget_setOnAttachHandler(pm->handle, phidgetmatch_onattach, pm);
		if (res != EPHIDGET_OK) {
			suplogerr("%s: Failed to set attach handler: %s", pm->alias, getErrorStr(res));
			goto bad;
		}
		suploginfo("%s: attach handler registered", pm->alias);

		res = Phidget_setOnDetachHandler(pm->handle, phidgetmatch_ondetach, pm);
		if (res != EPHIDGET_OK) {
			suplogerr("%s: Failed to set detach handler: %s", pm->alias, getErrorStr(res));
			goto bad;
		}
		suploginfo("%s: detach handler registered", pm->alias);

		res = Phidget_setOnErrorHandler(pm->handle, phidgetmatch_onerror, pm);
		if (res != EPHIDGET_OK) {
			suplogerr("%s: Failed to set error handler: %s", pm->alias, getErrorStr(res));
			goto bad;
		}
		suploginfo("%s: error handler registered", pm->alias);

		if (pm->match.sn != -1) {
			res = Phidget_setDeviceSerialNumber(pm->handle, pm->match.sn);
			if (res != EPHIDGET_OK) {
				suplogerr("%s: Failed to set device serial number (%d): %s", pm->alias,
				  pm->match.sn, getErrorStr(res));
				goto bad;
			}
			suploginfo("%s: device serial number: %d", pm->alias, pm->match.sn);
		}

		if (pm->match.label != NULL) {
			res = Phidget_setDeviceLabel(pm->handle, pm->match.label);
			if (res != EPHIDGET_OK) {
				suplogerr("%s: Failed to set device label (%s): %s", pm->alias, pm->match.label,
				  getErrorStr(res));
				goto bad;
			}
			suploginfo("%s: device label: %s", pm->alias, pm->match.label);
		}

		if (pm->match.flags & PHIDGETMATCH_ISLOCAL) {
			res = Phidget_setIsLocal(pm->handle, 1);
			if (res != EPHIDGET_OK) {
				suplogerr("%s: Failed to set is local: %s", pm->alias, getErrorStr(res));
				goto bad;
			}
			suploginfo("%s: is local", pm->alias);
		}

		if (pm->match.flags & PHIDGETMATCH_ISREMOTE) {
			res = Phidget_setIsRemote(pm->handle, 1);
			if (res != EPHIDGET_OK) {
				suplogerr("%s: Failed to set is remote: %s", pm->alias, getErrorStr(res));
				goto bad;
			}
			suploginfo("%s: is remote", pm->alias);
		}

		if (pm->match.channel != PHIDGET_CHANNEL_ANY) {
			res = Phidget_setChannel(pm->handle, pm->match.channel);
			if (res != EPHIDGET_OK) {
				suplogerr("%s: Failed to set channel to %d: %s", pm->alias, pm->match.channel,
				  getErrorStr(res));
				goto bad;
			}
			suploginfo("%s: channel:%d", pm->alias, pm->match.channel);
		}

		if (pm->match.hubport != PHIDGET_HUBPORT_ANY) {
			res = Phidget_setHubPort(pm->handle, pm->match.hubport);
			if (res != EPHIDGET_OK) {
				suplogerr("%s: Failed to set hub port to %d: %s", pm->alias, pm->match.hubport,
				  getErrorStr(res));
				goto bad;
			}
			suploginfo("%s: hub port:%d", pm->alias, pm->match.hubport);
		}

		if (pm->match.flags & PHIDGETMATCH_ISHUBPORT) {
			res = Phidget_setIsHubPortDevice(pm->handle, 1);
			if (res != EPHIDGET_OK) {
				suplogerr("%s: Failed to set is hub port device: %s", pm->alias, getErrorStr(res));
				goto bad;
			}
			suploginfo("%s: is hub port device", pm->alias);
		}

		res = Phidget_open(pm->handle);
		if (res != EPHIDGET_OK) {
			suploginfo("%s: Failed to open channel: %s", pm->alias, getErrorStr(res));
			goto bad;
		}
		suploginfo("%s: opened", pm->alias);
		continue;

bad:
		Phidget_delete(&pm->handle);
	}

	return (EPHIDGET_OK);
}
