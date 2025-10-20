#define _GNU_SOURCE
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <fcntl.h>
#include <utmpx.h>
#include <paths.h>
#include <time.h>
#if ! defined(__hpux)
// HP-UX 11 doesn't have this header file
#include <sys/select.h>
#endif
#include "pty_fork.h"           // declaration of ptyFork()
#include "tty_functions.h"      // declaration of ttySetRaw()
#include "inet_sockets.h"       // declarations of inet*() socket functions
#include "tlpi_hdr.h"

#define PORT "56789"
#define MAX_SNAME 1000
#define BUF_SIZE 4096

// SIGCHLD handler to reap dead child processes
static void
grimReaper(int sig)
{
    int savedErrno; // save 'errno' in case changed here

    savedErrno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        continue;
    errno = savedErrno;
}

// log client connection with IP address
static void
logClientConnection(struct sockaddr *addr, socklen_t addrlen)
{
    char host[NI_MAXHOST];
    
    if (getnameinfo(addr, addrlen, host, NI_MAXHOST, NULL, 0, 
                    NI_NUMERICHOST) == 0) {
        printf("Client connected from %s\n", host);
    } else {
        printf("Client connected\n");
    }
}

// record login in wtmp (utmp is handled by login(1) itself)
static void
recordLogin(const char *line, const char *host)
{
    struct utmpx ut;

    memset(&ut, 0, sizeof(struct utmpx));
    ut.ut_type = LOGIN_PROCESS;
    strncpy(ut.ut_line, line, sizeof(ut.ut_line));
    strncpy(ut.ut_host, host, sizeof(ut.ut_host));
    ut.ut_pid = getpid();
    ut.ut_tv.tv_sec = time(NULL);

    updwtmpx(_PATH_WTMP, &ut);
}

// record logout in wtmp
static void
recordLogout(const char *line)
{
    struct utmpx ut;

    memset(&ut, 0, sizeof(struct utmpx));
    ut.ut_type = DEAD_PROCESS;
    strncpy(ut.ut_line, line, sizeof(ut.ut_line));
    ut.ut_pid = getpid();
    ut.ut_tv.tv_sec = time(NULL);

    updwtmpx(_PATH_WTMP, &ut);
}

// handle a client request: copy socket input to pty and pty output to socket
static void
handleRequest(int cfd, const char *remoteHost)
{
    char slaveName[MAX_SNAME];
    fd_set inFds;
    int masterFd;
    pid_t childPid;
    char buf[BUF_SIZE];
    ssize_t numRead;
    struct termios tty;
    char *line;

    // set up canonical terminal settings for the pty slave.
    // login will control echo/noecho as needed for password prompt
    if (tcgetattr(STDIN_FILENO, &tty) == -1) {
        // if stdin isn't a terminal, try /dev/tty
        int ttyFd = open("/dev/tty", O_RDONLY);
        if (ttyFd == -1)
            errExit("open /dev/tty");
        if (tcgetattr(ttyFd, &tty) == -1)
            errExit("tcgetattr");
        close(ttyFd);
    }

    // create a child process, with parent and child connected via a
    // pty pair. the child is connected to the pty slave
    childPid = ptyFork(&masterFd, slaveName, MAX_SNAME, &tty, NULL);
    if (childPid == -1)
        errExit("ptyFork");
    
    if (childPid == 0) { // child: execute login on pty slave
        close(cfd); // not needed in child

        // reset SIGCHLD to default before exec'ing login
        signal(SIGCHLD, SIG_DFL);

        execlp("login", "login", (char *) NULL);
        errExit("execlp"); // if we get here, something went wrong
    }

    // record login session start in wtmp
    // extract just the pty name (e.g., "pts/3" from "/dev/pts/3")
    line = slaveName + 5; // skip "/dev/"
    recordLogin(line, remoteHost);

    // loop monitoring socket and pty master for input. if socket
    // is ready for input, relay to pty master. if pty master is ready
    // for input, relay to socket

    for (;;) {
        FD_ZERO(&inFds);
        FD_SET(cfd, &inFds);
        FD_SET(masterFd, &inFds);

        int maxFd = (cfd > masterFd) ? cfd : masterFd;

        if (select(maxFd + 1, &inFds, NULL, NULL, NULL) == -1)
            errExit("select");

        if (FD_ISSET(cfd, &inFds)) { // client --> pty
            numRead = read(cfd, buf, BUF_SIZE);
            if (numRead <= 0) {
                // client disconnected, kill the pty child and clean up
                kill(childPid, SIGHUP);
                recordLogout(line);
                close(cfd);
                close(masterFd);
                exit(EXIT_SUCCESS);
            }

            if (write(masterFd, buf, numRead) != numRead)
                fatal("partial/failed write (masterFd)");
        }

        if (FD_ISSET(masterFd, &inFds)) { // pty --> client
            numRead = read(masterFd, buf, BUF_SIZE);
            if (numRead < 0) {
                // EIO can happen transiently during login's terminal initialization.
                // check if login is still alive before retrying.
                if (errno == EIO) {
                    int status;
                    pid_t result = waitpid(childPid, &status, WNOHANG);
                    if (result == 0) {
                        // child still running, this is transient EIO during init
                        continue; // retry
                    } else {
                        // child exited, this is permanent failure
                        recordLogout(line);
                        close(cfd);
                        close(masterFd);
                        exit(EXIT_SUCCESS);
                    }
                }
                errExit("read masterFd");
            }
            if (numRead == 0) {
                // EOF: login exited
                recordLogout(line);
                close(cfd);
                close(masterFd);
                exit(EXIT_SUCCESS);
            }

            if (write(cfd, buf, numRead) != numRead)
                fatal("partial/failed write (cfd)");
        }
    }
}

int
main(int argc, char *argv[])
{
    int lfd, cfd; // listening and connected sockets
    struct sigaction sa;
    struct sockaddr_storage claddr;
    socklen_t addrlen;
    char host[NI_MAXHOST];

    // establish SIGCHLD handler to reap terminated child processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = grimReaper;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        fprintf(stderr, "Error from sigaction(): %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    lfd = inetListen(PORT, 10, NULL);
    if (lfd == -1) {
        fprintf(stderr, "Could not create server socket (%s)\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %s\n", PORT);

    for (;;) {
        addrlen = sizeof(struct sockaddr_storage);
        cfd = accept(lfd, (struct sockaddr *) &claddr, &addrlen);
        if (cfd == -1) {
            fprintf(stderr, "Failure in accept(): %s\n", strerror(errno));
            continue;
        }

        // get client hostname for wtmp logging
        if (getnameinfo((struct sockaddr *) &claddr, addrlen,
                        host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) != 0) {
            strncpy(host, "unknown", sizeof(host));
        }

        // handle each client request in a new child process
        switch (fork()) {
        case -1:
            fprintf(stderr, "Can't create child (%s)\n", strerror(errno));
            close(cfd); // give up on this client
            break; // may be temporary; try next client

        case 0: // child
            close(lfd); // unneeded copy of listening socket
            logClientConnection((struct sockaddr *) &claddr, addrlen);
            handleRequest(cfd, host);
            _exit(EXIT_SUCCESS);

        default: // parent
            close(cfd); // unneeded copy of connected socket
            break; // loop to accept next connection
        }
    }
}
