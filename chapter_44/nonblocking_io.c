#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "tlpi_hdr.h"

int main(int argc, char *argv[]) {
    int fd;
    const char *fifo = "myfifo";
    ssize_t read_cnt;
    pid_t child_pid;

    if (mkfifo(fifo, 0666) == -1 && errno != EEXIST)
        errExit("mkfifo");

    if ((child_pid = fork()) == -1)
        errExit("fork");

    switch (child_pid) {
        case 0: // child (writer)
            sleep(1);  // ensure reader has opened first

            fd = open(fifo, O_WRONLY);
            if (fd == -1)
                errExit("child open");

            // connect but don't write immediately - reader should see EAGAIN
            sleep(2);

            // now write something - reader should receive >0
            const char *msg = "hello";
            if (write(fd, msg, strlen(msg)) == -1)
                errExit("write");

            close(fd);
            _exit(EXIT_SUCCESS);

        default: // parent (reader)
            fd = open(fifo, O_RDONLY | O_NONBLOCK);
            if (fd == -1)
                errExit("open");

            char buf[100];

            // give child time to connect but not write yet
            sleep(2);

            printf("Trying first read (writer connected but no data yet)...\n");

            switch (read_cnt = read(fd, buf, sizeof(buf))) {
                case -1:
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        printf("read() would block (errno=EAGAIN/EWOULDBLOCK)\n");
                    } else {
                        errExit("read");
                    }
                    break;
                case 0:
                    printf("read() returned 0: no writer or end of stream\n");
                    break;
                default:
                    printf("Read %zd bytes: %.*s\n", read_cnt, (int)read_cnt, buf);
                    break;
            }

            
            sleep(2); // give writer time to send data

            printf("Trying second read (after writer has sent data)...\n");

            switch (read_cnt = read(fd, buf, sizeof(buf))) {
                case -1:
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        printf("read() would block (errno=EAGAIN/EWOULDBLOCK)\n");
                    } else {
                        errExit("read");
                    }
                    break;
                case 0:
                    printf("read() returned 0: no writer or end of stream\n");
                    break;
                default:
                    printf("Read %zd bytes: %.*s\n", read_cnt, (int)read_cnt, buf);
                    break;
            }

            close(fd);
            wait(NULL);  // wait for child to finish

            return 0;
    }
}
