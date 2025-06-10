#include "tlpi_hdr.h"


void func_b(void);

void __attribute__ ((constructor)) on_load(void);
void __attribute__ ((destructor)) on_unload(void);

void
func_b()
{
    printf("[libb] Hello from func_b\n");
}


void __attribute__ ((constructor)) on_load(void)
{
    printf("[libb] LOADED libb\n");
}


void __attribute__ ((destructor)) on_unload(void)
{
    printf("[libb] UNLOADED libb\n");
}
