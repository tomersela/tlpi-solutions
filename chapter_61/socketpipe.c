#include <signal.h>
#include <sys/socket.h>

#include "tlpi_hdr.h"


int socketpipe(int pipefd[2]);


int
socketpipe(int pipefd[2])
{
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pipefd) == -1)
        return -1;

    printf("socketpair created: %d, %d\n", pipefd[0], pipefd[1]);

    if (shutdown(pipefd[0], SHUT_WR) == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }
    printf("shutdown write on %d\n", pipefd[0]);

    if(shutdown(pipefd[1], SHUT_RD) == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }
    printf("shutdown read on %d\n", pipefd[1]);

    return 0;
}


int
main(int argc, char *argv[])
{
    int sp[2];
    char buf[20];
    ssize_t n;

    signal(SIGPIPE, SIG_IGN);  // ignore SIGPIPE

    if (socketpipe(sp) == -1)
        errExit("socketpipe");

    write(sp[1], "test", 4);
    n = read(sp[0], buf, sizeof(buf) - 1);
    buf[n] = '\0';
    printf("data: %s\n", buf);

    // verify unidirectional
    ssize_t ret = write(sp[0], "x", 1);
    printf("write to read-end returned: %zd\n", ret);
    if (ret == -1)
        printf("  error: %s\n", strerror(errno));

    printf("attempting read from write-end...\n");
    ret = read(sp[1], buf, 1);
    printf("read from write-end returned: %zd\n", ret);
    if (ret == -1)
        printf("  error: %s\n", strerror(errno));

    close(sp[0]);
    close(sp[1]);
    exit(EXIT_SUCCESS);
}
