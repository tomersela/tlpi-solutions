#define _GNU_SOURCE
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/resource.h>

#include "tlpi_hdr.h"


int
main(int argc, char *argv[])
{
    const size_t alloc_size = 128 * 1024;
    struct rlimit rl;

    // set the memlock limit to 64KB
    rl.rlim_cur = rl.rlim_max = 64 * 1024;
    if (setrlimit(RLIMIT_MEMLOCK, &rl) == -1)
        errExit("setrlimit");

    // allocate 128KB of memory
    void *addr = malloc(alloc_size);
    if (!addr) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // touch the memory to ensure it's mapped
    memset(addr, 0, alloc_size);

    // try to lock all mapped memory (more than the allowed limit)
    if (mlock(addr, alloc_size) == -1) {
        printf("mlock failed because we passed the defined limit\n");
        printf("%s (errno = %d)\n", strerror(errno), errno);
        munlock(addr, alloc_size);
        free(addr);
        exit(EXIT_SUCCESS);
    }

    printf("Unexpected! we shouldn't get this far.");
    return EXIT_FAILURE;
}
