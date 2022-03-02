#include "phidgetadmin.h"
#include "fwupgrade.h"
#include "mos/mos_getpasswd.h"
#include "phidget22extra.h"

#ifdef UNIX
#include <signal.h>
#endif

#define HUB_PORT_MAX 6

int stop = 0;

static char dictkey[32];
static char dictval[128];

static int waittime = 3000000;	/* 3 second default */

mos_mutex_t printLock; /* protects stdout */

static int Aflag;
static int Lflag;
static int Rflag;
static int Uflag;
static int aflag;
static int cflag;
static int dflag;
static int kflag;
static int lflag;
static int oflag;
static int qflag;
static int sflag;
static int uflag;
static int vflag;

static const char *srvname;
static const char *userfwpathname;
static int serialno = -1;
static int hubport = -1;
static int hubportmode;
static int channel = -1;

static int fwversionoverride = -1;
static int fwallowmajorupgrade = 0;

static const char *DEFAULT_FIRMWARE_PATH = "./firmware";
static int newestFirmware(PhidgetHandle ch);

typedef MTAILQ_HEAD(srvlist, srvent) srvlist_t;
typedef struct srvent {
	char					name[32];
	char					stype[32];
	char					addr[32];
	char					host[32];
	int						port;
	int						authreq;
	MTAILQ_ENTRY(srvent)	link;
} srvent_t;
static srvlist_t srvlist;

typedef MTAILQ_HEAD(devlist, devent) devlist_t;
typedef struct devent {
	int						serialno;
	MTAILQ_ENTRY(devent)	link;
} devent_t;
static devlist_t devlist;

typedef MTAILQ_HEAD(plist, pent) plist_t;
typedef struct pent {
	PhidgetHandle		phid;
	int					isch;
	plist_t				children;
	MTAILQ_ENTRY(pent)	link;
} pent_t;
static plist_t	plist;

#ifdef _WINDOWS
static BOOL
ctrlhandler(DWORD type) {

	switch (type) {
	case CTRL_C_EVENT:
		stop = 1;
		return (TRUE);
	case CTRL_SHUTDOWN_EVENT:
		stop = 1;
		return (TRUE);
	default:
		return (FALSE);
	}
}

static int
register_signalhandlers() {
	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrlhandler, TRUE) != 0)
		return (0);
	return (1);
}

#endif /* _WINDOWS */

#ifdef UNIX
static void
sighandler(int sig, siginfo_t *siginfo, void *context) {

	stop = 1;
}

static int
register_signalhandlers() {
	struct sigaction act;

	memset(&act, '\0', sizeof(act));
	act.sa_sigaction = &sighandler;
	act.sa_flags = SA_SIGINFO;

	sigaction(SIGINT, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);
	signal(SIGPIPE, SIG_IGN);

	return (0);
}
#endif /* UNIX */

static void
usage(const char *name, int err) {

	FILE * out = err ? stderr : stdout;
	fprintf(out, "Usage: %s [options]...\n", name);
	fprintf(out, "Options:\n");
	fprintf(out, "  -A            set password for server (requires -H)\n");
	fprintf(out, "  -F path       set path of firmware upgrade files\n");
	fprintf(out, "  -H srvname    filter by server name\n");
	fprintf(out, "  -L            include local devices\n");
	fprintf(out, "  -M sn[/hp/ch] filter by serial number / hub port / channel\n");
	fprintf(out, "  -R            include remote devices\n");
	fprintf(out, "  -U            perform a firmware upgrade (requires -M)\n");
	fprintf(out, "  -V version    specify firmware version for upgrade (default newest)\n");
	fprintf(out, "  -m            allow major firmware upgrades (default allow minor)\n");
	fprintf(out, "  -a            also list devices that don't need upgrades (requires -u)\n");
	fprintf(out, "  -c            list open channels on server (requires -H)\n");
	fprintf(out, "  -d            list Phidgets\n");
	fprintf(out, "  -k key[=val]  print [set] key on control dictionary (requires -H)\n");
	fprintf(out, "  -l            enable phidget22 logging\n");
	fprintf(out, "  -o            list open connections on server (requires -H)\n");
	fprintf(out, "  -q            quiet\n");
	fprintf(out, "  -s            list servers\n");
	fprintf(out, "  -u            firmware upgrade mode\n");
	fprintf(out, "  -v            verbose output\n");
	fprintf(out, "  -w seconds    time to wait for devices, servers (default is 3 sec)\n");
	fprintf(out, "\n");
	fprintf(out, "Examples:\n");
	fprintf(out, "  List all local and remote devices and channels:\n");
	fprintf(out, "    %s -d\n", name);
	fprintf(out, "\n");
	fprintf(out, "  List Phidget servers and Network Phidgets:\n");
	fprintf(out, "    %s -s\n", name);
	fprintf(out, "\n");
	fprintf(out, "  List devices which can be firmware upgraded:\n");
	fprintf(out, "    %s -du\n", name);
	fprintf(out, "\n");
	fprintf(out, "  Upgrade firmware of a local VINT Phidget (sn: 123456, port: 2):\n");
	fprintf(out, "    %s -M 123456/2 -U\n", name);
	fprintf(out, "\n");
	fprintf(out, "  Upgrade firmware of a local USB Phidget (sn: 123456):\n");
	fprintf(out, "    %s -M 123456 -U\n", name);
	fprintf(out, "\n");
	fprintf(out, "  Upgrade firmware of a local USB Phidget (sn: 123456),\n");
	fprintf(out, "  allowing for a major version upgrade which may introduce breaking changes:\n");
	fprintf(out, "    %s -M 123456 -Um\n", name);
	fprintf(out, "\n");
	fprintf(out, "  Upgrade firmware of a remote USB Phidget (srvname: phidgetsbc, sn: 123456):\n");
	fprintf(out, "    %s -H \"phidgetsbc\" -M 123456 -U\n", name);
	fprintf(out, "\n");

	exit(err);
}

static int
srvadd(PhidgetServer *srv) {
	srvent_t *se;

	se = mos_malloc(sizeof(*se));
	mos_strlcpy(se->name, srv->name, sizeof(se->name));
	mos_strlcpy(se->host, srv->host, sizeof(se->host));
	mos_strlcpy(se->addr, srv->addr, sizeof(se->addr));
	mos_strlcpy(se->stype, srv->stype, sizeof(se->stype));
	se->port = srv->port;
	se->authreq = srv->flags & PHIDGETSERVER_AUTHREQUIRED;

	MTAILQ_INSERT_HEAD(&srvlist, se, link);

	return (0);
}

/*
static srvent_t *
getsrvbyname(const char *name) {
	srvent_t *se;

	MTAILQ_FOREACH(se, &srvlist, link) {
		if (mos_strcmp(se->name, name) == 0)
			return (se);
	}

	return (NULL);
}
*/

static pent_t *
plistFind(plist_t *list, PhidgetHandle phid) {
	pent_t *sent;
	pent_t *ent;

	if (list == NULL)
		list = &plist;

	MTAILQ_FOREACH(ent, list, link) {
		if (ent->phid == phid)
			return (ent);
		sent = plistFind(&ent->children, phid);
		if (sent != NULL)
			return (sent);
	}
	return (NULL);
}

static pent_t *
plistFindDictionary(plist_t *list, const char *label) {
	Phidget_ChannelClass cc;
	const char *lbl;
	const char *srv;
	pent_t *sent;
	pent_t *ent;

	if (list == NULL)
		list = &plist;

	MTAILQ_FOREACH(ent, list, link) {
		if (!ent->isch)
			goto next;

		Phidget_getChannelClass(ent->phid, &cc);
		if (cc != PHIDCHCLASS_DICTIONARY)
			goto next;

		if (srvname != NULL) {
			Phidget_getServerName(ent->phid, &srv);
			if (srv != NULL && mos_strlen(srv) > 0 && mos_strcmp(srvname, srv) != 0)
				goto next;
		}
		Phidget_getDeviceLabel(ent->phid, &lbl);
		if (lbl != NULL && mos_strlen(lbl) > 0 && mos_strcmp(label, lbl) != 0)
			goto next;

		return (ent);

	next:
		sent = plistFindDictionary(&ent->children, label);
		if (sent != NULL)
			return (sent);
	}
	return (NULL);
}

static void
plist_add(pent_t *ent, PhidgetHandle phid) {
	pent_t *new;

	Phidget_retain(phid);

	new = mos_malloc(sizeof(*new));
	new->phid = phid;
	Phidget_getIsChannel(phid, &new->isch);
	MTAILQ_INIT(&new->children);
	if (ent == NULL)
		MTAILQ_INSERT_TAIL(&plist, new, link);
	else
		MTAILQ_INSERT_TAIL(&ent->children, new, link);
}

static void
plistAdd(PhidgetHandle phid) {
	PhidgetHandle parent;
	pent_t *pent;

	pent = plistFind(NULL, phid);
	if (pent != NULL)
		return;

	Phidget_getParent(phid, &parent);
	if (parent)
		plistAdd(parent);

	pent = plistFind(NULL, parent);
	plist_add(pent, phid);
}

typedef void (plist_visitor_t)(int, PhidgetHandle);

static void
_plistVisit(int depth, plist_t *list, plist_visitor_t v) {
	pent_t *ent;

	MTAILQ_FOREACH(ent, list, link) {
		v(depth, ent->phid);
		_plistVisit(depth + 1, &ent->children, v);
	}
}

static int
plistVisit(plist_visitor_t v) {
	pent_t *ent;

	MTAILQ_FOREACH(ent, &plist, link) {
		v(0, ent->phid);
		_plistVisit(1, &ent->children, v);
	}

	return (0);
}

typedef void (fwlist_visitor_t)(FirmwareUpgradeFileInfo *, void *ptr);

static int
fwlistVisit(fwlist_visitor_t v, void *ptr) {
	fwent_t *ent;

	MTAILQ_FOREACH(ent, &fwlist, link)
		v(&ent->fw, ptr);

	return (0);
}

#if 0
static void
fwlistPrintVisitor(FirmwareUpgradeFileInfo *fw, void *ptr) {

	mos_printf("\t%-10s v%d\n", fw->sku, fw->version);
}
#endif

static void
fwlistPrintMatchingVisitor(FirmwareUpgradeFileInfo *fw, void *ptr) {
	PhidgetHandle device = (PhidgetHandle)ptr;
	int version = 0;

	if (device == NULL)
		return;

	if (!phidgetFWMatchesDevice(device, fw))
		return;

	Phidget_getDeviceVersion(device, &version);

	if (version == fw->version)
		mos_printf("[v%d]", fw->version);
	else
		mos_printf(" v%d ", fw->version);
}

static int
fwlistFWAvailable(PhidgetHandle device) {
	fwent_t *ent;

	if (device == NULL)
		return 0;

	MTAILQ_FOREACH(ent, &fwlist, link) {
		if (phidgetFWMatchesDevice(device, &ent->fw))
			return 1;
	}

	return 0;
}

#define PRINT_SERVERNAME	0x01
#define PRINT_LABEL			0x02

// These are not in the public header so define here
#define _PHIDID_FIRMWARE_UPGRADE_STM32F0 0x66	/* VINT Device in firmware upgrade mode, STM32F0 Proc. */
#define _PHIDID_FIRMWARE_UPGRADE_STM8S 0x67		/* VINT Device in firmware upgrade mode, STM8S Proc. */
#define _PHIDID_FIRMWARE_UPGRADE_STM32G0 0x8f	/* VINT Device in firmware upgrade mode, STM32G0 Proc. */

static void
printHandle(PhidgetHandle phid, const char *prefix, int flags) {
	Phidget_DeviceClass cls;
	Phidget_DeviceID id;
	PhidgetReturnCode res;
	const char *_label;
	const char *hname;
	const char *sku;
	char postfix[128];
	char label[128];
	const char *srv;
	char ident[128];
	int remote;
	int isport;
	int isch;
	int port;
	int ver;
	int ch;
	int sn;
	int isInFirmwareUpgrade;

	Phidget_getDeviceSerialNumber(phid, &sn);
	Phidget_getDeviceVersion(phid, &ver);
	Phidget_getDeviceClass(phid, &cls);
	Phidget_getDeviceID(phid, &id);
	Phidget_getIsChannel(phid, &isch);
	Phidget_getIsRemote(phid, &remote);
	Phidget_getHubPort(phid, &port);
	Phidget_getDeviceSKU(phid, &sku);

	isport = 0;
	Phidget_getIsHubPortDevice(phid, &isport);

	postfix[0] = '\0';
	label[0] = '\0';

	if (flags & PRINT_LABEL) {
		Phidget_getDeviceLabel(phid, &_label);
		if (_label != NULL && mos_strlen(_label) > 0)
			mos_snprintf(label, sizeof(label), "[%s]", _label);
	}

	if (remote)
		Phidget_getServerName(phid, &srv);
	else
		srv = NULL;

	if (isch) {
		res = Phidget_getChannelName(phid, &hname);
		if (res != EPHIDGET_OK)
			hname = "";
		Phidget_getChannel(phid, &ch);
		if (srv && (flags & PRINT_SERVERNAME))
			if (cls == PHIDCLASS_VINT)
				snprintf(ident, sizeof(ident), "%s - (%d/%d/%d)", srv, sn, port, ch);
			else
				snprintf(ident, sizeof(ident), "%s - (%d//%d)", srv, sn, ch);
		else
			if (cls == PHIDCLASS_VINT)
				snprintf(ident, sizeof(ident), "(%d/%d/%d)", sn, port, ch);
			else
				snprintf(ident, sizeof(ident), "(%d//%d)", sn, ch);
	} else {
		Phidget_getDeviceName(phid, &hname);
		if (srv && (flags & PRINT_SERVERNAME))
			if (cls == PHIDCLASS_VINT)
				snprintf(ident, sizeof(ident), "%s - (%d/%d)", srv, sn, port);
			else
				snprintf(ident, sizeof(ident), "%s - (%d)", srv, sn);
		else
			if (cls == PHIDCLASS_VINT)
				snprintf(ident, sizeof(ident), "(%d/%d)", sn, port);
			else
				snprintf(ident, sizeof(ident), "(%d)", sn);
		mos_snprintf(postfix, sizeof(postfix), "v%d %s", ver, label);
	}

	// Firmware upgrade mode only print upgradable devices
	if (uflag) {
		isInFirmwareUpgrade = 0;
		if ((cls == PHIDCLASS_VINT && (id == _PHIDID_FIRMWARE_UPGRADE_STM32F0 || id == _PHIDID_FIRMWARE_UPGRADE_STM8S || id == _PHIDID_FIRMWARE_UPGRADE_STM32G0))
		  || cls == PHIDCLASS_FIRMWAREUPGRADE) {
			isInFirmwareUpgrade = 1;
			// Version is meaningless if device is in firmware upgrade mode (it's the bootloader version)
			ver = -1;
		}
		if (isch || isport)
			return;
		if (!fwlistFWAvailable(phid) && !isInFirmwareUpgrade)
			return;
		// for upgrade mode, only show vint devices if a hub port was specified (if a sn was specified)
		if (serialno != -1 && hubport == -1 && cls == PHIDCLASS_VINT)
			return;
		// only show usb devices if the port isn't specified
		if (hubport != -1 && cls != PHIDCLASS_VINT)
			return;
		// Don't show devices with up to date firmware unless asked
		if (!aflag && newestFirmware(phid) <= ver)
			return;

		if ((remote && Rflag) || (!remote && Lflag)) {
			if (!qflag) {
				if (cls == PHIDCLASS_VINT)
					mos_printf("%-25s %-9d %-8d %-9s ", (srv ? srv : ""), sn, port, sku);
				else
					mos_printf("%-25s %-9d %-8s %-9s ", (srv ? srv : ""), sn, "", sku);
				fwlistVisit(fwlistPrintMatchingVisitor, phid);
				mos_printf("\n");
			} else {
				if (cls == PHIDCLASS_VINT) {
					if (isInFirmwareUpgrade) {
						mos_printf("%d,1,%d,%d,%s,,,%s\n",
							remote, sn, port, sku, (srv ? srv : ""));
					} else {
						mos_printf("%d,1,%d,%d,%s,%d,%d,%s\n",
							remote, sn, port, sku, ver, newestFirmware(phid), (srv ? srv : ""));
					}
				} else {
					if (isInFirmwareUpgrade) {
						mos_printf("%d,0,%d,%d,%s,,%d,%s\n",
							remote, sn, port, sku, newestFirmware(phid), (srv ? srv : ""));
					} else {
						mos_printf("%d,0,%d,%d,%s,%d,%d,%s\n",
							remote, sn, port, sku, ver, newestFirmware(phid), (srv ? srv : ""));
					}
				}
			}
		}
	} else {
		if ((remote && Rflag) || (!remote && Lflag))
			mos_printf("%s %s %s %s\n", prefix, ident, hname, postfix);
	}
}

static void
plistPrintVisitor(int depth, PhidgetHandle phid) {
	const char *srv;
	char prefix[8];
	int flags;
	int sn = -1, hp = -1, ch = -1;
	int i;

	if (serialno != -1) {
		Phidget_getDeviceSerialNumber(phid, &sn);
		if (sn != serialno)
			return;
	}

	if (hubport != -1) {
		Phidget_getHubPort(phid, &hp);
		if (hp != hubport)
			return;
	}

	if (channel != -1) {
		Phidget_getChannel(phid, &ch);
		if (ch != channel)
			return;
	}

	if (srvname != NULL) {
		Phidget_getServerName(phid, &srv);
		if (srv != NULL) {
			if (mos_strcmp(srvname, srv) != 0)
				return;
		} else {
			return;
		}
	}

	flags = PRINT_LABEL;

	if (depth == 0)
		flags |= PRINT_SERVERNAME;

	for (i = 0; i < depth; i++)
		prefix[i] = '\t';
	prefix[i] = '\0';
	printHandle(phid, prefix, flags);
}

static void
printkv(kv_t *kv, const char *prefix) {
	kvent_t *e;

	if (kv == NULL)
		return;

	MTAILQ_FOREACH(e, &kv->list, link)
		mos_printf("\t\t%s=%s\n", e->key, e->val);
}

static void CCONV
serverAdded(void *ctx, PhidgetServer *server, void *kv) {
	char nm[38];

	srvadd(server);

	if (!sflag)
		return;

	snprintf(nm, sizeof(nm), "%s:%d", server->host, server->port);

	mos_mutex_lock(&printLock);
	mos_printf("%-18s %-30s %-24s %-13s %s\n", server->stype, server->name, nm, server->addr,
	  server->flags & PHIDGETSERVER_AUTHREQUIRED ? "AUTH" : "");
	if (vflag)
		printkv((kv_t *)kv, "\t\t");
	fflush(stdout);
	mos_mutex_unlock(&printLock);
}

static void
listServers() {

	PhidgetNet_setOnServerAddedHandler(serverAdded, NULL);
}

#if 0
static int
deviceSeen(int sn) {
	struct devent *de;

	MTAILQ_FOREACH(de, &devlist, link) {
		if (de->serialno == sn)
			return (1);
	}
	return (0);
}

static void
addDevice(int sn) {
	struct devent *de;

	de = mos_malloc(sizeof(*de));
	de->serialno = sn;
	MTAILQ_INSERT_HEAD(&devlist, de, link);
}

static void
removeDevice(int sn) {
	struct devent *de;

	MTAILQ_FOREACH(de, &devlist, link) {
		if (de->serialno == sn) {
			MTAILQ_REMOVE(&devlist, de, link);
			mos_free(de, sizeof(*de));
			return;
		}
	}
}
#endif

static void CCONV
mgrAttach(PhidgetManagerHandle mgr, void *ctx, PhidgetHandle ch) {

	if (vflag > 2)
		mos_printf("Attach: %P\n", ch);

	plistAdd(ch);
}

static int
listChannels() {
	PhidgetManagerHandle mgr;
	PhidgetReturnCode res;

	MTAILQ_INIT(&devlist);

	res = PhidgetManager_create(&mgr);
	if (res != EPHIDGET_OK) {
		mos_printef("failed to create manager handle\n");
		return (1);
	}

	PhidgetManager_setOnAttachHandler(mgr, mgrAttach, NULL);
	PhidgetManager_open(mgr);

	return (0);
}

static PhidgetReturnCode
setDictKeyValue() {
	PhidgetReturnCode res;
	char val[65536];
	pent_t *ent;

	ent = plistFindDictionary(NULL, "Phidget22 Control");
	if (ent == NULL) {
		mos_printef("Failed to find control dictionary for %s, does the server exist?\n", srvname);
		return (EPHIDGET_NOENT);
	}
	res = Phidget_openWaitForAttachment(ent->phid, 2000);
	if (res != EPHIDGET_OK) {
		mos_printef("Failed to open control dictionary for %s:%d\n", srvname, res);
		return (res);
	}

	if (mos_strlen(dictval) == 0) {
		if (vflag)
			mos_printf("getting key [%s] from server %s\n", dictkey, srvname);

		res = PhidgetDictionary_get((PhidgetDictionaryHandle)ent->phid, dictkey, val, sizeof(val));
		if (res != EPHIDGET_OK) {
			mos_printef("Failed to get key %s on server %s:%d\n", dictkey, srvname, res);
			Phidget_close(ent->phid);
			return (res);
		}
		mos_printf("%s = [%s]\n", dictkey, val);
	} else {
		if (vflag)
			mos_printf("setting [%s] = [%s] on server %s\n", dictkey, dictval, srvname);

		res = PhidgetDictionary_set((PhidgetDictionaryHandle)ent->phid, dictkey, dictval);
		if (res != EPHIDGET_OK) {
			mos_printef("Failed to set key %s on server %s:%d\n", dictkey, res);
			Phidget_close(ent->phid);
			return (res);
		}
	}

	Phidget_close(ent->phid);
	return (EPHIDGET_OK);
}

static PhidgetReturnCode
listOpenConnections() {
	char ins[12], outs[12], evs[12];
	char cver[12], sver[12];
	const char *inu, *outu, *evu;
	uint16_t in, out, ev;
	PhidgetReturnCode res;
	char json[65536];
	pent_t *ent;
	pconf_t *pc;
	int cnt;
	int i;

	ent = plistFindDictionary(NULL, "Phidget22 Control");
	if (ent == NULL) {
		mos_printef("Failed to find control dictionary for %s, does the server exist?\n", srvname);
		return (EPHIDGET_NOENT);
	}

	res = Phidget_openWaitForAttachment(ent->phid, 2000);
	if (res != EPHIDGET_OK) {
		mos_printef("Failed to open control dictionary for %s:%d\n", srvname, res);
		return (res);
	}

	res = PhidgetDictionary_get((PhidgetDictionaryHandle)ent->phid, "openconnections", json, sizeof(json));
	Phidget_close(ent->phid);
	if (res != EPHIDGET_OK) {
		mos_printef("failed to get open connections from %s:%d\n", srvname, res);
		return (res);
	}

	res = pconf_parsejson(&pc, json, mos_strlen(json));
	if (res != EPHIDGET_OK) {
		mos_printef("failed to parse open connections json [%s]\n", json);
		return (res);
	}

	cnt = pconf_get32(pc, 0, "cnt");
	mos_printf("%d client connections to server '%s'\n\n", cnt, srvname);
	mos_printf("%-4s%-20s%-5s%-24s%-14s%-14s%-8s%-8s%-9s%-9s%-9s\n", "#", "Connected", "Open",
	  "Peer", "Type", "Protocol", "Srv Ver", "Cli Ver", "IO In",
	  "IO Out", "Events");
	mos_printf("--- ------------------- ---- ----------------------- ------------- ------------- "
		"------- ------- -------- -------- --------\n");
	for (i = 0; i < cnt; i++) {
		in = mos_bytes2units(pconf_getu32(pc, 0, "connections.%d.ioin", i), &inu);
		mos_snprintf(ins, sizeof(ins), "%u%s", in, inu);
		out = mos_bytes2units(pconf_getu32(pc, 0, "connections.%d.ioout", i), &outu);
		mos_snprintf(outs, sizeof(outs), "%u%s", out, outu);
		ev = mos_bytes2units(pconf_getu32(pc, 0, "connections.%d.ioev", i), &evu);
		mos_snprintf(evs, sizeof(evs), "%u%s", ev, evu);
		mos_snprintf(sver, sizeof(sver), "%d.%d", pconf_get32(pc, 0, "connections.%d.pmajor", i),
		  pconf_get32(pc, 0, "connections.%d.pminor", i));
		mos_snprintf(cver, sizeof(cver), "%d.%d", pconf_get32(pc, 0, "connections.%d.ppmajor", i),
		  pconf_get32(pc, 0, "connections.%d.ppminor", i));
		mos_printf("%-4d%-20s%-5d%-24s%-14s%-14s%-8s%-8s%-9s%-9s%-9s\n",
			i + 1,
			pconf_getstr(pc, "", "connections.%d.ctime", i),
			pconf_get32(pc, 0, "connections.%d.openchannels", i),
			pconf_getstr(pc, "", "connections.%d.peer", i),
			pconf_getstr(pc, "", "connections.%d.conntype", i),
			pconf_getstr(pc, "", "connections.%d.proto", i),
			sver, cver,
			ins, outs, evs);
	}

	pconf_release(&pc);
	return (EPHIDGET_OK);
}

static PhidgetReturnCode
listOpenChannels() {
	PhidgetReturnCode res;
	char json[65536];
	char port[12];
	pent_t *ent;
	pconf_t *pc;
	int nccnt;
	int ccnt;
	int i, j;

	ent = plistFindDictionary(NULL, "Phidget22 Control");
	if (ent == NULL) {
		mos_printef("Failed to find control dictionary for %s, does the server exist?\n", srvname);
		return (EPHIDGET_NOENT);
	}

	res = Phidget_openWaitForAttachment(ent->phid, 2000);
	if (res != EPHIDGET_OK) {
		mos_printef("Failed to open control dictionary for %s:%d\n", srvname, res);
		return (res);
	}

	res = PhidgetDictionary_get((PhidgetDictionaryHandle)ent->phid, "openchannels", json, sizeof(json));
	Phidget_close(ent->phid);
	if (res != EPHIDGET_OK) {
		mos_printef("failed to get open connections from %s:%d\n", srvname, res);
		return (res);
	}

	res = pconf_parsejson(&pc, json, mos_strlen(json));
	if (res != EPHIDGET_OK) {
		mos_printef("Failed to parse open channels json\n");
		return (res);
	}

	ccnt = pconf_get32(pc, 0, "cnt");
	mos_printf("%d open channels on server '%s'\n\n", ccnt, srvname);
	mos_printf("%-24s%-28s%-10s%-7s%-8s%-5s%-5s%-20s\n", "Name", "Class", "Conn", "Type", "SN", "Port",
	  "Ch", "Label");
	mos_printf("----------------------- --------------------------- --------- ------ ------- ---- ---- "
	  "--------------------\n");
	for (i = 0; i < ccnt; i++) {
		if (pconf_getbool(pc, 0, "channels.%d.ishubport", i))
			mos_snprintf(port, sizeof(port), "%d*", pconf_get32(pc, -1, "channels.%d.port", i));
		else
			mos_snprintf(port, sizeof(port), "%d", pconf_get32(pc, -1, "channels.%d.port", i));
		mos_printf("%-24s%-28s%-10s%-7s%-8d%-5s%-5d%-20s\n",
		  pconf_getstr(pc, "", "channels.%d.name", i),
		  pconf_getstr(pc, "", "channels.%d.class", i),
		  pconf_getstr(pc, "", "channels.%d.conntype", i),
		  pconf_getbool(pc, 0, "channels.%d.network", i) ? "local" : "net",
		  pconf_get32(pc, -1, "channels.%d.sn", i),
		  port,
		  pconf_get32(pc, -1, "channels.%d.ch", i),
		  pconf_getstr(pc, "", "channels.%d.label", i));


		nccnt = pconf_get32(pc, 0, "channels.%d.netconncnt", i);
		if (nccnt > 0) {
			mos_printf("\n\t%d remote connection(s)\n", nccnt);
			for (j = 0; j < nccnt; j++)
				mos_printf("\t%-22s\n", pconf_getstr(pc, "", "channels.%d.conn.%d.peer", i, j));
			mos_printf("\n");
		}
	}

	pconf_release(&pc);

	return (EPHIDGET_OK);
}

static int
parseKeyValue(const char *kv) {
	const char *s, *c;
	char val[128];
	char key[32];

	for (s = c = kv; *c; c++) {
		if (*c == '=') {
			if (s == c)
				return (1);
			if (c - s >= (int)(sizeof(key) - 1))
				return (1);
			memset(key, 0, sizeof(key));
			mos_strncpy(key, s, c - s);

			c++;
			if (*c == '\0')
				return (1);
			mos_strlcpy(val, c, sizeof(val));
			mos_strtrim(key, dictkey, sizeof(dictkey));
			if (mos_strlen(dictkey) == 0)
				return (1);
			mos_strtrim(val, dictval, sizeof(dictval));
			if (mos_strlen(dictval) == 0)
				return (1);
			return (0);
		}
	}

	/* If no =, then we do a get instead of a set */
	mos_strtrim(kv, dictkey, sizeof(dictkey));

	return (0);
}

static int
parseMatch(const char *match) {
	const char *c, *s;
	char val[32];
	int args[3];
	int off;
	int res;

	off = 0;
	args[0] = -1;
	args[1] = -1;
	args[2] = -1;

	for (s = c = match; *c; c++) {
		switch (*c) {
		case '/':
			if (s == c) {
				s = c + 1;
			} else {
				if (c - s > (int)sizeof(val) - 1)
					return (1);
				memset(val, 0, sizeof(val));
				mos_strncpy(val, s, c - s);
				res = mos_strto32(val, 0, &args[off]);
				s = c + 1;
				if (res != 0)
					return (1);
			}
			off++;
			if (off > 2)
				return (1);
			break;
		}
	}

	if (s != c) {
		if (c - s > (int)sizeof(val) - 1)
			return (1);
		memset(val, 0, sizeof(val));
		mos_strncpy(val, s, c - s);
		res = mos_strto32(val, 0, &args[off]);
		if (res != 0)
			return (1);
	}

	serialno = args[0];
	hubport = args[1];
	channel = args[2];

	if (hubport >= HUB_PORT_MAX) {
		hubportmode = 1;
		hubport -= HUB_PORT_MAX;
		if (hubport >= HUB_PORT_MAX)
			return (1);
	}

	return (0);
}

// This matches devices NOT channels
static int
fwUpgradeDeviceMatch(PhidgetHandle phid) {
	int isport = -1;
	int isch = -1;
	int port = -1;
	int sn = -1;
	const char *srv = NULL;
	Phidget_DeviceClass deviceClass = PHIDCLASS_NOTHING;

	Phidget_getDeviceSerialNumber(phid, &sn);
	Phidget_getIsChannel(phid, &isch);
	Phidget_getHubPort(phid, &port);
	Phidget_getIsHubPortDevice(phid, &isport);
	Phidget_getDeviceClass(phid, &deviceClass);
	Phidget_getServerName(phid, &srv);

	if (isch == 1 || isport == 1)
		return 0;

	if (serialno != sn)
		return 0;

	if (srv != NULL) {
		if (mos_strcmp(srvname, srv) != 0)
			return 0;
	}

	if (deviceClass == PHIDCLASS_VINT) {
		// If we didn't specify a hub port don't consider VINT devices
		if (hubport == -1)
			return 0;
		if (hubport != port)
			return 0;
	} else {
		// If we specified a hub port only consider VINT devices
		if (hubport != -1)
			return 0;
	}

	return 1;
}

static pent_t *
plistFindFWUpgradeDevice(plist_t *list, pent_t **match, int *matchCnt) {
	pent_t *sent;
	pent_t *ent;

	if (list == NULL)
		list = &plist;

	MTAILQ_FOREACH(ent, list, link) {
		if (fwUpgradeDeviceMatch(ent->phid)) {
			(*match) = ent;
			(*matchCnt)++;
		}
		sent = plistFindFWUpgradeDevice(&ent->children, match, matchCnt);
		if (sent != NULL)
			return (sent);
	}
	return (NULL);
}

static int newestFirmware(PhidgetHandle ch) {
	fwent_t *fwent = NULL;
	int fwversion = 0;

	// Find the newest firmware (or a specified version) and upgrade
	MTAILQ_FOREACH(fwent, &fwlist, link) {
		if (phidgetFWMatchesDevice(ch, &fwent->fw)) {
			if (fwversionoverride >= 0) {
				if (fwent->fw.version == fwversionoverride) {
					fwversion = fwent->fw.version;
					break;
				}
			} else {
				if (fwent->fw.version > fwversion) {
					fwversion = fwent->fw.version;
				}
			}
		}
	}

	return fwversion;
}

static int
doFirmwareUpgrade(void) {
	int res = 0;
	pent_t *match = NULL;
	fwent_t *fwent = NULL;
	PhidgetHandle dev = NULL, ch = NULL;
	int devmatches = 0;
	int fwversion = 0;
	int version = 0;
	FirmwareUpgradeFileInfo *fwfileinfo = NULL;
	Phidget_ChannelClass channelClass = PHIDCHCLASS_NOTHING;
	Phidget_DeviceClass deviceClass = PHIDCLASS_NOTHING;
	PhidgetReturnCode ret;

	plistFindFWUpgradeDevice(&plist, &match, &devmatches);

	if (devmatches < 1) {
		mos_printef("Couldn't find a matching device for firmware upgrade\n");
		return (1);
}

#if 0
	if (devmatches > 1) {
		mos_printef("Found multiple matching devices for firmware upgrade\n");
		return (1);
	}
#endif

	dev = match->phid;
	ch = match->children.tqh_first->phid;

	if (dev == NULL || ch == NULL) {
		mos_printef("Couldn't find a matching device for firmware upgrade\n");
		return (1);
	}

	// Did we select a VINT device in firmware upgrade mode?
	Phidget_getDeviceClass(ch, &deviceClass);
	Phidget_getChannelClass(ch, &channelClass);
	Phidget_getDeviceVersion(ch, &version);
	if (deviceClass == PHIDCLASS_VINT && channelClass == PHIDCHCLASS_FIRMWAREUPGRADE) {
		// We need to open the device to discover the correct firmware
		ret = Phidget_openWaitForAttachment(ch, 1500);
		if (ret != EPHIDGET_OK) {
			fprintf(stderr, "Error on open wait for attach: %s\n", Phidget_strerror(ret));
			return (1);
		}
	}

	// Find the newest firmware (or a specified version) and upgrade
	MTAILQ_FOREACH(fwent, &fwlist, link) {
		if (phidgetFWMatchesDevice(ch, &fwent->fw)) {
			if (fwversionoverride >= 0) {
				if (fwent->fw.version == fwversionoverride) {
					fwversion = fwent->fw.version;
					fwfileinfo = &fwent->fw;
					break;
				}
			} else {
				if (!fwallowmajorupgrade) {
					if ((int)(fwent->fw.version / 100) != (int)(version / 100))
						continue;
				}
				if (fwent->fw.version > fwversion && fwent->fw.version > version) {
					fwversion = fwent->fw.version;
					fwfileinfo = &fwent->fw;
				}
			}
		}
	}

	if (fwfileinfo == NULL) {
		if (fwversionoverride >= 0) {
			mos_printef("Couldn't find specified firmware version v%d\n", fwversionoverride);
		} else {
			mos_printef("Couldn't find firmware to upgrade this device\n");
			mos_printef("If newer firmware is a major upgrade (100+), -m must be specified.\n");
		}
		return (1);
	}

	mos_printf("Selected Firmware version v%d for upgrade.\n", fwfileinfo->version);
	res = upgradePhidgetFirmware(ch, fwfileinfo, (fwversionoverride >= 0 ? 1 : 0));

	return (res);
}

static PhidgetReturnCode
setAuth() {
	PhidgetReturnCode res;
	char prompt[44];
	char passwd[32];
	int err;

	if (srvname == NULL)
		return (EPHIDGET_INVALID);

	mos_snprintf(prompt, sizeof(prompt), "Password>");
	err = mos_getpasswd(NULL, prompt, passwd, sizeof(passwd));
	if (err != 0) {
		mos_printef("failed to get the password\n");
		return (EPHIDGET_UNEXPECTED);
	}

	res = PhidgetNet_setServerPassword(srvname, passwd);
	if (res != EPHIDGET_OK)
		mos_printef("Failed to set password:%s\n", getErrorStr(res));
	return (res);
}

int
main(int argc, char **argv) {
	int res;
	int ch;
	int i;

	MTAILQ_INIT(&srvlist);
	MTAILQ_INIT(&plist);
	MTAILQ_INIT(&fwlist);

	mos_mutex_init(&printLock);

	register_signalhandlers();

	while ((ch = mos_getopt(argc, argv, "AF:H:LM:RUV:acdk:lmoqsuvw:-")) != -1) {
		switch (ch) {
		case 'A':
			Aflag++;
			break;
		case 'F':
			userfwpathname = mos_optarg;
			break;
		case 'H':
			srvname = mos_optarg;
			break;
		case 'L':
			Lflag++;
			break;
		case 'M':
			i = parseMatch(mos_optarg);
			if (i != 0) {
				mos_printef("invalid match: %s\n", mos_optarg);
				usage(argv[0], 1);
				/* NOT REACHED */
			}
			break;
		case 'R':
			Rflag++;
			break;
		case 'U':
			Uflag++;
			uflag++;
			break;
		case 'V':
			i = mos_strto32(mos_optarg, 0, &fwversionoverride);
			if (i != 0 || fwversionoverride < 0) {
				mos_printef("invalid firmware version: %s\n", mos_optarg);
				usage(argv[0], 1);
				/* NOT REACHED */
			}
			break;
		case 'm':
			fwallowmajorupgrade++;
			break;
		case 'a':
			aflag++;
			break;
		case 'c':
			cflag++;
			break;
		case 'd':
			dflag++;
			break;
		case 'k':
			kflag++;
			if (parseKeyValue(mos_optarg) != 0)
				usage(argv[0], 1);
				/* NOT REACHED */
			break;
		case 'l':
			lflag++;
			break;
		case 'o':
			oflag++;
			break;
		case 'q':
			qflag++;
			break;
		case 's':
			sflag++;
			break;
		case 'u':
			uflag++;
			break;
		case 'v':
			vflag++;
			break;
		case 'w':
			i = mos_strto32(mos_optarg, 0, &waittime);
			if (i != 0 || waittime < 0 || waittime > 60) {
				mos_printef("invalid wait time: %s\n", mos_optarg);
				usage(argv[0], 1);
				/* NOT REACHED */
			}
			waittime = waittime * 1000000;
			break;
		case '-':
			usage(argv[0], 0); /* To handle --help and --version arguments */
			/* NOT REACHED */
		default:
			usage(argv[0], 1);
			/* NOT REACHED */
		}
	}

	if (cflag == 0 && dflag == 0 && kflag == 0 && oflag == 0 && sflag == 0 && uflag == 0)
		usage(argv[0], 1);
		/* NOT REACHED */

	if (kflag && srvname == NULL)
		usage(argv[0], 1);
		/* NOT REACHED */

	if ((Aflag || oflag || cflag) && srvname == NULL) {
		mos_printef("server name required (-H)\n");
		usage(argv[0], 1);
		/* NOT REACHED */
	}

	argc -= mos_optind;
	argv += mos_optind;

	if (Uflag) {

		// Make sure they have specified a single unique device
		if (serialno <= 0) {
			mos_printef("Serial number must be specified for firmware upgrade\n");
			return (1);
		}

		if (srvname == NULL && Rflag) {
			mos_printef("Server name must be specified for remote firmware upgrade\n");
			return (1);
		}

		if (srvname != NULL && Lflag) {
			mos_printef("Server name should not be specified with local flag\n");
			return (1);
		}

		if (srvname != NULL) {
			Rflag = 1;
			Lflag = 0;
		} else {
			Rflag = 0;
			Lflag = 1;
		}
	}

	if (Lflag == 0 && Rflag == 0)
		Lflag = Rflag = 1;

	if (lflag > 0) {
		lflag = PHIDGET_LOG_ERROR + (lflag - 1);
		if (lflag > PHIDGET_LOG_VERBOSE)
			lflag = PHIDGET_LOG_VERBOSE;

		res = PhidgetLog_enable(lflag, "phidget22admin.log");
		if (res != EPHIDGET_OK)
			mos_printef("failed to enable logging: %s\n", getErrorStr(res));
	}

	listChannels();
	listServers();

	// Firmware upgrade mode
	if (uflag) {

		if (userfwpathname == NULL) {
			res = readPhidgetFirmwareFiles(DEFAULT_FIRMWARE_PATH);
#ifdef AM_FIRMWAREDATADIR
			if (res)
				res = readPhidgetFirmwareFiles(AM_FIRMWAREDATADIR);
#endif
		} else {
			res = readPhidgetFirmwareFiles(userfwpathname);
		}

		if (res) {
			mos_printef("failed to read in firmware files\n");
			exit(res);
		}

#if 0
		// Print firmware files
		mos_mutex_lock(&printLock);
		mos_printf("Available Firmware Upgrade Files:\n");
		fwlistVisit(fwlistPrintVisitor, NULL);
		mos_printf("\n");
		mos_mutex_unlock(&printLock);
#endif

		if (dflag && !qflag) {
			if (aflag)
				mos_printf("Finding matching devices that support firmware upgrade...\n\n");
			else
				mos_printf("Finding matching devices that require a firmware upgrade...\n\n");
			mos_printf("%-25s %-9s %-8s %-9s %-20s", "ServerName", "SerialNo", "HubPort", "SKU", "Firmware [Installed]\n");
			mos_printf("------------------------- --------- -------- --------- ---------------------\n");
		}
		}

	if (Rflag || sflag)
		PhidgetNet_enableServerDiscovery(PHIDGETSERVER_DEVICEREMOTE);
	if (sflag) {
		PhidgetNet_enableServerDiscovery(PHIDGETSERVER_WWWREMOTE);
		PhidgetNet_enableServerDiscovery(PHIDGETSERVER_SBC);
	}

	if (Aflag) {
		res = setAuth();
		if (res != EPHIDGET_OK)
			exit(res);
	}

	mos_usleep(waittime);	/* give the event handlers time to run */

	res = 0;
	mos_mutex_lock(&printLock);
	if (cflag || oflag || dflag) {
		if (dflag)
			res = plistVisit(plistPrintVisitor);
		if (res == EPHIDGET_OK && cflag)
			res = listOpenChannels();
		if (res == EPHIDGET_OK && oflag)
			res = listOpenConnections();
	} else if (kflag) {
		res = setDictKeyValue();
	}
	mos_mutex_unlock(&printLock);

	if (res != EPHIDGET_OK)
		exit(res);

	if (Uflag) {
		if (!qflag)
			mos_printf("\n");
		res = doFirmwareUpgrade();
		if (res != EPHIDGET_OK)
			exit(res);
	}

	exit(res);
	}
