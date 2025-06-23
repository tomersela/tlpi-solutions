#include <sys/stat.h>
#include <fcntl.h>

#include "tlpi_hdr.h"

int main(int argc, char *argv[]) {
    int fd;
    const char *fifo = "myfifo";

    printf("Opening FIFO for writing with O_NONBLOCK...\n");

    fd = open(fifo, O_WRONLY | O_NONBLOCK);
    if (fd == -1) {
        if (errno == ENXIO) {
            printf("No reader on the FIFO (errno=ENXIO)\n");
            exit(EXIT_SUCCESS);
        } else {
            errExit("open");
        }
    }

    printf("Opened FIFO for writing (nonblocking mode) successfully.\n");

    close(fd);
    return 0;
}
