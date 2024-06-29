#include <stdio.h>
#include <fcntl.h>

#include <sys/stat.h>
#include "tlpi_hdr.h"


int dedup(int fd);
int dedup2(int oldfd, int newfd);

int
main (int argc, char* argv[]) {

    int newfd;
    if ((newfd = dedup(STDOUT_FILENO)) == -1)
        errExit("dedup");

    dprintf(newfd, "Hello! (fd = %d)\n", newfd);

    int newfd2;
    
    if ((newfd2 = dedup2(newfd, 5)) == -1)
        errExit("dedup2");

    dprintf(newfd2, "Hello! (fd = %d)\n", newfd2);
}

int dedup(int fd) {
    return fcntl(fd, F_DUPFD, 0);
}

int dedup2(int oldfd, int newfd) {
    if (oldfd == newfd) {
        return newfd;
    }
    close(newfd); // ignore errors
    return fcntl(oldfd, F_DUPFD, newfd);
}
