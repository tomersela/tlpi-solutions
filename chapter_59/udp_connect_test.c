#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[]) {
    int sockA, sockB, sockC;
    struct sockaddr_in addrA, addrB, addrC;
    char message[] = "Hello from socket C";
    char buffer[1024];
    ssize_t numBytes;

    // create three UDP sockets
    sockA = socket(AF_INET, SOCK_DGRAM, 0);
    sockB = socket(AF_INET, SOCK_DGRAM, 0);
    sockC = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (sockA == -1 || sockB == -1 || sockC == -1)
        errExit("socket");

    // setup addresses
    memset(&addrA, 0, sizeof(addrA));
    addrA.sin_family = AF_INET;
    addrA.sin_addr.s_addr = inet_addr("127.0.0.1");
    addrA.sin_port = htons(12345);

    memset(&addrB, 0, sizeof(addrB));
    addrB.sin_family = AF_INET;
    addrB.sin_addr.s_addr = inet_addr("127.0.0.1");
    addrB.sin_port = htons(12346);

    memset(&addrC, 0, sizeof(addrC));
    addrC.sin_family = AF_INET;
    addrC.sin_addr.s_addr = inet_addr("127.0.0.1");
    addrC.sin_port = htons(12347);

    // bind sockets A and B to their addresses
    if (bind(sockA, (struct sockaddr *) &addrA, sizeof(addrA)) == -1)
        errExit("bind sockA");
    printf("sockA bound to 127.0.0.1:12345\n");
    
    if (bind(sockB, (struct sockaddr *) &addrB, sizeof(addrB)) == -1)
        errExit("bind sockB");
    printf("sockB bound to 127.0.0.1:12346\n");

    if (bind(sockC, (struct sockaddr *) &addrC, sizeof(addrC)) == -1)
        errExit("bind sockC");
    printf("sockC bound to 127.0.0.1:12347\n");

    

    // connect socket A to socket B's address
    if (connect(sockA, (struct sockaddr *) &addrB, sizeof(addrB)) == -1)
        errExit("connect sockA to sockB");
    
    printf("sockA connected to sockB\n");

    printf("sockC sending to sockA address...\n");
    
    if (sendto(sockC, message, strlen(message), 0, 
               (struct sockaddr *) &addrA, sizeof(addrA)) == -1) {
        errMsg("sendto from C to A");
    } else {
        printf("sendto() from C to A succeeded\n");
    }

    struct timeval timeout = {.tv_sec = 2, .tv_usec = 0};
    setsockopt(sockA, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    numBytes = recvfrom(sockA, buffer, sizeof(buffer) - 1, 0, NULL, NULL);
    if (numBytes == -1) {
        errMsg("recvfrom on sockA");
        printf("sockA did not receive message\n");
    } else {
        buffer[numBytes] = '\0';
        printf("sockA received: '%s'\n", buffer);
    }

    printf("sockA sending to sockB...\n");
    char message2[] = "Hello from sockA";
    if (send(sockA, message2, strlen(message2), 0) == -1) {
        errMsg("send from A to B");
    } else {
        printf("send() from A to B succeeded\n");
    }

    // try to receive on socket B
    setsockopt(sockB, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    numBytes = recvfrom(sockB, buffer, sizeof(buffer) - 1, 0, NULL, NULL);
    if (numBytes == -1) {
        errMsg("recvfrom on sockB");
    } else {
        buffer[numBytes] = '\0';
        printf("sockB received: '%s'\n", buffer);
    }
    
    close(sockA);
    close(sockB);
    close(sockC);

    return 0;
}
