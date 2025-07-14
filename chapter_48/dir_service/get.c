#include "tlpi_hdr.h"
#include "dir_service.h"


int
main(int argc, char *argv[])
{
    char value[VALUE_LEN + 1] = {0};

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <name>\n", argv[0]);
        return 1;
    }

    if (dir_get(argv[1], value) == 0)
        printf("%s\n", value);
    else
        printf("Not found\n");

    return 0;
}
