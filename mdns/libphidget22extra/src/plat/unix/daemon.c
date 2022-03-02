#include "phidget22extra/projectsupport.h"
#include "phidget22extra/phidgetloghelpers.h"
#include "mos/init_daemon.h"

static void
writepid(const char *pidfile) {
	FILE *fp;

	if (pidfile == NULL)
		return;

	if (mos_strlen(pidfile) < 6)
		return;

	/*
	 * Only allow under /var for security reasons: ie. we don't unlink just anything.
	 */
	if (mos_strncmp(pidfile, "/var", 4) != 0)
		return;

	unlink(pidfile);

	fp = fopen(pidfile, "w");
	if (fp == NULL)
		return;
	fprintf(fp, "%d\n", getpid());
	fclose(fp);
}

PHIDAPI void
removepid(const char *pidfile) {

	if (pidfile == NULL)
		return;

	if (mos_strlen(pidfile) < 6)
		return;

	/*
	 * Only allow under /var for security reasons: ie. we don't unlink just anything.
	 */
	if (mos_strncmp(pidfile, "/var", 4) != 0)
		return;

	unlink(pidfile);
}

PHIDAPI PhidgetReturnCode
startDaemon(const char *name, daemonstart_t start, daemonstop_t stop, void *ctx, const char *pidfile) {
	PhidgetReturnCode res;

	if (init_daemon(0) != 0) {
		suplogerr("init_daemon() failed");
		return (EPHIDGET_UNEXPECTED);
	}

	writepid(pidfile);

	res = start(ctx);
	if (res != EPHIDGET_OK)
		suplogerr("'%s' failed to run", name);

	return (res);
}
