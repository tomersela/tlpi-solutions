#define _GNU_SOURCE
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>

#include "tlpi_hdr.h"

#define FILE_SIZE 9500
#define REQ_LEN 6000


static void
segv(int sig, siginfo_t *si, void *ucontext)
{
    printf("\n%s at %p\n", strsignal(sig), si->si_addr);
    _exit(EXIT_SUCCESS);
}


int
main(int argc, char *argv[])
{
    struct sigaction sa;
    long page_size = sysconf(_SC_PAGESIZE);

    if (argc != 2)
        usageErr("%s <file>\n", argv[0]);

    sa.sa_sigaction = segv;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGSEGV, &sa, NULL);

    // create backing file
    int fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1)
        errExit("open");
    if (ftruncate(fd, FILE_SIZE))
        errExit("ftruncate");

    // map (6000 bytes + page size).
    // This will result in the mapping of 3 pages since the kernel rounds up to full page
    size_t guard_len = 3 * page_size; // reserve 3 pages
    void *hole = mmap(NULL, guard_len, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (hole == MAP_FAILED)
        errExit("find hole");

    // unmap the 3 pages
    munmap(hole, guard_len);

    // map the 6000-byte region at the start of that hole
    char *map = mmap(hole, REQ_LEN, PROT_READ | PROT_WRITE,
                     MAP_SHARED | MAP_FIXED_NOREPLACE, fd, 0);
    if (map == MAP_FAILED)
        errExit("MAP_FIXED_NOREPLACE");

    size_t rounded = ((REQ_LEN + page_size - 1) / page_size) * page_size; // 8192
    printf("Actual mapping: %p ... %p (%ld bytes)\n", map, map + rounded - 1, rounded);

    printf("Accessing last byte... this should work\n");
    map[rounded - 1] = 'A';
    printf("Accessing one  byte atfer the mapped range...\n");
    map[rounded] = 'B';     // should trigger SIGSEGV since the next page unmapped
}
