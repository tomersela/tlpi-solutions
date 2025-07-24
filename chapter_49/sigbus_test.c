#define _GNU_SOURCE
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include "tlpi_hdr.h"

#define FILE_SIZE 2200

static void
handler(int sig)
{
    printf("\nCaught signal: %s\n", strsignal(sig));
    exit(EXIT_FAILURE);
}


int
main(int argc, char *argv[])
{
    char *map;
    int fd;
    long page_size;
    size_t map_size;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file-path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    signal(SIGBUS, handler);

    page_size = sysconf(_SC_PAGESIZE);
    map_size = 2 * page_size;

    fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1)
        errExit("open");

    if (ftruncate(fd, FILE_SIZE) == -1)
        errExit("ftruncate");

    map = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED)
        errExit("mmap");

    printf("Page size: %ld\n", page_size);
    printf("Mapped size: %zu bytes (file is %d bytes)\n", map_size, FILE_SIZE);

    printf("Access within file range...\n");
    map[100] = 'A';

    printf("Access beyond file range but still in the first page... this should work\n");
    map[page_size - 1] = 'A';

    printf("Access beyond file range but in the second page...\n");
    map[page_size] = 'A';

    munmap(map, map_size);
    close(fd);
    return 0;
}
