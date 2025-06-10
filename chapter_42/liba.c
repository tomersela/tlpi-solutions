#include <dlfcn.h>

#include "./dylib.h"
#include "tlpi_hdr.h"

void func_a(void);
void func_b(void);

void __attribute__ ((constructor)) on_load(void);
void __attribute__ ((destructor)) on_unload(void);


static void *libb_handle;

void
func_a()
{
    void (*func_b)(void);
    
    printf("[liba] Hello from func_a\n");

    printf("[liba] loading libb\n");
    libb_handle = load_lib_func("./libb.so", "func_b", &func_b);
    if (func_b == NULL)
        errExit("[liba] func_b is NULL\n");

    (*func_b)();
}


void __attribute__ ((constructor))
on_load(void)
{
    printf("[liba] LOADED liba\n");
}


void __attribute__ ((destructor))
on_unload(void)
{
    printf("[liba] UNLOADED liba\n");
    if (libb_handle != NULL)
        dlclose(libb_handle);
}
