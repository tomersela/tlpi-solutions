#include <stdbool.h>
#include <dlfcn.h>


#include "tlpi_hdr.h"
#include "./dylib.h"

void *
load_lib_func(const char *lib_name, const char *func_name, void (**func)(void))
{
    void *lib_handle;
    const char *err;

    lib_handle = dlopen(lib_name, RTLD_LAZY);
    if (lib_handle == NULL)
        fatal("dlopen: %s", dlerror());

    (void) dlerror();
    *(void **) (func) = dlsym(lib_handle, func_name);
    err = dlerror();
    if (err != NULL)
        fatal("dlsym: %s", err);

    return lib_handle;
}


bool
is_lib_loaded(const char *lib_name)
{
    void *handle = dlopen(lib_name, RTLD_NOLOAD | RTLD_LAZY);
    if (handle) {
        dlclose(handle); // Important: don't increase refcount
        return 1;
    } else {
        return 0;
    }
}
