#ifndef _PHIDGET_CONFIG_H_
#define _PHIDGET_CONFIG_H_

#include "phidget22extra_int.h"
#include "mos/bsdtree.h"

#define pclogerr(...) PhidgetLog_loge(__FILE__, __LINE__, __func__, "_pconf", PHIDGET_LOG_ERROR, __VA_ARGS__)
#define pclogdebug(...) PhidgetLog_loge(__FILE__, __LINE__, __func__, "_pconf", PHIDGET_LOG_DEBUG, __VA_ARGS__)

#define PHIDGETCONFIG_PATH_MAX		512
#define PHIDGETCONFIG_COMP_MAX		64
#define PHIDGETCONFIG_STR_MAX		8192
#define MAX_KEY_DEPTH				64	/* used by parser */

typedef enum pconftype {
	PHIDGETCONFIG_BLOCK = 1,
	PHIDGETCONFIG_ARRAY,
	PHIDGETCONFIG_STRING,
	PHIDGETCONFIG_NUMBER,
	PHIDGETCONFIG_U64,
	PHIDGETCONFIG_I64,
	PHIDGETCONFIG_BOOLEAN,
	PHIDGETCONFIG_NULL
} pconftype_t;

typedef union pconfvalue {
	double					num;
	int64_t					i64;
	uint64_t				u64;
	char					*str;
	const char				*cstr;
	int						bl;
} pconfvalue_t;

typedef struct _pconf {
	void *ctx;
} pconf_t;

PHIDIAPI PhidgetReturnCode CCONV pconf_create(pconf_t **);
PHIDIAPI PhidgetReturnCode CCONV pconf_parsejson(pconf_t **, const char *, size_t);
PHIDIAPI PhidgetReturnCode CCONV pconf_renderjson(pconf_t *, char *, size_t);
PHIDIAPI PhidgetReturnCode CCONV pconf_renderpc(pconf_t *, char *, size_t);
PHIDIAPI PhidgetReturnCode CCONV pconf_release(pconf_t **);
PHIDIAPI PhidgetReturnCode CCONV pconf_merge(pconf_t *, pconf_t **, const char *, const char *, ...);
PHIDIAPI PhidgetReturnCode CCONV pconf_setcreatemissing(pconf_t *, int);
PHIDIAPI PhidgetReturnCode CCONV pconf_removev(pconf_t *, const char *, va_list);
PHIDIAPI PhidgetReturnCode CCONV pconf_remove(pconf_t *, const char *, ...);
PHIDIAPI const char * CCONV pconf_getentryname(pconf_t *, int, const char *, ...);
PHIDIAPI int32_t CCONV pconf_getcount(pconf_t *, const char *, ...);
PHIDIAPI PhidgetReturnCode CCONV pconf_detecttype(const char *, pconftype_t *, pconfvalue_t *);
PHIDIAPI PhidgetReturnCode CCONV pconf_cast(const char *, pconftype_t, pconfvalue_t *);

PHIDIAPI PhidgetReturnCode CCONV pconf_addblockv(pconf_t *, const char *, va_list);
PHIDIAPI PhidgetReturnCode CCONV pconf_addblock(pconf_t *, const char *, ...);
PHIDIAPI PhidgetReturnCode CCONV pconf_addarrayv(pconf_t *, const char *, va_list);
PHIDIAPI PhidgetReturnCode CCONV pconf_addarray(pconf_t *, const char *, ...);
PHIDIAPI PhidgetReturnCode CCONV pconf_addstrv(pconf_t *, const char *, const char *, va_list);
PHIDIAPI PhidgetReturnCode CCONV pconf_addstr(pconf_t *, const char *, const char *, ...);
PHIDIAPI PhidgetReturnCode CCONV pconf_addnumv(pconf_t *, double, const char *, va_list);
PHIDIAPI PhidgetReturnCode CCONV pconf_addnum(pconf_t *, double, const char *, ...);
PHIDIAPI PhidgetReturnCode CCONV pconf_addiv(pconf_t *, int64_t, const char *, va_list);
PHIDIAPI PhidgetReturnCode CCONV pconf_addi(pconf_t *, int64_t, const char *, ...);
PHIDIAPI PhidgetReturnCode CCONV pconf_adduv(pconf_t *, uint64_t, const char *, va_list);
PHIDIAPI PhidgetReturnCode CCONV pconf_addu(pconf_t *, uint64_t, const char *, ...);
PHIDIAPI PhidgetReturnCode CCONV pconf_addboolv(pconf_t *, int, const char *, va_list);
PHIDIAPI PhidgetReturnCode CCONV pconf_addbool(pconf_t *, int, const char *, ...);
PHIDIAPI PhidgetReturnCode CCONV pconf_addv(pconf_t *, const char *, const char *, va_list);
PHIDIAPI PhidgetReturnCode CCONV pconf_add(pconf_t *, const char *, const char *, ...);
PHIDIAPI PhidgetReturnCode CCONV pconf_updatev(pconf_t *, const char *, const char *, va_list);
PHIDIAPI PhidgetReturnCode CCONV pconf_update(pconf_t *, const char *, const char *, ...);
PHIDIAPI PhidgetReturnCode CCONV pconf_setv(pconf_t *, const char *, const char *, va_list);
PHIDIAPI PhidgetReturnCode CCONV pconf_set(pconf_t *, const char *, const char *, ...);

PHIDIAPI PhidgetReturnCode CCONV pconf_tostring(pconf_t *, char *, size_t, const char *, ...);
PHIDIAPI int32_t CCONV pconf_get32v(pconf_t *, int32_t, const char *, va_list);
PHIDIAPI int32_t CCONV pconf_get32(pconf_t *, int32_t, const char *, ...);
PHIDIAPI uint32_t CCONV pconf_getu32v(pconf_t *, uint32_t, const char *, va_list);
PHIDIAPI uint32_t CCONV pconf_getu32(pconf_t *, uint32_t, const char *, ...);
PHIDIAPI int64_t CCONV pconf_get64v(pconf_t *, int64_t, const char *, va_list);
PHIDIAPI int64_t CCONV pconf_get64(pconf_t *, int64_t, const char *, ...);
PHIDIAPI uint64_t CCONV pconf_getu64v(pconf_t *, uint64_t, const char *, va_list);
PHIDIAPI uint64_t CCONV pconf_getu64(pconf_t *, uint64_t, const char *, ...);
PHIDIAPI double CCONV pconf_getdblv(pconf_t *, double, const char *, va_list);
PHIDIAPI double CCONV pconf_getdbl(pconf_t *, double, const char *, ...);
PHIDIAPI const char * CCONV pconf_getstrv(pconf_t *, const char *, const char *, va_list);
PHIDIAPI const char * CCONV pconf_getstr(pconf_t *, const char *, const char *, ...);
PHIDIAPI int CCONV pconf_getboolv(pconf_t *, int, const char *, va_list);
PHIDIAPI int CCONV pconf_getbool(pconf_t *, int, const char *, ...);

PHIDIAPI int CCONV pconf_existsv(pconf_t *, const char *, va_list);
PHIDIAPI int CCONV pconf_exists(pconf_t *, const char *, ...);
PHIDIAPI int CCONV pconf_isblockv(pconf_t *, const char *, va_list);
PHIDIAPI int CCONV pconf_isblock(pconf_t *, const char *, ...);
PHIDIAPI int CCONV pconf_isarrayv(pconf_t *, const char *, va_list);
PHIDIAPI int CCONV pconf_isarray(pconf_t *, const char *, ...);

PHIDAPI PhidgetReturnCode CCONV pconf_parsepcs(pconf_t **, char *, size_t, const char *str, ...);
PHIDAPI PhidgetReturnCode CCONV pconf_parsepc(pconf_t **, char *, size_t, const char *file, ...);

PHIDAPI PhidgetReturnCode CCONV pconf_parsepc_locked(pconf_t **, char *, size_t, const char *file, ...);
PHIDAPI PhidgetReturnCode CCONV pconf_renderpc_locked(pconf_t **);
PHIDAPI PhidgetReturnCode CCONV pconf_unlock_locked(pconf_t **);

#endif /* _PHIDGET_CONFIG_H_ */
