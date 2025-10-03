/*************************************************************************
*                  Copyright (C) Michael Kerrisk, 2024.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* time_wait_test_server.c

   A test server to demonstrate TIME_WAIT behavior and EADDRINUSE errors.

   This server:
   1. Binds to a specified port
   2. Accepts one connection
   3. Receives data and echoes it back
   4. Actively closes the connection (server initiates close)
   5. Exits, leaving the socket in TIME_WAIT state

   Usage: ./time_wait_test_server [port] [use_reuseaddr]
   - port: port number to bind to (default: 12345)
   - use_reuseaddr: 1 to set SO_REUSEADDR, 0 to not set it (default: 0)
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "tlpi_hdr.h"

#define BUF_SIZE 1024
#define BACKLOG 5

static volatile sig_atomic_t gotSigterm = 0;

static void
sigtermHandler(int sig)
{
    gotSigterm = 1;
}

int
main(int argc, char *argv[])
{
    int lfd, cfd;
    struct sockaddr_in svaddr, claddr;
    socklen_t addrlen;
    char buf[BUF_SIZE];
    ssize_t numRead;
    int port = 12345;
    int useReuseAddr = 0;
    int optval = 1;
    struct sigaction sa;

    // parse command line arguments
    if (argc > 1) {
        port = getInt(argv[1], GN_GT_0, "port");
    }
    if (argc > 2) {
        useReuseAddr = getInt(argv[2], 0, "use_reuseaddr");
    }

    printf("starting server on port %d (SO_REUSEADDR: %s)\n",
           port, useReuseAddr ? "enabled" : "disabled");

    // set up signal handler for graceful shutdown
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = sigtermHandler;
    if (sigaction(SIGTERM, &sa, NULL) == -1)
        errExit("sigaction");
    if (sigaction(SIGINT, &sa, NULL) == -1)
        errExit("sigaction");

    // create socket
    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1)
        errExit("socket");

    // set SO_REUSEADDR if requested
    if (useReuseAddr) {
        if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
            errExit("setsockopt SO_REUSEADDR");
        printf("SO_REUSEADDR enabled\n");
    }

    // bind to address
    memset(&svaddr, 0, sizeof(svaddr));
    svaddr.sin_family = AF_INET;
    svaddr.sin_addr.s_addr = INADDR_ANY;
    svaddr.sin_port = htons(port);

    if (bind(lfd, (struct sockaddr *) &svaddr, sizeof(svaddr)) == -1) {
        if (errno == EADDRINUSE) {
            printf("ERROR: address already in use (EADDRINUSE)\n");
            printf("this indicates the port is still in TIME_WAIT state\n");
        }
        errExit("bind");
    }

    printf("bind successful\n");

    // listen for connections
    if (listen(lfd, BACKLOG) == -1)
        errExit("listen");

    printf("listening for connections...\n");

    // accept one connection
    addrlen = sizeof(claddr);
    cfd = accept(lfd, (struct sockaddr *) &claddr, &addrlen);
    if (cfd == -1)
        errExit("accept");

    printf("connection accepted from %s:%d\n",
           inet_ntoa(claddr.sin_addr), ntohs(claddr.sin_port));

    // echo loop - read data and echo it back
    while (!gotSigterm) {
        numRead = read(cfd, buf, BUF_SIZE - 1);
        if (numRead == -1) {
            if (errno == EINTR)
                continue;
            errExit("read");
        }

        if (numRead == 0) {
            printf("client closed connection\n");
            break;
        }

        buf[numRead] = '\0';
        printf("received: %s", buf);

        if (write(cfd, buf, numRead) != numRead)
            errExit("write");

        // if client sends "quit", server initiates close
        if (strncmp(buf, "quit", 4) == 0) {
            printf("quit command received, server closing connection\n");
            break;
        }
    }

    // server actively closes the connection
    printf("server closing connection (will enter TIME_WAIT state)\n");
    close(cfd);
    close(lfd);

    printf("server exiting - socket should be in TIME_WAIT state\n");
    printf("try running this program again immediately to see EADDRINUSE error\n");

    return 0;
}
