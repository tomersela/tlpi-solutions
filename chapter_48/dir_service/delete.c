#include "tlpi_hdr.h"
#include "dir_service.h"


int
main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <name>\n", argv[0]);
        return 1;
    }

    if (dir_delete(argv[1]) == 0)
        printf("Deleted\n");
    else
        printf("Not found\n");

    return 0;
}
