#ifndef _PHIDGETADMIN_H_
#define _PHIDGETADMIN_H_

#include <stdio.h>
#include <stdlib.h>

#include "phidget22.h"
#include "phidget22extra.h"

typedef struct {
	char *sku;
	int version;
	int isVint;
	unsigned int vintID;
	char *filename;
	char *path;
} FirmwareUpgradeFileInfo;

typedef MTAILQ_HEAD(fwlist, fwent) fwlist_t;
typedef struct fwent {
	FirmwareUpgradeFileInfo fw;
	MTAILQ_ENTRY(fwent)	link;
} fwent_t;

extern fwlist_t fwlist;

int readPhidgetFirmwareFiles(const char *path);
int upgradePhidgetFirmware(PhidgetHandle device, FirmwareUpgradeFileInfo *fwFileInfo, int versionOverride);
int getPhidgetFWFileInfo(const char *path, const char *filename, FirmwareUpgradeFileInfo *info);
int phidgetFWMatchesDevice(PhidgetHandle device, FirmwareUpgradeFileInfo *fwfileinfo);

#endif /* _PHIDGETADMIN_H_ */