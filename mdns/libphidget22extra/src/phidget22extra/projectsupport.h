#ifndef _PROJECTSUPPORT_H_
#define _PROJECTSUPPORT_H_

#include "phidget22extra_int.h"

#define SVC_RUN			0x01
#define SVC_DONE		0x02

#define HUB_PORT_MAX	6

PHIDAPI const char * CCONV getPhidgetExtraVersion(void);

typedef struct _PhidgetModuleLoadArgs {
	void	*cfg;
} PhidgetModuleLoadArgs, *PhidgetModuleLoadArgsHandle;

typedef void (CCONV *PhidgetModuleLoad_t)(PhidgetModuleLoadArgsHandle);

/* Module entry point */
PHIDAPI void CCONV phidgetModuleEntry(PhidgetModuleLoadArgsHandle);

typedef PhidgetReturnCode (*daemonstart_t)(void *);
typedef void (*daemonstop_t)(void *);
typedef PhidgetReturnCode (CCONV *phidcreate_t)(PhidgetHandle *);
typedef PhidgetReturnCode (CCONV *phiddelete_t)(PhidgetHandle *);
typedef void (CCONV *phidonattach_t)(PhidgetHandle, void *);
typedef void (CCONV *phidondetach_t)(PhidgetHandle, void *);

PHIDAPI const char * CCONV getComputerName(char *buf, size_t, const char *);
PHIDAPI Phidget_LogLevel CCONV getLogLevel(const char *);

PHIDAPI PhidgetReturnCode CCONV loadModules(const char *, PhidgetModuleLoadArgsHandle);
PHIDAPI PhidgetReturnCode CCONV startDaemon(const char *, daemonstart_t, daemonstop_t, void *, const char *);

typedef struct _ProjectMatch {
	int			serialnumber;	/* -1 default, else set */
	int			hubport;		/* -1 default, else set */
	int			hubportmode;	/* 1 enable */
	int			channel;		/* -1 default, else set */
	const char	*label;			/* set if not NULL */
	const char	*servername;	/* set if not NULL */
	int			remote;			/* 0 no, 1 yes, -1 default */
	int			local;			/* 0 no, 1 yes, -1 default */
} ProjectMatch, *ProjectMatchHandle;

PHIDAPI void CCONV initProjectMatch(ProjectMatchHandle);
PHIDAPI PhidgetReturnCode CCONV parseProjectMatch(const char *, ProjectMatchHandle);
PHIDAPI PhidgetReturnCode CCONV setChannelProjectMatch(PhidgetHandle, ProjectMatchHandle);
PHIDAPI PhidgetReturnCode CCONV setChannelProjectMatchS(PhidgetHandle, const char *);

#endif /* _PROJECTSUPPORT_H_ */