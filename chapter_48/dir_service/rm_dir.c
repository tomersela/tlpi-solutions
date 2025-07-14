#include "tlpi_hdr.h"
#include "dir_service.h"


int main(void) {
    dir_cleanup();
    printf("Directory service removed (shared memory + semaphore removed).\n");
    return 0;
}
