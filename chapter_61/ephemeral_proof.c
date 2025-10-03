#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "tlpi_hdr.h"


int
main(int argc, char *argv[])
{
    int sockfd[3];
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int i;

    for (i = 0; i < 3; i++) {
        sockfd[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd[i] == -1)
            errExit("socket");

        if (listen(sockfd[i], 5) == -1)
            errExit("listen");

        if (getsockname(sockfd[i], (struct sockaddr *) &addr, &addrlen) == -1)
            errExit("getsockname");

        printf("socket %d: fd=%d, port=%d\n", i + 1, sockfd[i], ntohs(addr.sin_port));
    }

    // clean up
    for (i = 0; i < 3; i++)
        close(sockfd[i]);

    exit(EXIT_SUCCESS);
}
