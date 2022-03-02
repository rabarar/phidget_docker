#include "phidget22extra/phidgetdicthelpers.h"
#include "phidget22extra/phidgetconfig.h"

PhidgetReturnCode CCONV
dictionarySet(PhidgetDictionaryHandle dict, const char *key, const char *fmt, ...) {
	char val[1024];
	va_list va;

	va_start(va, fmt);
	mos_vsnprintf(val, sizeof (val), fmt, va);
	va_end(va);

	return (PhidgetDictionary_set(dict, key, val));
}

const char * CCONV
dictionaryGet(PhidgetDictionaryHandle dict, char *val, size_t valsz, const char *def,
  const char *fmt, ...) {
	PhidgetReturnCode res;
	char key[PHIDGETCONFIG_PATH_MAX];
	va_list va;

	va_start(va, fmt);
	mos_vsnprintf(key, sizeof (key), fmt, va);
	va_end(va);

	res = PhidgetDictionary_get(dict, key, val, valsz);
	if (res == EPHIDGET_OK)
		return (val);

	return (def);
}

double CCONV
dictionaryGetDoublev(PhidgetDictionaryHandle dict, double def, const char *fmt, va_list va) {
	char key[PHIDGETCONFIG_PATH_MAX];
	PhidgetReturnCode res;
	pconfvalue_t eval;
	char val[1024];
	size_t n;

	n = mos_vsnprintf(key, sizeof (key), fmt, va);
	if (n >= sizeof (key))
		return (def);

	res = PhidgetDictionary_get(dict, key, val, sizeof (val));
	if (res != EPHIDGET_OK)
		return (def);

	res = pconf_cast(val, PHIDGETCONFIG_NUMBER, &eval);
	if (res != EPHIDGET_OK)
		return (def);

	return (eval.num);
}

double CCONV
dictionaryGetDouble(PhidgetDictionaryHandle dict, double def, const char *fmt, ...) {
	double res;
	va_list va;

	va_start(va, fmt);
	res = dictionaryGetDoublev(dict, def, fmt, va);
	va_end(va);

	return (res);
}

int64_t CCONV
dictionaryGetI64v(PhidgetDictionaryHandle dict, int64_t def, const char *fmt, va_list va) {
	char key[PHIDGETCONFIG_PATH_MAX];
	PhidgetReturnCode res;
	pconfvalue_t eval;
	char val[1024];
	size_t n;

	n = mos_vsnprintf(key, sizeof (key), fmt, va);
	if (n >= sizeof (key))
		return (def);

	res = PhidgetDictionary_get(dict, key, val, sizeof (val));
	if (res != EPHIDGET_OK)
		return (def);

	res = pconf_cast(val, PHIDGETCONFIG_I64, &eval);
	if (res != EPHIDGET_OK)
		return (def);

	return (eval.i64);
}

int64_t CCONV
dictionaryGetI64(PhidgetDictionaryHandle dict, int64_t def, const char *fmt, ...) {
	int64_t res;
	va_list va;

	va_start(va, fmt);
	res = dictionaryGetI64v(dict, def, fmt, va);
	va_end(va);

	return (res);
}

uint64_t CCONV
dictionaryGetU64v(PhidgetDictionaryHandle dict, uint64_t def, const char *fmt, va_list va) {
	char key[PHIDGETCONFIG_PATH_MAX];
	PhidgetReturnCode res;
	pconfvalue_t eval;
	char val[1024];
	size_t n;

	n = mos_vsnprintf(key, sizeof (key), fmt, va);
	if (n >= sizeof (key))
		return (def);

	res = PhidgetDictionary_get(dict, key, val, sizeof (val));
	if (res != EPHIDGET_OK)
		return (def);

	res = pconf_cast(val, PHIDGETCONFIG_U64, &eval);
	if (res != EPHIDGET_OK)
		return (def);

	return (eval.u64);
}

uint64_t CCONV
dictionaryGetU64(PhidgetDictionaryHandle dict, uint64_t def, const char *fmt, ...) {
	uint64_t res;
	va_list va;

	va_start(va, fmt);
	res = dictionaryGetU64v(dict, def, fmt, va);
	va_end(va);

	return (res);
}

const char * CCONV
dictionaryGetStringv(PhidgetDictionaryHandle dict, const char *def, const char *fmt, va_list va) {
	char key[PHIDGETCONFIG_PATH_MAX];
	PhidgetReturnCode res;
	pconfvalue_t eval;
	char val[1024];
	size_t n;

	n = mos_vsnprintf(key, sizeof (key), fmt, va);
	if (n >= sizeof (key))
		return (def);

	res = PhidgetDictionary_get(dict, key, val, sizeof (val));
	if (res != EPHIDGET_OK)
		return (def);

	res = pconf_cast(val, PHIDGETCONFIG_STRING, &eval);
	if (res != EPHIDGET_OK)
		return (def);

	return (eval.str);
}

const char * CCONV
dictionaryGetString(PhidgetDictionaryHandle dict, const char *def, const char *fmt, ...) {
	const char *res;
	va_list va;

	va_start(va, fmt);
	res = dictionaryGetStringv(dict, def, fmt, va);
	va_end(va);

	return (res);
}

int CCONV
dictionaryGetBoolv(PhidgetDictionaryHandle dict, int def, const char *fmt, va_list va) {
	char key[PHIDGETCONFIG_PATH_MAX];
	PhidgetReturnCode res;
	pconfvalue_t eval;
	char val[1024];
	size_t n;

	n = mos_vsnprintf(key, sizeof (key), fmt, va);
	if (n >= sizeof (key))
		return (def);

	res = PhidgetDictionary_get(dict, key, val, sizeof (val));
	if (res != EPHIDGET_OK)
		return (def);

	res = pconf_cast(val, PHIDGETCONFIG_BOOLEAN, &eval);
	if (res != EPHIDGET_OK)
		return (def);

	return (eval.bl);
}

int CCONV
dictionaryGetBool(PhidgetDictionaryHandle dict, int def, const char *fmt, ...) {
	int res;
	va_list va;

	va_start(va, fmt);
	res = dictionaryGetBoolv(dict, def, fmt, va);
	va_end(va);

	return (res);
}