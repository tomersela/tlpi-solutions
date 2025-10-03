#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <signal.h>

#include "tlpi_hdr.h"
#include "rdwrn.h"

#define PORT_NUM "50000"
#define BACKLOG 50
#define BUF_SIZE 4096

static void
handleRequest(int cfd)
{
    char buf[BUF_SIZE];
    ssize_t numRead, totalRead;

    // read command from client until EOF
    totalRead = 0;
    while ((numRead = read(cfd, buf + totalRead, BUF_SIZE - totalRead - 1)) > 0) {
        totalRead += numRead;
        if (totalRead >= BUF_SIZE - 1) {
            // command too long, send error message
            const char *error_msg = "Error: Command too long (max 4095 characters)\n";
            write(cfd, error_msg, strlen(error_msg));
            close(cfd);
            _exit(1);
        }
    }

    if (numRead == -1)
        err_exit("read");

    buf[totalRead] = '\0';  // null-terminate command string

    // duplicate socket file descriptor to stdout and stderr
    if (dup2(cfd, STDOUT_FILENO) == -1)
        err_exit("dup2 stdout");

    if (dup2(cfd, STDERR_FILENO) == -1)
        err_exit("dup2 stderr");

    // close original socket fd since we have duplicates
    if (close(cfd) == -1)
        err_exit("close");

    // execute command using shell
    execl("/bin/sh", "sh", "-c", buf, (char *) NULL);

    // if we get here, exec failed
    _exit(127);
}

static void
sigchldHandler(int sig)
{
    int savedErrno = errno;

    // reap all available zombie children
    while (waitpid(-1, NULL, WNOHANG) > 0)
        continue;

    errno = savedErrno;
}

int
main(int argc, char *argv[])
{
    int lfd, cfd;
    struct addrinfo hints, *result, *rp;
    struct sigaction sa;

    // ignore SIGPIPE signal
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        errExit("signal");

    // install SIGCHLD handler to reap zombie children
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = sigchldHandler;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
        errExit("sigaction");

    // create listening socket
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;

    if (getaddrinfo(NULL, PORT_NUM, &hints, &result) != 0)
        errExit("getaddrinfo");

    // try each address until we successfully bind one
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        lfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (lfd == -1)
            continue;

        // enable address reuse
        int optval = 1;
        if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
            close(lfd);
            continue;
        }

        if (bind(lfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;  // success

        close(lfd);
    }

    if (rp == NULL)
        fatal("Could not bind to any address");

    freeaddrinfo(result);

    if (listen(lfd, BACKLOG) == -1)
        errExit("listen");

    printf("Shell server listening on port %s\n", PORT_NUM);

    // main server loop
    for (;;) {
        cfd = accept(lfd, NULL, NULL);
        if (cfd == -1) {
            if (errno == EINTR)  // interrupted by signal
                continue;
            errExit("accept");
        }

        // handle each client in a separate child process
        switch (fork()) {
        case -1:
            errMsg("fork");
            close(cfd);
            break;

        case 0: // child
            close(lfd);  // child doesn't need listening socket
            handleRequest(cfd);
            // handleRequest() calls exec, so we never return here
            break;

        default: // parent
            close(cfd);  // parent doesn't need connected socket
            break;
        }
    }
}
