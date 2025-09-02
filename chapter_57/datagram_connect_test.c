#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>

#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int sock_a, sock_b, sock_c;
    struct sockaddr_un addr_a, addr_b;
    char buf[100];
    int connect_flag = 0;
    int opt;

    while ((opt = getopt(argc, argv, "c")) != -1) {
        switch (opt) {
        case 'c':
            connect_flag = 1;
            break;
        default:
            fprintf(stderr, "Usage: %s [-c]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    unlink("/tmp/sock_a");
    unlink("/tmp/sock_b");

    sock_a = socket(AF_UNIX, SOCK_DGRAM, 0); // create and bind socket A to /tmp/sock_a
    memset(&addr_a, 0, sizeof(addr_a));
    addr_a.sun_family = AF_UNIX;
    strcpy(addr_a.sun_path, "/tmp/sock_a");
    if (bind(sock_a, (struct sockaddr *) &addr_a, sizeof(addr_a)) == -1)
        errExit("bind");

    sock_b = socket(AF_UNIX, SOCK_DGRAM, 0); // create and bind socket B to /tmp/sock_b
    memset(&addr_b, 0, sizeof(addr_b));
    addr_b.sun_family = AF_UNIX;
    strcpy(addr_b.sun_path, "/tmp/sock_b");
    if (bind(sock_b, (struct sockaddr *) &addr_b, sizeof(addr_b)) == -1)
        errExit("bind");

    if (connect_flag) {
        printf("Connecting Socket A to socket B...\n");
        if (connect(sock_a, (struct sockaddr *) &addr_b, sizeof(addr_b)) == -1)
            errExit("connect"); // connect socket A to socket B
    }

    sock_c = socket(AF_UNIX, SOCK_DGRAM, 0); // create third socket C
    if (sendto(sock_c, "test", 5, 0, (struct sockaddr *) &addr_a, sizeof(addr_a)) == -1) // try to send from C to A
        errExit("Failed to send message from C to A");

    // check if A received the message
    fcntl(sock_a, F_SETFL, O_NONBLOCK);
    if (recv(sock_a, buf, sizeof(buf), 0) == -1)
        printf("A did not receive message\n");
    else
        printf("A received message - %s\n", buf);

    exit(EXIT_SUCCESS);
}
