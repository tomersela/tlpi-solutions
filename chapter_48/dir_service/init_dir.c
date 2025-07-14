#include "tlpi_hdr.h"
#include "dir_service.h"


int
main()
{
    if (dir_init() == 0) {
        printf("Directory service initialized.\n");
    } else {
        printf("Initialization failed.\n");
    }
    return 0;
}
