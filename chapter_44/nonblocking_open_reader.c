#include <sys/stat.h>
#include <fcntl.h>

#include "tlpi_hdr.h"

int main(int argc, char *argv[]) {
    int fd;
    const char *fifo = "myfifo";
    ssize_t read_cnt;
    char buf[100];

    if (mkfifo(fifo, 0666) == -1 && errno != EEXIST)
        errExit("mkfifo");

    printf("Opening FIFO for reading with O_NONBLOCK...\n");
    if ((fd = open(fifo, O_RDONLY | O_NONBLOCK)) == -1)
        errExit("open");

    printf("Opened FIFO for reading (nonblocking mode) successfully.\n");

    switch (read_cnt = read(fd, buf, sizeof(buf)))
    {
        case -1:
            errExit("read");
        case 0:
            printf("No data to read (read() returned 0)\n");
            break;
        default:
            printf("Read %zd bytes\n", read_cnt);
            break;
    }

    close(fd);
    return 0;
}
