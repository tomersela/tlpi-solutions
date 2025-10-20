#include <sys/select.h>
#include "inet_sockets.h"
#include "tty_functions.h"
#include "tlpi_hdr.h"

#define BUF_SIZE 4096

int
main(int argc, char *argv[])
{
    char buf[BUF_SIZE];
    fd_set inFds;
    ssize_t numRead;
    int sfd;
    struct termios ttyOrig;

    if (argc != 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s host port\n", argv[0]);

    // connect to server
    sfd = inetConnect(argv[1], argv[2], SOCK_STREAM);
    if (sfd == -1)
        errExit("inetConnect");

    // put terminal in raw mode so we don't echo locally
    if (ttySetRaw(STDIN_FILENO, &ttyOrig) == -1)
        errExit("ttySetRaw");

    // relay between stdin/stdout and socket
    for (;;) {
        FD_ZERO(&inFds);
        FD_SET(STDIN_FILENO, &inFds);
        FD_SET(sfd, &inFds);

        int maxFd = (STDIN_FILENO > sfd) ? STDIN_FILENO : sfd;

        if (select(maxFd + 1, &inFds, NULL, NULL, NULL) == -1)
            errExit("select");

        if (FD_ISSET(STDIN_FILENO, &inFds)) { // stdin --> socket
            numRead = read(STDIN_FILENO, buf, BUF_SIZE);
            if (numRead <= 0)
                break;

            // check for escape characters (Ctrl+C or Ctrl+D) to force disconnect
            if (numRead == 1 && (buf[0] == 3 || buf[0] == 4)) { // 3=Ctrl+C, 4=Ctrl+D
                break;
            }

            if (write(sfd, buf, numRead) != numRead)
                fatal("partial/failed write (sfd)");
        }

        if (FD_ISSET(sfd, &inFds)) { // socket --> stdout
            numRead = read(sfd, buf, BUF_SIZE);
            if (numRead <= 0)
                break;

            if (write(STDOUT_FILENO, buf, numRead) != numRead)
                fatal("partial/failed write (stdout)");
        }
    }

    // restore terminal settings
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &ttyOrig) == -1)
        errExit("tcsetattr");

    exit(EXIT_SUCCESS);
}
