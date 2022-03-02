#ifndef _PHIDGET_MATCH_H_
#define _PHIDGET_MATCH_H_

#if 0

Generalizes matching and attaching to Phidget channels.

To use, register an array of phidgetmatch_t structures:

static void
onAttach(phidgetmatch_t *pd) {
	suploginfo("%P: %s", pd->handle, pd->alias);
}

static phidgetmatch_t phidgets[] = {
	{ 	"Name of Entry", 0 /* flags */, <class of channel>
		{ PHIDGETMATCH_ISREMOTE, <serialno>, <label>, PHIDGET_CHANNEL_ANY, PHIDGET_HUBPORT_ANY },
		250 /* data interval */, onAttach, onDetach, onError, NULL, NULL /* User ctx pointer */
	},
	{ 	"Temperature", 0, PHIDCHCLASS_TEMPERATURESENSOR,
		{ PHIDGETMATCH_ISREMOTE, 491581, NULL, 7, PHIDGET_HUBPORT_ANY },
		0 /* do not set */, onTemperatureAttach, onDetach, onError, NULL, NULL
	},
	{ 	"VINT Hub VR0", 0, PHIDCHCLASS_VOLTAGERATIOINPUT,
		{ PHIDGETMATCH_ISLOCAL | PHIDGETMATCH_ISHUBPORT, 370005, NULL, 0, 0 },
		1 /* as fast as it can go */, onAttach, onDetach, onError, NULL, NULL
	},
	{ 0 }
};

Then:
	res = PhidgetMatch_openChannels(phidgets);

#endif /* DOCS */

#include "phidget22extra_int.h"

#define PHIDGETMATCH_ISHUBPORT		0x01
#define PHIDGETMATCH_ISREMOTE		0x02
#define PHIDGETMATCH_ISLOCAL		0x04

struct _phidgetmatch;

typedef void (CCONV *phidgetmatch_attach_t)(struct _phidgetmatch *);
typedef void (CCONV *phidgetmatch_detach_t)(struct _phidgetmatch *);
typedef void (CCONV *phidgetmatch_error_t)(struct _phidgetmatch *, Phidget_ErrorEventCode, const char *);

typedef struct _phidgetmatchparams {
	int			flags;
	int			sn;
	const char	*label;
	int			channel;
	int			hubport;
} phidgetmatchparams_t;

typedef struct _phidgetmatch {
	const char				*alias;
	int						flags;
	Phidget_ChannelClass	cclass;
	phidgetmatchparams_t	match;
	int						datainterval;
	phidgetmatch_attach_t	onattach;
	phidgetmatch_detach_t	ondetach;
	phidgetmatch_error_t	onerror;
	PhidgetHandle			handle;
	void					*ctx;
} phidgetmatch_t;

PHIDAPI PhidgetReturnCode CCONV PhidgetMatch_openChannels(phidgetmatch_t *);

PHIDAPI PhidgetReturnCode CCONV PhidgetMatch_find(phidgetmatch_t *, const char *, phidgetmatch_t **);
PHIDAPI PhidgetReturnCode CCONV PhidgetMatch_updateChannel(phidgetmatch_t *, const char *, const char *);
PHIDAPI PhidgetReturnCode CCONV PhidgetMatch_setChannel(phidgetmatch_t *, const char *, int);
PHIDAPI PhidgetReturnCode CCONV PhidgetMatch_setHubPort(phidgetmatch_t *, const char *, int);
PHIDAPI PhidgetReturnCode CCONV PhidgetMatch_setDeviceSerialNumber(phidgetmatch_t *, const char *, int);
PHIDAPI PhidgetReturnCode CCONV PhidgetMatch_setDeviceLabel(phidgetmatch_t *, const char *, const char *);

#endif