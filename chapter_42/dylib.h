#include <stdbool.h>

#ifndef DYLIB_H
#define DYLIB_H         /* Prevent accidental double inclusion */

void *load_lib_func(const char *lib_name, const char *func_name, void (**func)(void));
bool is_lib_loaded(const char *lib_name);

#endif