#define _LARGEFILE64_SOURCE
#define _FILE_OFFSET_BITS 64
#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int fd;

    if (argc <= 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s num_bytes pathname\n", argv[0]);

    char *filename = argv[1];
    long num_bytes = atoll(argv[2]);
    bool x_param = argc >= 4 && !strcmp(argv[3], "x");
    int maybe_o_append_flag = x_param ? 0 : O_APPEND;


    fd = open(filename, O_RDWR | O_CREAT | maybe_o_append_flag, S_IRUSR | S_IWUSR);
    if (fd == -1)
        errExit("open");

    for (long l = 0; l < num_bytes; l++) {
        if (x_param) {
            if (lseek(fd, 0, SEEK_END) == -1)
                errExit("lseek");
        }
        if (write(fd, "x", 1) == -1) errExit("write");
    }

    if (close(fd) == -1) errExit("close");

    exit(EXIT_SUCCESS);
}
