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
#include <signal.h>
#include <sys/wait.h>
#include "pty_fork.h"           /* Declaration of ptyFork() */
#include "tty_functions.h"      /* Declaration of ttySetRaw() */
#include "tlpi_hdr.h"

#define BUF_SIZE 256
#define MAX_SNAME 1000

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
    int masterFd, scriptFd;
    struct winsize ws;
    char buf[BUF_SIZE];
    ssize_t numRead;
    // three children: shell (shellPid), stdin->pty handler (readerPid), pty->stdout handler (writerPid)
    pid_t shellPid, readerPid, writerPid, exitedPid;

    /* Retrieve the attributes of terminal on which we are started */

    if (tcgetattr(STDIN_FILENO, &ttyOrig) == -1)
        errExit("tcgetattr");
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) < 0)
        errExit("ioctl-TIOCGWINSZ");

    /* Create a child process, with parent and child connected via a
       pty pair. The child is connected to the pty slave and its terminal
       attributes are set to be the same as those retrieved above. */

    shellPid = ptyFork(&masterFd, slaveName, MAX_SNAME, &ttyOrig, &ws);
    if (shellPid == -1)
        errExit("ptyFork");

    if (shellPid == 0) {        /* Child: execute a shell on pty slave */

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

    /* Place terminal in raw mode so that we can pass all terminal
     input to the pseudoterminal master untouched */

    ttySetRaw(STDIN_FILENO, &ttyOrig);

    if (atexit(ttyReset) != 0)
        errExit("atexit");

    /* Create two handler processes: one relays stdin to pty master,
       the other relays pty master to stdout and typescript file */

    // fork first child: stdin -> masterFd
    readerPid = fork();
    if (readerPid == -1)
        errExit("fork");

    if (readerPid == 0) {       // reader child: read from stdin, write to pty
        close(scriptFd);        // don't need scriptFd

        for (;;) {
            numRead = read(STDIN_FILENO, buf, BUF_SIZE);
            if (numRead <= 0)
                exit(EXIT_SUCCESS);

            if (write(masterFd, buf, numRead) != numRead)
                fatal("partial/failed write (masterFd)");
        }
    }

    // fork second child: masterFd -> stdout + scriptFd
    writerPid = fork();
    if (writerPid == -1)
        errExit("fork");

    if (writerPid == 0) {       // writer child: read from pty, write to stdout and file
        for (;;) {
            numRead = read(masterFd, buf, BUF_SIZE);
            if (numRead <= 0)
                exit(EXIT_SUCCESS);

            if (write(STDOUT_FILENO, buf, numRead) != numRead)
                fatal("partial/failed write (STDOUT_FILENO)");
            if (write(scriptFd, buf, numRead) != numRead)
                fatal("partial/failed write (scriptFd)");
        }
    }

    // parent: orchestrate termination when a handler exits
    close(masterFd);            // parent doesn't need these
    close(scriptFd);

    // wait for first child to exit
    exitedPid = wait(NULL);
    if (exitedPid == -1)
        errExit("wait");

    // if shell exited first, wait for a handler to exit
    while (exitedPid == shellPid) {
        exitedPid = wait(NULL);
        if (exitedPid == -1)
            errExit("wait");
    }

    // now exitedPid is definitely a handler - handle termination
    if (exitedPid == writerPid) {
        // writer exited (shell closed pty), kill reader (won't exit on its own)
        kill(readerPid, SIGTERM);
        wait(NULL);  // killed reader
        wait(NULL);  // shell (if not already reaped above)
    } else {
        // reader exited (stdin closed), let writer drain pty output naturally
        wait(NULL);  // writer will finish draining
        wait(NULL);  // shell
    }

    exit(EXIT_SUCCESS);
}
