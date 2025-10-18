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
#include <signal.h>
#if ! defined(__hpux)
/* HP-UX 11 doesn't have this header file */
#include <sys/select.h>
#endif
#include "pty_fork.h"           /* Declaration of ptyFork() */
#include "tty_functions.h"      /* Declaration of ttySetRaw() */
#include "tlpi_hdr.h"

#define BUF_SIZE 256
#define MAX_SNAME 1000

struct termios ttyOrig;
int scriptFd;
int sigwinchPipeWrite;

static void             /* Reset terminal mode on program exit */
ttyReset(void)
{
    if (tcsetattr(STDIN_FILENO, TCSANOW, &ttyOrig) == -1)
        errExit("tcsetattr");
}

static void
formatTimestamp(char *buf, size_t bufSize, const char *prefix)
{
    time_t t;
    struct tm *tm;
    
    t = time(NULL);
    tm = localtime(&t);
    snprintf(buf, bufSize, "%s", prefix);
    strftime(buf + strlen(buf), bufSize - strlen(buf),
             "%a %d %b %Y %I:%M:%S %p %Z\n", tm);
}

static void
scriptClose(void)
{
    char buf[200];
    
    formatTimestamp(buf, sizeof(buf), "Script done on ");
    write(scriptFd, buf, strlen(buf));
    close(scriptFd);
}

static void
sigwinchHandler(int sig)
{
    char c = 0;
    write(sigwinchPipeWrite, &c, 1);
}

int
main(int argc, char *argv[])
{
    char slaveName[MAX_SNAME];
    char *shell;
    int masterFd, sigwinchPipe[2];
    int flags;
    struct winsize ws;
    struct sigaction sa;
    fd_set inFds;
    char buf[BUF_SIZE];
    char timestampBuf[200];
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

    formatTimestamp(timestampBuf, sizeof(timestampBuf), "Script started on ");
    write(scriptFd, timestampBuf, strlen(timestampBuf));

    if (atexit(scriptClose) != 0)
        errExit("atexit");

    // create pipe for SIGWINCH handling
    if (pipe(sigwinchPipe) == -1)
        errExit("pipe");

    // set both pipe ends to non-blocking
    flags = fcntl(sigwinchPipe[0], F_GETFL);
    if (flags == -1)
        errExit("fcntl-F_GETFL");
    if (fcntl(sigwinchPipe[0], F_SETFL, flags | O_NONBLOCK) == -1)
        errExit("fcntl-F_SETFL");

    flags = fcntl(sigwinchPipe[1], F_GETFL);
    if (flags == -1)
        errExit("fcntl-F_GETFL");
    if (fcntl(sigwinchPipe[1], F_SETFL, flags | O_NONBLOCK) == -1)
        errExit("fcntl-F_SETFL");

    sigwinchPipeWrite = sigwinchPipe[1];

    // install SIGWINCH handler
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sigwinchHandler;
    if (sigaction(SIGWINCH, &sa, NULL) == -1)
        errExit("sigaction");

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
        FD_SET(sigwinchPipe[0], &inFds); // subscribe to window size changes

        int maxFd = (masterFd > sigwinchPipe[0]) ? masterFd : sigwinchPipe[0];
        if (select(maxFd + 1, &inFds, NULL, NULL, NULL) == -1) {
            if (errno == EINTR)
                continue;
            errExit("select");
        }

        if (FD_ISSET(sigwinchPipe[0], &inFds)) {   /* SIGWINCH received */
            char c;
            while (read(sigwinchPipe[0], &c, 1) > 0);  // drain pipe

            // forward current window size to pty
            if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) >= 0)
                ioctl(masterFd, TIOCSWINSZ, &ws);
        }

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
        }
    }
}
