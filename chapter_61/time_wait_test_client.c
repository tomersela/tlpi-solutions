/*************************************************************************
*                  Copyright (C) Michael Kerrisk, 2024.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* time_wait_test_client.c

   A simple client to test the TIME_WAIT behavior with the test server.

   This client:
   1. Connects to the test server
   2. Sends a message
   3. Receives the echo
   4. Sends "quit" to trigger server-initiated close
   5. Exits

   Usage: ./time_wait_test_client [host] [port]
   - host: server hostname or IP (default: localhost)
   - port: server port (default: 12345)
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "tlpi_hdr.h"

#define BUF_SIZE 1024

int
main(int argc, char *argv[])
{
    int sfd;
    struct sockaddr_in svaddr;
    char *host = "localhost";
    int port = 12345;
    char buf[BUF_SIZE];
    ssize_t numRead;
    struct hostent *he;

    // parse command line arguments
    if (argc > 1) {
        host = argv[1];
    }
    if (argc > 2) {
        port = getInt(argv[2], GN_GT_0, "port");
    }

    printf("connecting to %s:%d\n", host, port);

    // create socket
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1)
        errExit("socket");

    // resolve hostname
    he = gethostbyname(host);
    if (he == NULL) {
        fprintf(stderr, "gethostbyname failed for '%s'\n", host);
        exit(EXIT_FAILURE);
    }

    // set up server address
    memset(&svaddr, 0, sizeof(svaddr));
    svaddr.sin_family = AF_INET;
    svaddr.sin_port = htons(port);
    memcpy(&svaddr.sin_addr.s_addr, he->h_addr_list[0], he->h_length);

    // connect to server
    if (connect(sfd, (struct sockaddr *) &svaddr, sizeof(svaddr)) == -1)
        errExit("connect");

    printf("connected to server\n");

    // send a test message
    const char *msg = "hello from client\n";
    if (write(sfd, msg, strlen(msg)) == -1)
        errExit("write");

    printf("sent: %s", msg);

    // read echo response
    numRead = read(sfd, buf, BUF_SIZE - 1);
    if (numRead == -1)
        errExit("read");

    buf[numRead] = '\0';
    printf("received echo: %s", buf);

    // send quit command to trigger server close
    const char *quit_msg = "quit\n";
    if (write(sfd, quit_msg, strlen(quit_msg)) == -1)
        errExit("write");

    printf("sent quit command\n");

    // read final response
    numRead = read(sfd, buf, BUF_SIZE - 1);
    if (numRead > 0) {
        buf[numRead] = '\0';
        printf("received final echo: %s", buf);
    }

    // server should close connection now
    printf("waiting for server to close connection...\n");
    numRead = read(sfd, buf, BUF_SIZE);
    if (numRead == 0) {
        printf("server closed connection (as expected)\n");
    } else if (numRead == -1) {
        printf("error reading from server: %s\n", strerror(errno));
    }

    close(sfd);
    printf("client exiting\n");

    return 0;
}
