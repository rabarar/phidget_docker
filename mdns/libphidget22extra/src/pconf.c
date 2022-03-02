#include "phidget22extra/phidgetconfig.h"
#include "mos/mos_fileio.h"
#include "mos/mos_lock.h"

#include <sys/stat.h>

#define PCONFLOCK	"pconf_lock"

#define MAXPCSIZE	(256 * 1024)

PhidgetReturnCode pconf_parsepcsv(pconf_t **, char *, size_t, const char *, va_list);

PhidgetReturnCode CCONV
pconf_parsepcs(pconf_t **upc, char *errbuf, size_t errbufsz, const char *fmt, ...) {
	PhidgetReturnCode res;
	va_list va;

	/*
	 * The parser uses global variables, so cannot be run in parallel.
	 */
	mos_glock((void *)0x6);
	va_start(va, fmt);
	res = pconf_parsepcsv(upc, errbuf, errbufsz, fmt, va);
	va_end(va);
	mos_gunlock((void *)0x6);

	return (res);
}

static PhidgetReturnCode
pconf_parsepcv(pconf_t **upc, char *errbuf, size_t errbufsz, const char *fmt, va_list va) {
	char path[MOS_PATH_MAX];
	PhidgetReturnCode res;
	struct stat sb;
	char *data;
	size_t n;
	int err;
	int i;

	n = mos_vsnprintf(path, sizeof (path), fmt, va);
	if (n >= sizeof (path))
		return (EPHIDGET_NOSPC);

	err = stat(path, &sb);
	if (err != 0)
		return (EPHIDGET_IO);

	if (sb.st_size >= MAXPCSIZE)
		return (EPHIDGET_NOSPC);

	n = sb.st_size + 1;
	data = mos_zalloc(n);

	for (i = 0; i <= 3; i++) {
		res = mos_file_readx(MOS_IOP_IGNORE, data, &n, "%s", path);
		if (res == 0)
			break;
		if (res == MOSN_BUSY && i < 3) {
			mos_usleep(250000);
			continue;
		}
		goto bad;
	}

	res = pconf_parsepcs(upc, errbuf, errbufsz, "%s", data);

bad:
	mos_free(data, sb.st_size + 1);
	return (res);
}

PhidgetReturnCode CCONV
pconf_parsepc(pconf_t **upc, char *errbuf, size_t errbufsz, const char *fmt, ...) {
	PhidgetReturnCode res;
	va_list va;

	va_start(va, fmt);
	res = pconf_parsepcv(upc, errbuf, errbufsz, fmt, va);
	va_end(va);

	return (res);
}

/*
 * Reads and parses the specified file with the global pconf lock held,
 * and returns the pconf object with the lock still held.
 * The lock is valid across processes.
 *
 * The caller is expected to update the pconf structure and then write the object with
 * pconf_render_locked().
 */
PhidgetReturnCode CCONV
pconf_parsepc_locked(pconf_t **upc, char *errbuf, size_t errbufsz, const char *fmt, ...) {
	char path[MOS_PATH_MAX];
	PhidgetReturnCode res;
	struct stat sb;
	mos_file_t *mf;
	char *data;
	va_list va;
	size_t n;
	int err;

	errbuf[0] = '\0';

	va_start(va, fmt);
	n = mos_vsnprintf(path, sizeof (path), fmt, va);
	va_end(va);
	if (n >= sizeof (path))
		return (EPHIDGET_NOSPC);

	res = mos_file_open(MOS_IOP_IGNORE, &mf, MOS_FILE_READ | MOS_FILE_WRITE | MOS_FILE_LOCK,
	  "%s", path);
	if (res != 0) {
		mos_snprintf(errbuf, errbufsz, "failed to open file %s", path);
		return (res);
	}

	err = stat(path, &sb);
	if (err != 0)
		return (EPHIDGET_IO);

	if (sb.st_size >= MAXPCSIZE)
		return (EPHIDGET_NOSPC);

	n = sb.st_size + 1;
	data = mos_malloc(n);

	res = mos_file_read(MOS_IOP_IGNORE, mf, data, &n);
	if (res != 0) {
		mos_snprintf(errbuf, errbufsz, "failed to read file %s", path);
		mos_file_close(MOS_IOP_IGNORE, &mf);
		mos_free(data, sb.st_size + 1);
		return (res);
	}

	res = pconf_parsepcs(upc, errbuf, errbufsz, "%s", data);
	mos_free(data, sb.st_size + 1);
	if (res == 0) {
		(*upc)->ctx = mf;
		return (EPHIDGET_OK);
	}

	mos_file_close(MOS_IOP_IGNORE, &mf);
	return (res);
}

/*
 * Unlocks the locked pconf object.  The lock and the pconf object are released,
 * unless the pconf object was not from pconf_parsepc_lock(),
 * in which case INVALIDARG is returned.
 *
 * The pconf object pointer must have come from pconf_parsepc_locked(), and will be NULL on return.
 *
 * This call is intended for use when mos_renderpc_locked() is not desired for some reason.
 */
PhidgetReturnCode CCONV
pconf_unlock_locked(pconf_t **upc) {
	mos_file_t *mf;

	if ((*upc)->ctx == NULL)
		return (EPHIDGET_INVALIDARG);

	mf = (*upc)->ctx;

	mos_file_close(MOS_IOP_IGNORE, &mf);
	pconf_release(upc);

	return (EPHIDGET_OK);
}

/*
 * Renders the locked pconf object.  The lock and the pconf object are release even in the
 * case of error, unless the pconf object was not from pconf_parsepc_lock(),
 * in which case INVALIDARG is returned.
 *
 * The pconf object pointer must have come from pconf_parsepc_locked(), and will be NULL on return.
 */
PhidgetReturnCode CCONV
pconf_renderpc_locked(pconf_t **upc) {
	PhidgetReturnCode res;
	mos_file_t *mf;
	char *data;

	if ((*upc)->ctx == NULL)
		return (EPHIDGET_INVALIDARG);

	mf = (*upc)->ctx;

	data = mos_malloc(MAXPCSIZE);
	res = pconf_renderpc(*upc, data, MAXPCSIZE);
	if (res != EPHIDGET_OK)
		goto bad;

	res = mos_file_trunc(MOS_IOP_IGNORE, mf, 0);
	if (res != 0)
		goto bad;

	res = mos_file_seek(MOS_IOP_IGNORE, mf, 0);
	if (res != 0)
		goto bad;

	res = mos_file_write(MOS_IOP_IGNORE, mf, data, mos_strlen(data));

bad:
	mos_file_close(MOS_IOP_IGNORE, &mf);
	mos_free(data, MAXPCSIZE);
	pconf_release(upc);

	return (res);
}