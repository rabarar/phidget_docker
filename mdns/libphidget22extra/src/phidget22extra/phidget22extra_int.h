#ifndef _PHIDGET_EXTRA_INT_H_
#define _PHIDGET_EXTRA_INT_H_

#include "mos/mos_os.h"
#include "mos/mos_iop.h"
#include "mos/mos_lock.h"
#include "mos/mos_time.h"
#include "mos/bsdtree.h"
#include "mos/mos_task.h"
#include "mos/kv/kv.h"

#include "phidget22.h"

#define PHIDGETEXTRA_VERSION	"1.0"

#if defined(Windows)

#define PHIDIAPI __declspec(dllimport)
#ifndef CCONV
#define CCONV __stdcall
#endif

#ifdef PHIDGETEXTRA_SRC
#define PHIDAPI __declspec(dllexport)
#else /* !PHIDGETEXTRA_SRC */
#define PHIDAPI
#endif /* PHIDGETEXTRA_SRC */

#include "io.h"
#define open _open
#define close _close
#define read _read
#define write _write
#define unlink _unlink

#else /* !Windows */

#define PHIDIAPI
#ifndef CCONV
#define CCONV
#endif

#ifdef PHIDGETEXTRA_SRC
#define PHIDAPI __attribute__((visibility("default")))
#else /* !PHIDGETEXTRA_SRC */
#define PHIDAPI
#endif /* PHIDGETEXTRA_SRC */

PHIDAPI void removepid(const char *);

#endif /* Windows */

/*
 * Phidget22 Exported Functions
 */
PHIDIAPI PhidgetReturnCode CCONV createTypedPhidgetChannelHandle(PhidgetHandle *,
  Phidget_ChannelClass);
PHIDIAPI void CCONV PhidgetRelease(void *);
PHIDIAPI void CCONV PhidgetRetain(void *);

#endif /* _PHIDGET_EXTRA_INT_H_ */
