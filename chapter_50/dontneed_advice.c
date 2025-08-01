#define _GNU_SOURCE
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "tlpi_hdr.h"

#define FILE_PATH "testfile.txt"
#define FILE_SIZE 4096

int
main()
{
    // create a test file
    int fd = open(FILE_PATH, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1)
        errExit("open");

    // fill with a known pattern (all 'A's)
    char original[FILE_SIZE];
    memset(original, 'A', FILE_SIZE);
    if (write(fd, original, FILE_SIZE) != FILE_SIZE)
        errExit("write");

    // mmap with MAP_PRIVATE
    char *addr = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED)
        errExit("mmap");

    close(fd); // file descriptor no longer needed

    // modify memory
    addr[0] = 'Z';
    addr[1] = 'Z';

    printf("Before madvise: %c %c\n", addr[0], addr[1]);

    // discard changes with MADV_DONTNEED
    if (madvise(addr, FILE_SIZE, MADV_DONTNEED) == -1)
        errExit("madvise");

    // read again
    printf("After madvise:  %c %c\n", addr[0], addr[1]);

    // cleanup
    munmap(addr, FILE_SIZE);
    unlink(FILE_PATH);  // Remove the file

    return 0;
}
