/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2024.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Listing 46-8 */

/* svmsg_file_server.c

   A file server that uses System V message queues to handle client requests
   (see svmsg_file_client.c). The client sends an initial request containing
   the name of the desired file, and the identifier of the message queue to be
   used to send the file contents back to the child. The server attempts to
   open the desired file. If the file cannot be opened, a failure response is
   sent to the client, otherwise the contents of the requested file are sent
   in a series of messages.

   This application makes use of multiple message queues. The server maintains
   a queue (with a well-known key) dedicated to incoming client requests. Each
   client creates its own private queue, which is used to pass response
   messages from the server back to the client.

   This program operates as a concurrent server, forking a new child process to
   handle each client request while the parent waits for further client requests.
*/
#include <syslog.h>

#include "svmsg_file_mod.h"


static int serverId = -1;


static void             /* SIGCHLD handler */
grimReaper(int sig)
{
    int savedErrno;

    savedErrno = errno;                 /* waitpid() might change 'errno' */
    while (waitpid(-1, NULL, WNOHANG) > 0)
        continue;
    errno = savedErrno;
}

static void
removeServerQueue(void)
{
    if (serverId != -1) {
        if (msgctl(serverId, IPC_RMID, NULL) == -1)
            syslog(LOG_ERR, "Failed to remove message queue: %s", strerror(errno));
        else
            syslog(LOG_INFO, "Removed server message queue (ID %d)", serverId);
    }
}

static void
cleanup(void)
{
    syslog(LOG_INFO, "Server exiting. removing key file...");

    /* Remove Key file*/
    unlink(KEY_FILE);

    /* Close syslog */
    closelog();
}

static void
termHandler(int sig)
{
    syslog(LOG_INFO, "Received signal %d, initiating shutdown...", sig);
    removeServerQueue();   // msgctl + log
    cleanup();             // key file + syslog

    /* Disestablish this handler and re-raise signal to terminate properly and with the right status */
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(sig, &sa, NULL) == -1)
        syslog(LOG_ERR, "Failed to reset signal handler: %s", strerror(errno));

    raise(sig);
}

static void
daemonize(void)
{
    pid_t pid = fork();
    if (pid == -1)
        errExit("fork");

    if (pid != 0)         // Parent exits
        exit(EXIT_SUCCESS);

    if (setsid() == -1) // Free ourself from the controlling terminal
        errExit("setsid");

    umask(0);

    if (chdir("/") == -1)
        errExit("chdir");

    // Redirect standard I/O to /dev/null
    int fd = open("/dev/null", O_RDWR);
    if (fd == -1)
        errExit("open /dev/null");

    if (dup2(fd, STDIN_FILENO) == -1)
        errExit("dup2 - stdin");
    if (dup2(fd, STDOUT_FILENO) == -1)
        errExit("dup2 - stdout");
    if (dup2(fd, STDERR_FILENO) == -1)
        errExit("dup2 - stderr");

    if (fd > STDERR_FILENO)
        close(fd);

}

static void             /* Executed in child process: serve a single client */
serveRequest(const struct requestMsg *req)
{
    int fd;
    ssize_t numRead;
    struct responseMsg resp;

    fd = open(req->pathname, O_RDONLY);
    if (fd == -1) {                     /* Open failed: send error text */
        syslog(LOG_ERR, "Failed to open file '%s': %s", req->pathname, strerror(errno));
        resp.mtype = RESP_MT_FAILURE;
        snprintf(resp.data, sizeof(resp.data), "%s", "Couldn't open");
        msgsnd(req->clientId, &resp, strlen(resp.data) + 1, 0);
        _exit(EXIT_FAILURE);             /* and terminate */
    }

    /* Transmit file contents in messages with type RESP_MT_DATA. We don't
       diagnose read() and msgsnd() errors since we can't notify client. */

    resp.mtype = RESP_MT_DATA;
    while ((numRead = read(fd, resp.data, RESP_MSG_SIZE)) > 0) {
        if (msgsnd(req->clientId, &resp, numRead, 0) == -1) {
            syslog(LOG_ERR, "Failed to send data to client queue %d: %s", req->clientId, strerror(errno));
            break;
        }
    }

    /* Send a message of type RESP_MT_END to signify end-of-file */

    resp.mtype = RESP_MT_END;
    if (msgsnd(req->clientId, &resp, 0, 0) == -1)         /* Zero-length mtext */
        syslog(LOG_ERR, "Failed to send EOF message to client %d: %s", req->clientId, strerror(errno));
}

int
main(int argc, char *argv[])
{
    struct requestMsg req;
    pid_t pid;
    ssize_t msgLen;
    struct sigaction sa;

    /* Run server as daemon */
    daemonize();

    /* Create server message queue */

    serverId = msgget(IPC_PRIVATE, S_IRUSR | S_IWUSR | S_IWGRP);
    if (serverId == -1)
        errExit("msgget");
    
    /* Open syslog for our server */
    openlog("svmsgd", LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "Server started and message queue created with ID %d", serverId);


    /* Save server's message queue id to a well known file */

    int fd = open(KEY_FILE, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1)
        errExit("open");

    char keyStr[32];
    snprintf(keyStr, sizeof(keyStr), "%d\n", serverId);

    if (write(fd, keyStr, strlen(keyStr)) != (ssize_t)strlen(keyStr))
        errExit("write");

    if (close(fd) == -1)
        errExit("close");

    /* Register a function to remove the key file on exit */
    if (atexit(cleanup) != 0)
        errExit("atexit");

    /* Establish SIGCHLD handler to reap terminated children */

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = grimReaper;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
        errExit("sigaction");
    
    /* Handle SIGINT and SIGTERM */
    
    struct sigaction sa_term;
    sa_term.sa_handler = termHandler;
    sigemptyset(&sa_term.sa_mask);
    sa_term.sa_flags = 0;

    if (sigaction(SIGTERM, &sa_term, NULL) == -1)
        errExit("sigaction - SIGTERM");

    if (sigaction(SIGINT, &sa_term, NULL) == -1)
        errExit("sigaction - SIGINT");

    /* Read requests, handle each in a separate child process */

    for (;;) {
        msgLen = msgrcv(serverId, &req, REQ_MSG_SIZE, 0, 0);
        if (msgLen == -1) {
            if (errno == EINTR)         /* Interrupted by SIGCHLD handler? */
                continue;               /* ... then restart msgrcv() */
            errMsg("msgrcv");           /* Some other error */
            break;                      /* ... so terminate loop */
        }

        pid = fork();                   /* Create child process */
        if (pid == -1) {
            errMsg("fork");
            break;
        }

        if (pid == 0) {                 /* Child handles request */
            serveRequest(&req);
            _exit(EXIT_SUCCESS);
        }

        /* Parent loops to receive next client request */
    }

    /* If msgrcv() or fork() fails, remove server MQ and exit */

    removeServerQueue();
    syslog(LOG_INFO, "Server exiting via fallback cleanup path in main()");
    exit(EXIT_SUCCESS);
}
