#ifndef _PHIDGET_DICT_HELPERS_H_
#define _PHIDGET_DICT_HELPERS_H_

#include "phidget22extra_int.h"

PHIDAPI PhidgetReturnCode CCONV dictionarySet(PhidgetDictionaryHandle, const char *, const char *, ...);
PHIDAPI const char * CCONV dictionaryGet(PhidgetDictionaryHandle, char *, size_t, const char *, const char *, ...);

PHIDAPI double CCONV dictionaryGetDoublev(PhidgetDictionaryHandle, double, const char *, va_list);
PHIDAPI double CCONV dictionaryGetDouble(PhidgetDictionaryHandle, double, const char *, ...);

PHIDAPI uint64_t CCONV dictionaryGetU64v(PhidgetDictionaryHandle, uint64_t, const char *, va_list);
PHIDAPI uint64_t CCONV dictionaryGetU64(PhidgetDictionaryHandle, uint64_t, const char *, ...);

PHIDAPI int64_t CCONV dictionaryGetI64v(PhidgetDictionaryHandle, int64_t, const char *, va_list);
PHIDAPI int64_t CCONV dictionaryGetI64(PhidgetDictionaryHandle, int64_t, const char *, ...);

PHIDAPI const char * CCONV dictionaryGetStringv(PhidgetDictionaryHandle, const char *, const char *, va_list);
PHIDAPI const char * CCONV dictionaryGetString(PhidgetDictionaryHandle, const char *, const char *, ...);

PHIDAPI int CCONV dictionaryGetBoolv(PhidgetDictionaryHandle, int, const char *, va_list);
PHIDAPI int CCONV dictionaryGetBool(PhidgetDictionaryHandle, int, const char *, ...);

#endif /* _PHIDGET_DICT_HELPERS_H_ */