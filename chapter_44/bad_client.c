#include "fifo_seqnum.h"

int main(void) {
    char fifo[CLIENT_FIFO_NAME_LEN];
    struct request req = { getpid(), 1 };

    snprintf(fifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long) req.pid);
    mkfifo(fifo, 0660);  // create a client FIFO, don't open it

    // send request to server
    int fd = open(SERVER_FIFO, O_WRONLY);
    write(fd, &req, sizeof(req));
    close(fd);

    return 0;
}
