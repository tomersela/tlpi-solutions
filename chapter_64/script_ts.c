/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2025.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Listing 64-3 */

/* script.c

   A simple version of script(1).
*/
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <termios.h>
#include <time.h>
#if ! defined(__hpux)
/* HP-UX 11 doesn't have this header file */
#include <sys/select.h>
#endif
#include "pty_fork.h"           /* Declaration of ptyFork() */
#include "tty_functions.h"      /* Declaration of ttySetRaw() */
#include "tlpi_hdr.h"

#define BUF_SIZE 256
#define MAX_SNAME 1000
#define TIMED_BUF_SIZE 1024

struct termios ttyOrig;

static void             /* Reset terminal mode on program exit */
ttyReset(void)
{
    if (tcsetattr(STDIN_FILENO, TCSANOW, &ttyOrig) == -1)
        errExit("tcsetattr");
}

int
main(int argc, char *argv[])
{
    char slaveName[MAX_SNAME];
    char *shell;
    char timedFilename[MAX_SNAME];
    int masterFd, scriptFd, timedFd;
    struct winsize ws;
    struct timespec startTime, currentTime;
    fd_set inFds;
    char buf[BUF_SIZE];
    char timedBuf[TIMED_BUF_SIZE];
    char lineBuf[BUF_SIZE];
    int lineBufLen = 0;
    ssize_t numRead;
    pid_t childPid;

    /* Retrieve the attributes of terminal on which we are started */

    if (tcgetattr(STDIN_FILENO, &ttyOrig) == -1)
        errExit("tcgetattr");
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0)
        errExit("ioctl-TIOCGWINSZ");

    /* Create a child process, with parent and child connected via a
       pty pair. The child is connected to the pty slave and its terminal
       attributes are set to be the same as those retrieved above. */

    childPid = ptyFork(&masterFd, slaveName, MAX_SNAME, &ttyOrig, &ws);
    if (childPid == -1)
        errExit("ptyFork");

    if (childPid == 0) {        /* Child: execute a shell on pty slave */

        /* If the SHELL variable is set, use its value to determine
           the shell execed in child. Otherwise use /bin/sh. */

        shell = getenv("SHELL");
        if (shell == NULL || *shell == '\0')
            shell = "/bin/sh";

        execlp(shell, shell, (char *) NULL);
        errExit("execlp");      /* If we get here, something went wrong */
    }

    /* Parent: relay data between terminal and pty master */

    scriptFd = open((argc > 1) ? argv[1] : "typescript",
                        O_WRONLY | O_CREAT | O_TRUNC,
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                                S_IROTH | S_IWOTH);
    if (scriptFd == -1)
        errExit("open typescript");

    snprintf(timedFilename, MAX_SNAME, "%s.timed", (argc > 1) ? argv[1] : "typescript");
    timedFd = open(timedFilename, O_WRONLY | O_CREAT | O_TRUNC,
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                                S_IROTH | S_IWOTH);
    if (timedFd == -1)
        errExit("open typescript.timed");

    // record session start time
    if (clock_gettime(CLOCK_MONOTONIC, &startTime) == -1)
        errExit("clock_gettime");

    /* Place terminal in raw mode so that we can pass all terminal
     input to the pseudoterminal master untouched */

    ttySetRaw(STDIN_FILENO, &ttyOrig);

    if (atexit(ttyReset) != 0)
        errExit("atexit");

    /* Loop monitoring terminal and pty master for input. If the
       terminal is ready for input, then read some bytes and write
       them to the pty master. If the pty master is ready for input,
       then read some bytes and write them to the terminal. */

    for (;;) {
        FD_ZERO(&inFds);
        FD_SET(STDIN_FILENO, &inFds);
        FD_SET(masterFd, &inFds);

        if (select(masterFd + 1, &inFds, NULL, NULL, NULL) == -1)
            errExit("select");

        if (FD_ISSET(STDIN_FILENO, &inFds)) {   /* stdin --> pty */
            numRead = read(STDIN_FILENO, buf, BUF_SIZE);
            if (numRead <= 0)
                exit(EXIT_SUCCESS);

            if (write(masterFd, buf, numRead) != numRead)
                fatal("partial/failed write (masterFd)");
        }

        if (FD_ISSET(masterFd, &inFds)) {      /* pty --> stdout+file */
            numRead = read(masterFd, buf, BUF_SIZE);
            if (numRead <= 0)
                exit(EXIT_SUCCESS);

            if (write(STDOUT_FILENO, buf, numRead) != numRead)
                fatal("partial/failed write (STDOUT_FILENO)");
            if (write(scriptFd, buf, numRead) != numRead)
                fatal("partial/failed write (scriptFd)");

            // accumulate into line buffer and write timestamped lines
            for (ssize_t i = 0; i < numRead; i++) {
                lineBuf[lineBufLen++] = buf[i];

                // write line when we hit newline or buffer is full
                if (buf[i] == '\n' || lineBufLen >= BUF_SIZE - 1) {
                    if (clock_gettime(CLOCK_MONOTONIC, &currentTime) == -1)
                        errExit("clock_gettime");

                    long long elapsedMs = (currentTime.tv_sec - startTime.tv_sec) * 1000LL +
                                          (currentTime.tv_nsec - startTime.tv_nsec) / 1000000;

                    int timedLen = snprintf(timedBuf, TIMED_BUF_SIZE, "%lld ", elapsedMs);

                    // escape newlines, carriage returns, and backslashes
                    for (int j = 0; j < lineBufLen && timedLen < TIMED_BUF_SIZE - 2; j++) {
                        if (lineBuf[j] == '\n') {
                            timedBuf[timedLen++] = '\\';
                            timedBuf[timedLen++] = 'n';
                        } else if (lineBuf[j] == '\r') {
                            timedBuf[timedLen++] = '\\';
                            timedBuf[timedLen++] = 'r';
                        } else if (lineBuf[j] == '\\') {
                            timedBuf[timedLen++] = '\\';
                            timedBuf[timedLen++] = '\\';
                        } else {
                            timedBuf[timedLen++] = lineBuf[j];
                        }
                    }
                    timedBuf[timedLen++] = '\n';

                    if (write(timedFd, timedBuf, timedLen) != timedLen)
                        fatal("partial/failed write (timedFd)");

                    lineBufLen = 0;  // reset line buffer
                }
            }
        }
    }
}
