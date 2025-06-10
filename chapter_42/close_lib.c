#include <dlfcn.h>

#include "./dylib.h"
#include "tlpi_hdr.h"


int
main(int argc, char *argv[])
{
    void *liba_handle;
    void *libb_handle;
    void (*func_a)(void);
    void (*func_b)(void);
    
    printf("[main] loading liba\n");
    liba_handle = load_lib_func("./liba.so", "func_a", &func_a);
    if (func_a == NULL)
        errExit("func_a is NULL\n");

    printf("[main] loading libb\n");
    libb_handle = load_lib_func("./libb.so", "func_b", &func_b);
    if (func_b == NULL)
        errExit("func_b is NULL\n");

    (*func_a)(); // this will load libb and make liba dependent on it

    printf("[main] is liba loaded? - %s\n", is_lib_loaded("./liba.so") ? "YES" : "NO");
    printf("[main] is libb loaded? - %s\n", is_lib_loaded("./libb.so") ? "YES" : "NO");

    printf("[main] closing libb\n");
    if (dlclose(libb_handle) == -1)
        errExit("dlclose");

    printf("[main] is liba loaded? - %s\n", is_lib_loaded("./liba.so") ? "YES" : "NO");
    printf("[main] is libb loaded? - %s\n", is_lib_loaded("./libb.so") ? "YES" : "NO");

    printf("[main] closing liba\n");
    if (dlclose(liba_handle) == -1)
        errExit("dlclose");

    printf("[main] is liba loaded? - %s\n", is_lib_loaded("./liba.so") ? "YES" : "NO");
    printf("[main] is libb loaded? - %s\n", is_lib_loaded("./libb.so") ? "YES" : "NO");
}
