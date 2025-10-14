/* Exercise 63-3 */

/* msgq_select.c

   Demonstrates using select() to monitor both terminal input and a System V 
   message queue. A child process copies messages from the queue to a pipe,
   allowing the parent to use select() on both stdin and the pipe.

   Usage: ./msgq_select msqid
*/
#include <sys/msg.h>
#include <sys/select.h>
#include "tlpi_hdr.h"

#define MAX_MSG_SIZE 1024

struct mbuf {
    long mtype;
    char mtext[MAX_MSG_SIZE];
};

int
main(int argc, char *argv[])
{
    int msgid, pipefd[2];
    pid_t childPid;
    fd_set readfds;
    char buf[MAX_MSG_SIZE];
    ssize_t numRead;
    struct mbuf msg;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s msqid\n", argv[0]);

    msgid = getInt(argv[1], 0, "msqid");

    // create pipe for child-to-parent communication
    if (pipe(pipefd) == -1)
        errExit("pipe");

    switch (childPid = fork()) {
    case -1:
        errExit("fork");

    case 0:    // child: read from message queue, write to pipe
        close(pipefd[0]);  // close read end

        for (;;) {
            if (msgrcv(msgid, &msg, MAX_MSG_SIZE, 0, 0) == -1)
                errExit("msgrcv");

            // write message to pipe
            if (write(pipefd[1], msg.mtext, strlen(msg.mtext) + 1) == -1)
                errExit("write to pipe");
        }

    default:   // parent: use select() to monitor stdin and pipe
        close(pipefd[1]);  // close write end

        for (;;) {
            FD_ZERO(&readfds);
            FD_SET(STDIN_FILENO, &readfds);
            FD_SET(pipefd[0], &readfds);

            if (select(pipefd[0] + 1, &readfds, NULL, NULL, NULL) == -1)
                errExit("select");

            // check terminal input
            if (FD_ISSET(STDIN_FILENO, &readfds)) {
                numRead = read(STDIN_FILENO, buf, MAX_MSG_SIZE - 1);
                if (numRead == -1)
                    errExit("read stdin");
                if (numRead == 0)
                    break;  // EOF

                buf[numRead] = '\0';
                printf("Terminal: %s", buf);
            }

            // check message queue (via pipe)
            if (FD_ISSET(pipefd[0], &readfds)) {
                numRead = read(pipefd[0], buf, MAX_MSG_SIZE);
                if (numRead == -1)
                    errExit("read pipe");
                if (numRead == 0)
                    break;  // child closed pipe

                printf("Message queue: %s\n", buf);
            }
        }
    }

    exit(EXIT_SUCCESS);
}
