#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

int
main (int argc, char *argv[]) {
    int fd;
    off_t start_off = 0;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pathname\n", argv[0]);

    fd = open(argv[1], O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if (fd == -1)
        errExit("open");

    if (lseek(fd, start_off, SEEK_SET) == -1)
        errExit("lseek");
    

    if (write(fd, "test", 4) == -1)
        errExit("write");
}