#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "phidgetadmin.h"
#include "phidget22.h"
#include "fwupgrade.h"
#include "phidget22extra.h"

#include "mos/mos_readdir.h"

fwlist_t fwlist;

static int fwadd(const char *path, const char *file) {
	fwent_t *fwe;

	fwe = mos_malloc(sizeof(*fwe));
	if (getPhidgetFWFileInfo(path, file, &fwe->fw)) {
		mos_free(fwe, (sizeof(*fwe)));
		return (1);
	}

	MTAILQ_INSERT_HEAD(&fwlist, fwe, link);

	return (0);
}

/* Combine two linked lists */
static fwent_t *merge(fwent_t *node1, fwent_t *node2) {
	if (node1 == NULL) {
		return node2;
	}
	if (node2 == NULL) {
		return node1;
	}

	fwent_t *res, *ptr;

	if (node1->fw.version > node2->fw.version) {
		res = node1;
		node1 = MTAILQ_NEXT(node1, link);
	} else {
		res = node2;
		node2 = MTAILQ_NEXT(node2, link);
	}

	ptr = res;

	while (node1 != NULL && node2 != NULL) {
		if (node1->fw.version > node2->fw.version) {
			MTAILQ_NEXT(ptr, link) = node1;
			node1 = MTAILQ_NEXT(node1, link);
		} else {
			MTAILQ_NEXT(ptr, link) = node2;
			node2 = MTAILQ_NEXT(node2, link);
		}
		ptr = MTAILQ_NEXT(ptr, link);
	}

	if (node1 != NULL) {
		MTAILQ_NEXT(ptr, link) = node1;
	} else if (node2 != NULL) {
		MTAILQ_NEXT(ptr, link) = node2;
	}

	return res;
}

/* Sort and sort the linked list nodes */
static fwent_t *mergeSort(fwent_t *node) {
	if (node == NULL || MTAILQ_NEXT(node, link) == NULL) {
		return node;
	}

	fwent_t *fast = node;
	fwent_t *slow = node;
	while (MTAILQ_NEXT(fast, link) != NULL) {
		if (MTAILQ_NEXT(MTAILQ_NEXT(fast, link), link) == NULL) {
			break;
		}
		fast = MTAILQ_NEXT(MTAILQ_NEXT(fast, link), link);
		slow = MTAILQ_NEXT(slow, link);
	}

	fast = slow;
	slow = MTAILQ_NEXT(slow, link);
	MTAILQ_NEXT(fast, link) = NULL;
	fast = mergeSort(node);
	slow = mergeSort(slow);
	return merge(fast, slow);
}

static void sortFirmware(fwlist_t *list) {
	MTAILQ_FIRST(list) = mergeSort(MTAILQ_FIRST(list));
}

int readPhidgetFirmwareFiles(const char *path) {
	mos_dirinfo_t *di;
	int err;

	err = mos_opendir(NULL, path, &di);
	if (err != 0) {
		return 1;
	}

	for (;;) {
		err = mos_readdir(NULL, di);
		if (err != 0) {
			fprintf(stderr, "failed to read directory '%s'", path);
			goto bad;
		}

		if (di->errcode == MOSN_NOENT)
			break;

		if (di->flags & MOS_DIRINFO_ISDIR)
			continue;

		if (!mos_endswith(di->filename, "rc4") && !mos_endswith(di->filename, "obf"))
			continue;

		if(fwadd(di->path, di->filename))
			goto bad;
	}

	mos_closedir(&di);

	sortFirmware(&fwlist);

	return (0);

bad:
	mos_closedir(&di);
	return (1);
}

int getPhidgetFWFileInfo(const char *path, const char *filename, FirmwareUpgradeFileInfo *info) {
	const char *ext;
	int underscores;
	const char *c;

	if (filename == NULL)
		return 1;

	ext = filename + strlen(filename) - 3;

	// Count underscores
	underscores = 0;
	c = filename;
	while (c[0]) {
		if (c[0] == '_')
			underscores++;
		c++;
	}

	if (!strcmp(ext, "rc4")) {
		if (underscores != 1)
			goto error;
		info->isVint = 0;
		c = strchr(filename, 'v');
		if (c == NULL)
			goto error;
		info->sku = malloc(c - filename + 1);
		strncpy(info->sku, filename, (c - filename));
		info->sku[(c - filename)] = '\0';
		info->version = strtol(c + 1, NULL, 10);
	} else if (!strcmp(ext, "obf")) {
		if (underscores != 3 && underscores != 2)
			goto error;
		info->isVint = 1;
		c = strchr(filename, '_');
		if (c == NULL)
			goto error;
		info->sku = malloc(c - filename + 1);
		strncpy(info->sku, filename, (c - filename));
		info->sku[(c - filename)] = '\0';

		// sku contains rev - skip it
		if (underscores == 3)
			c = strchr(c + 1, '_');

		info->vintID = strtoul(c + 1, NULL, 16);
		c = strchr(c + 1, 'v');
		info->version = strtol(c + 1, NULL, 10);
	} else {
		fprintf(stderr, "Invalid firmware extension: %s\n", ext);
		return 1;
	}

	info->filename = mos_strdup(filename, NULL);
	info->path = mos_strdup(path, NULL);

	return 0;

error:
	fprintf(stderr, "Invalid firmware filename: %s\n", filename);
	return 1;
}

static void CCONV progress(PhidgetFirmwareUpgradeHandle fw, void *ctx, double p) {

	printf("\rProgress: %5.1lf%%", p * 100.0);
	fflush(stdout);
}

int phidgetFWMatchesDevice(PhidgetHandle device, FirmwareUpgradeFileInfo *fwfileinfo) {
	Phidget_ChannelClass channelClass;
	Phidget_DeviceClass deviceClass;
	PhidgetReturnCode ret;
	const char *deviceSKU;
	const char *deviceFirmwareUpgradeString;
	uint32_t deviceVINTID;
	int isCh;

	ret = Phidget_getDeviceClass(device, &deviceClass);
	if (ret != EPHIDGET_OK) {
		fprintf(stderr, "Error reading device class: %s\n", Phidget_strerror(ret));
		return 1;
	}

	ret = Phidget_getIsChannel(device, &isCh);
	if (ret != EPHIDGET_OK) {
		fprintf(stderr, "Error reading is channel: %s\n", Phidget_strerror(ret));
		return 1;
	}

	if (isCh) {
		ret = Phidget_getChannelClass(device, &channelClass);
		if (ret != EPHIDGET_OK) {
			fprintf(stderr, "Error reading channel class: %s\n", Phidget_strerror(ret));
			return 1;
		}
	}

	ret = Phidget_getDeviceSKU(device, &deviceSKU);
	if (ret != EPHIDGET_OK) {
		fprintf(stderr, "Error reading device SKU: %s\n", Phidget_strerror(ret));
		return 1;
	}

	ret = Phidget_getDeviceFirmwareUpgradeString(device, &deviceFirmwareUpgradeString);
	if (ret != EPHIDGET_OK) {
		fprintf(stderr, "Error reading device firmware upgrade string: %s\n", Phidget_strerror(ret));
		return 1;
	}

	if (deviceClass == PHIDCLASS_VINT) {

		if (!fwfileinfo->isVint) {
			return 0;
		}

		if (isCh && channelClass == PHIDCHCLASS_FIRMWAREUPGRADE) {
			ret = PhidgetFirmwareUpgrade_getActualDeviceVINTID((PhidgetFirmwareUpgradeHandle)device, &deviceVINTID);
			if (ret != EPHIDGET_OK) {
				fprintf(stderr, "Error reading device VINT ID: %s\n", Phidget_strerror(ret));
				return 0;
			}
			if (fwfileinfo->vintID != deviceVINTID) {
				return 0;
			}
		} else {

			ret = Phidget_getDeviceVINTID(device, &deviceVINTID);
			if (ret != EPHIDGET_OK) {
				fprintf(stderr, "Error reading device VINT ID: %s\n", Phidget_strerror(ret));
				return 0;
			}
			if (fwfileinfo->vintID != deviceVINTID) {
				return 0;
			}
			if (strcmp(fwfileinfo->sku, deviceFirmwareUpgradeString)) {
				return 0;
			}
		}

	} else {

		if (fwfileinfo->isVint) {
			return 0;
		}

		if (strcmp(fwfileinfo->sku, deviceFirmwareUpgradeString)) {
			return 0;
		}

	}

	return 1;
}

// Pass is a opened device handle
static int verify_filename(PhidgetHandle device, FirmwareUpgradeFileInfo *fwfileinfo) {
	Phidget_ChannelClass channelClass;
	Phidget_DeviceClass deviceClass;
	PhidgetReturnCode ret;
	const char *deviceSKU;
	const char *deviceFirmwareUpgradeString;
	uint32_t deviceVINTID;
	int isCh;

	ret = Phidget_getDeviceClass(device, &deviceClass);
	if (ret != EPHIDGET_OK) {
		fprintf(stderr, "Error reading device class: %s\n", Phidget_strerror(ret));
		return 1;
	}

	ret = Phidget_getIsChannel(device, &isCh);
	if (ret != EPHIDGET_OK) {
		fprintf(stderr, "Error reading is channel: %s\n", Phidget_strerror(ret));
		return 1;
	}

	if (isCh) {
		ret = Phidget_getChannelClass(device, &channelClass);
		if (ret != EPHIDGET_OK) {
			fprintf(stderr, "Error reading channel class: %s\n", Phidget_strerror(ret));
			return 1;
		}
	}

	ret = Phidget_getDeviceSKU(device, &deviceSKU);
	if (ret != EPHIDGET_OK) {
		fprintf(stderr, "Error reading device SKU: %s\n", Phidget_strerror(ret));
		return 1;
	}

	ret = Phidget_getDeviceFirmwareUpgradeString(device, &deviceFirmwareUpgradeString);
	if (ret != EPHIDGET_OK) {
		fprintf(stderr, "Error reading device firmware upgrade string: %s\n", Phidget_strerror(ret));
		return 1;
	}

	if (deviceClass == PHIDCLASS_VINT) {

		if (!fwfileinfo->isVint) {
			fprintf(stderr, "Firmware specified is not for a VINT device");
			return 1;
		}

		if (isCh && channelClass == PHIDCHCLASS_FIRMWAREUPGRADE) {
			ret = PhidgetFirmwareUpgrade_getActualDeviceVINTID((PhidgetFirmwareUpgradeHandle )device, &deviceVINTID);
			if (ret != EPHIDGET_OK) {
				fprintf(stderr, "Error reading device VINT ID: %s\n", Phidget_strerror(ret));
				return 1;
			}
			if (fwfileinfo->vintID != deviceVINTID) {
				fprintf(stderr, "Device VINT ID does not match firmware file: 0x%03x vs 0x%03x\n", deviceVINTID, fwfileinfo->vintID);
				return 1;
			}
		} else {

			ret = Phidget_getDeviceVINTID(device, &deviceVINTID);
			if (ret != EPHIDGET_OK) {
				fprintf(stderr, "Error reading device VINT ID: %s\n", Phidget_strerror(ret));
				return 1;
			}
			if (fwfileinfo->vintID != deviceVINTID) {
				fprintf(stderr, "Device VINT ID does not match firmware file: 0x%03x vs 0x%03x\n", deviceVINTID, fwfileinfo->vintID);
				return 1;
			}
			if (strcmp(fwfileinfo->sku, deviceFirmwareUpgradeString)) {
				fprintf(stderr, "Device SKU does not match firmware file: %s vs %s\n", deviceFirmwareUpgradeString, fwfileinfo->sku);
				return 1;
			}
		}

	} else {

		if (fwfileinfo->isVint) {
			fprintf(stderr, "Firmware specified is for a VINT device");
			return 1;
		}

		if (strcmp(fwfileinfo->sku, deviceFirmwareUpgradeString)) {
			fprintf(stderr, "Device Upgrade SKU does not match firmware file: %s vs %s\n", deviceFirmwareUpgradeString, fwfileinfo->sku);
			return 1;
		}

	}

	return 0;
}

// This expects an intialized device be passed in (could come from a PhidgetManager attach)
int upgradePhidgetFirmware(PhidgetHandle device, FirmwareUpgradeFileInfo *fwFileInfo, int versionOverride) {
	unsigned long fileLen;
	unsigned char *buffer;
	char fwfile[MOS_PATH_MAX];
	FILE *file = NULL;
	const char *srv = NULL;
	size_t readres;
	int remote;

	PhidgetReturnCode ret;
	int retryCnt = 0;

	PhidgetFirmwareUpgradeHandle fw = NULL;
	PhidgetHubHandle hub = NULL;

	Phidget_ChannelClass channelClass;
	Phidget_DeviceClass deviceClass = PHIDCLASS_NOTHING;
	Phidget_DeviceID id;
	int hubport;
	int serial;
	int currentVersion = -1;

	ret = Phidget_getDeviceSerialNumber(device, &serial);
	if (ret != EPHIDGET_OK) {
		fprintf(stderr, "Error reading device serial number: %s\n", Phidget_strerror(ret));
		goto error;
	}

	ret = Phidget_getDeviceClass(device, &deviceClass);
	if (ret != EPHIDGET_OK) {
		fprintf(stderr, "Error reading device class: %s\n", Phidget_strerror(ret));
		goto error;
	}

	ret = Phidget_getChannelClass(device, &channelClass);
	if (ret != EPHIDGET_OK) {
		fprintf(stderr, "Error reading channel class: %s\n", Phidget_strerror(ret));
		goto error;
	}

	ret = Phidget_getDeviceID(device, &id);
	if (ret != EPHIDGET_OK) {
		fprintf(stderr, "Error reading device ID: %s\n", Phidget_strerror(ret));
		goto error;
	}

	ret = Phidget_getIsRemote(device, &remote);
	if (ret != EPHIDGET_OK) {
		fprintf(stderr, "Error reading remote: %s\n", Phidget_strerror(ret));
		goto error;
	}

	if (remote) {
		Phidget_getServerName(device, &srv);
		// Make a copy because it can become invalid once the handle is opened/closed
		srv = mos_strdup(srv, NULL);
	}

	if (deviceClass == PHIDCLASS_VINT) {
		ret = Phidget_getHubPort(device, &hubport);
		if (ret != EPHIDGET_OK) {
			fprintf(stderr, "Error reading device hub port: %s\n", Phidget_strerror(ret));
			goto error;
		}

		PhidgetHub_create(&hub);
		Phidget_setDeviceSerialNumber((PhidgetHandle)hub, serial);
		if (remote) {
			Phidget_setIsRemote((PhidgetHandle)hub, 1);
			Phidget_setServerName((PhidgetHandle)hub, srv);
		}
	}

	// See if we passed in a FirmwareUpgrade handle
	if (channelClass == PHIDCHCLASS_FIRMWAREUPGRADE)
		goto fw;

	ret = Phidget_getDeviceVersion(device, &currentVersion);
	if (ret != EPHIDGET_OK) {
		fprintf(stderr, "Error reading device version: %s\n", Phidget_strerror(ret));
		goto error;
	}

	if (!versionOverride) {
		if (fwFileInfo->version <= currentVersion) {
			fprintf(stderr, "Firmware file is not newer than existing firmware v%d\n", currentVersion);
			goto abort;
		}

		if (remote) {
			int minor, major;
			ret = Phidget_getServerVersion(device, &major, &minor);
			if (ret != EPHIDGET_OK) {
				fprintf(stderr, "Error reading server version: %s\n", Phidget_strerror(ret));
				goto error;
			}
			if (((major < 2) || (major == 2 && minor < 3))) {
				// If the item is remote and it's server version is eariler than 2.3
				int upgradeServer = 0;

				// HUB0000 v300+
				if (id == PHIDID_HUB0000 && fwFileInfo->version >= 300)
					upgradeServer = 1;

				// HUB0004 v200+ (SBC3003)
				if (id == PHIDID_HUB0004 && fwFileInfo->version >= 200)
					upgradeServer = 1;

				// HUB5000 v200+
				if (id == PHIDID_HUB5000 && fwFileInfo->version >= 200)
					upgradeServer = 1;

				if (upgradeServer) {
					fprintf(stderr, "The Phidget Network Server and libraries must be upgraded (on the remote machine) to support the latest firmware release for this Phidget.\n");
					goto abort;
				}
			}
		}
	}

retry:

	if (deviceClass == PHIDCLASS_VINT) {
		ret = Phidget_openWaitForAttachment((PhidgetHandle)hub, 2500);
		if (ret != EPHIDGET_OK) {
			fprintf(stderr, "Error on hub wait for attach: %s\n", Phidget_strerror(ret));
			goto error;
		}

		printf("Power cycling hub port\n");
		ret = PhidgetHub_setPortPower(hub, hubport, 0);
		if (ret != EPHIDGET_OK) {
			fprintf(stderr, "Error power cycling hub port: %s\n", Phidget_strerror(ret));
			goto error;
		}
		mos_usleep(100000);
		ret = PhidgetHub_setPortPower(hub, hubport, 1);
		if (ret != EPHIDGET_OK) {
			fprintf(stderr, "Error power cycling hub port: %s\n", Phidget_strerror(ret));
			goto error;
		}

		Phidget_close((PhidgetHandle)hub);
	}

	// See if we passed in a FirmwareUpgrade handle
	if (channelClass == PHIDCHCLASS_FIRMWAREUPGRADE)
		goto fw;

	printf("Opening device...\n");

	ret = Phidget_openWaitForAttachment(device, 2500);
	if (ret != EPHIDGET_OK) {
		// Maybe it's already in firmware upgrade mode
		if (ret == EPHIDGET_TIMEOUT)
			goto fw;
		fprintf(stderr, "Error on open wait for attach: %s\n", Phidget_strerror(ret));
		goto error;
	}

	ret = Phidget_getChannelClass(device, &channelClass);
	if (ret != EPHIDGET_OK) {
		fprintf(stderr, "Error reading channel class: %s\n", Phidget_strerror(ret));
		goto error;
	}

	// See if we passed in a FirmwareUpgrade handle
	if (channelClass == PHIDCHCLASS_FIRMWAREUPGRADE)
		goto fw;

	// Again after opening
	ret = Phidget_getDeviceVersion(device, &currentVersion);
	if (ret != EPHIDGET_OK) {
		fprintf(stderr, "Error reading device version: %s\n", Phidget_strerror(ret));
		goto error;
	}

	if (!versionOverride) {
		if (fwFileInfo->version <= currentVersion) {
			fprintf(stderr, "Firmware file is not newer than existing firmware v%d\n", currentVersion);
			goto abort;
		}

		if (remote) {
			int minor, major;
			ret = Phidget_getServerVersion(device, &major, &minor);
			if (ret != EPHIDGET_OK) {
				fprintf(stderr, "Error reading server version: %s\n", Phidget_strerror(ret));
				goto error;
			}
			if (((major < 2) || (major == 2 && minor < 3))) {
				// If the item is remote and it's server version is eariler than 2.3
				int upgradeServer = 0;

				// HUB0000 v300+
				if (id == PHIDID_HUB0000 && fwFileInfo->version >= 300)
					upgradeServer = 1;

				// HUB0004 v200+ (SBC3003)
				if (id == PHIDID_HUB0004 && fwFileInfo->version >= 200)
					upgradeServer = 1;

				// HUB5000 v200+
				if (id == PHIDID_HUB5000 && fwFileInfo->version >= 200)
					upgradeServer = 1;

				if (upgradeServer) {
					fprintf(stderr, "The Phidget Network Server and libraries must be upgraded (on the remote machine) to support the latest firmware release for this Phidget.\n");
					goto abort;
				}
			}
		}
	}

	// Verify that we have the correct firmware file
	if (verify_filename(device, fwFileInfo))
		goto error;

	printf("Rebooting into firmware upgrade mode...\n");

	ret = Phidget_rebootFirmwareUpgrade(device, 1500);
	if (ret != EPHIDGET_OK) {
		fprintf(stderr, "Error rebooting into firmware upgrade mode: %s\n", Phidget_strerror(ret));
		goto error;
	}

fw:
	Phidget_close(device);
	PhidgetFirmwareUpgrade_create(&fw);

	// Make sure we open the correct device
	Phidget_setDeviceSerialNumber((PhidgetHandle)fw, serial);
	if (deviceClass == PHIDCLASS_VINT) {
		Phidget_setHubPort((PhidgetHandle)fw, hubport);
	}
	if (remote) {
		Phidget_setIsRemote((PhidgetHandle)fw, 1);
		Phidget_setServerName((PhidgetHandle)fw, srv);
	}

	printf("Opening in firmware upgrade mode...\n");

	ret = Phidget_openWaitForAttachment((PhidgetHandle)fw, 2500);
	if (ret != EPHIDGET_OK) {
		fprintf(stderr, "Error on fw wait for attach: %s\n", Phidget_strerror(ret));
		goto error;
	}

	if (deviceClass == PHIDCLASS_VINT && currentVersion == -1) {
		ret = PhidgetFirmwareUpgrade_getActualDeviceVersion((PhidgetFirmwareUpgradeHandle)fw, &currentVersion);
		if (ret != EPHIDGET_OK) {
			fprintf(stderr, "Error reading device version: %s\n", Phidget_strerror(ret));
			goto error;
		}
	}

	if (fwFileInfo->version <= currentVersion && !versionOverride) {
		fprintf(stderr, "Firmware file is not newer than existing firmware.\n");
		goto abort;
	}

	// Verify that we have the correct firmware file
	if (verify_filename((PhidgetHandle)fw, fwFileInfo))
		goto error;

	printf("Loading firmware file...\n");

	// Load firmware upgrade file
	snprintf(fwfile, sizeof(fwfile), "%s/%s", fwFileInfo->path, fwFileInfo->filename);
	file = fopen(fwfile, "rb");
	if (!file) {
		fprintf(stderr, "Unable to open file: %s\n", fwfile);
		goto error;
	}

	// Get file length
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);

	// Sanity check <= 64K
	if (fileLen > (64 * 1024)) {
		fclose(file);
		fprintf(stderr, "Firmware file is too big\n");
		goto error;
	}

	// Read file contents into buffer
	buffer = malloc(fileLen);
	memset(buffer, 0, fileLen);
	readres = fread(buffer, fileLen, 1, file);
	fclose(file);

	if (readres != 1) {
		fprintf(stderr, "Failed to read full firmware file\n");
		goto error;
	}

	printf("Sending firmware to device...\n");

	PhidgetFirmwareUpgrade_setOnProgressChangeHandler(fw, progress, NULL);

	// Send it
	ret = PhidgetFirmwareUpgrade_sendFirmware(fw, buffer, fileLen);
	free(buffer);
	if (ret != EPHIDGET_OK) {
		fprintf(stderr, "\nError sending firmware to device: %s\n", Phidget_strerror(ret));
		goto error;
	}

	PhidgetFirmwareUpgrade_setOnProgressChangeHandler(fw, NULL, NULL);
	// Newline after progress event prints
	printf("\n");

	if (deviceClass == PHIDCLASS_VINT) {
		// We do a reboot AND power cycle as this is the most reliable for VINT
		Phidget_reboot((PhidgetHandle)fw);
		mos_usleep(100000);

		ret = Phidget_openWaitForAttachment((PhidgetHandle)hub, 2500);
		if (ret != EPHIDGET_OK) {
			fprintf(stderr, "Error on hub wait for attach: %s\n", Phidget_strerror(ret));
			goto error;
		}

		printf("Power cycling hub port\n");
		ret = PhidgetHub_setPortPower(hub, hubport, 0);
		if (ret != EPHIDGET_OK) {
			fprintf(stderr, "Error power cycling hub port: %s\n", Phidget_strerror(ret));
			goto error;
		}
		mos_usleep(100000);
		ret = PhidgetHub_setPortPower(hub, hubport, 1);
		if (ret != EPHIDGET_OK) {
			fprintf(stderr, "Error power cycling hub port: %s\n", Phidget_strerror(ret));
			goto error;
		}
	} else {
		printf("Rebooting device...\n");

		// Reboot
		ret = Phidget_reboot((PhidgetHandle)fw);
		if (ret != EPHIDGET_OK) {
			fprintf(stderr, "Error rebooting device: %s\n", Phidget_strerror(ret));
			goto error;
		}
	}

	// Unless a firmware upgrade device was passed in, try opening the new device
	if (channelClass != PHIDCHCLASS_FIRMWAREUPGRADE) {
		ret = Phidget_openWaitForAttachment(device, 2500);
		if (ret != EPHIDGET_OK) {
			fprintf(stderr, "Error on wait for attach after upgrade: %s\n", Phidget_strerror(ret));
			goto error;
		}

		// Again after opening
		ret = Phidget_getDeviceVersion(device, &currentVersion);
		if (ret != EPHIDGET_OK) {
			fprintf(stderr, "Error reading device version: %s\n", Phidget_strerror(ret));
			goto error;
		}

		if (currentVersion != fwFileInfo->version) {
			fprintf(stderr, "Error - device version after upgrade is not expected version: %d vs %d\n", currentVersion, fwFileInfo->version);
			goto error;
		}
	}

	Phidget_close(device);
	Phidget_delete((PhidgetHandle*)&hub);
	Phidget_delete((PhidgetHandle*)&fw);

	printf("Success!\n\n");

	return 0;

error:
	fprintf(stderr, "Fail!\n\n");

	Phidget_close(device);
	Phidget_close((PhidgetHandle)hub);
	Phidget_delete((PhidgetHandle*)&fw);

	if (retryCnt++ < 3) {
		printf("Trying again...\n");
		goto retry;
	}

	Phidget_delete((PhidgetHandle*)&hub);

	return 1;

abort:
	Phidget_close(device);
	Phidget_delete((PhidgetHandle*)&hub);
	Phidget_delete((PhidgetHandle*)&fw);

	printf("Upgrade aborted!\n\n");

	return 1;
}
