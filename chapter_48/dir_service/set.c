#include "dir_service.h"
#include <stdio.h>


int
main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <name> <value>\n", argv[0]);
        return 1;
    }

    if (dir_set(argv[1], argv[2]) >= 0)
        printf("Set OK\n");
    else
        printf("Set failed\n");

    return 0;
}
