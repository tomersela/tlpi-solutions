#include <sys/socket.h>
#include <netdb.h>

#include "tlpi_hdr.h"
#include "rdwrn.h"

#define PORT_NUM "50000"

int
main(int argc, char *argv[])
{
    int sfd;
    struct addrinfo hints, *result;
    char *server_host, *command;

    if (argc != 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s server-host 'shell-command'\n", argv[0]);

    server_host = argv[1];
    command = argv[2];

    // create socket and connect to server
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    // allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;

    if (getaddrinfo(server_host, PORT_NUM, &hints, &result) != 0)
        errExit("getaddrinfo");

    sfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sfd == -1)
        errExit("socket");

    if (connect(sfd, result->ai_addr, result->ai_addrlen) == -1)
        errExit("connect");

    freeaddrinfo(result);

    // send command to server
    if (writen(sfd, command, strlen(command)) != strlen(command))
        fatal("writen");

    // close writing half of socket to signal end-of-file to server
    if (shutdown(sfd, SHUT_WR) == -1)
        errExit("shutdown");

    // read and display output from server
    char buf[1024];
    ssize_t numRead;

    while ((numRead = read(sfd, buf, sizeof(buf))) > 0) {
        if (writen(STDOUT_FILENO, buf, numRead) != numRead)
            fatal("writen to stdout");
    }

    if (numRead == -1)
        errExit("read");

    if (close(sfd) == -1)
        errExit("close");

    exit(EXIT_SUCCESS);
}
