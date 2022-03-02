#include <dirent.h>
#include <dlfcn.h>

#include "phidget22extra/projectsupport.h"
#include "phidget22extra/phidgetloghelpers.h"

typedef void (CCONV *moduleEntry_t)(PhidgetModuleLoadArgsHandle);

struct entrycall {
	moduleEntry_t entry;
	PhidgetModuleLoadArgsHandle args;
};

static MOS_TASK_RESULT
runModule(void *args) {
	struct entrycall *ec;

	ec = args;
	ec->entry(ec->args);
	mos_free(ec, sizeof (*ec));

	MOS_TASK_EXIT(0);
}

#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic" // disable pedantic for dlsym()
#endif
static PhidgetReturnCode
loadModule(const char *modules, struct dirent *dent, PhidgetModuleLoadArgsHandle args) {
	char module[MOS_PATH_MAX];
	struct entrycall *ec;
	mos_task_t task;
	void *mod;

	mos_snprintf(module, sizeof (module), "%s/%s", modules, dent->d_name);
	suploginfo("load module: %s", module);

	mod = dlopen(module, RTLD_NOW | RTLD_LOCAL);
	if (mod == NULL) {
		suplogerr("failed to load module [%s]: %s", module, dlerror());
		return (EPHIDGET_UNEXPECTED);
	}

	ec = mos_malloc(sizeof (*ec));
	ec->args = args;
	ec->entry = dlsym(mod, "phidgetModuleEntry");
	if (ec->entry != NULL) {
		if (mos_task_create(&task, runModule, ec) != 0)
			mos_free(ec, sizeof (*ec));
	}

	return (EPHIDGET_OK);
}
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)
#pragma GCC diagnostic pop
#endif

PhidgetReturnCode
loadModules(const char *modules, PhidgetModuleLoadArgsHandle args) {
	struct dirent *dent;
	DIR *dir;

	dir = opendir(modules);
	if (dir == NULL)
		return (EPHIDGET_NOENT);

	while ((dent = readdir(dir)) != NULL) {
		if (!mos_endswith(dent->d_name, ".pm"))
			continue;
		loadModule(modules, dent, args);
	}

	closedir(dir);
	return (EPHIDGET_OK);
}
