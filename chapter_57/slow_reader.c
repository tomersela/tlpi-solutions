#define _GNU_SOURCE
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#include "curr_time.h"
#include "tlpi_hdr.h"

#define BUF_SIZE 8192

int
main(int argc, char* argv[])
{
    int sockfd[2];
    ssize_t num_bytes;
    char buf[BUF_SIZE];

    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sockfd) == -1)
        errExit("socketpair");

    switch(fork()) {
        case -1:
            errExit("fork");

        case 0: // child
            close(sockfd[0]); // close server (parent) side
            printf("About to send message to server at %s\n", currTime("%T"));
            
            char msg[BUF_SIZE];

            for (int i = 1; i <= 20; i++) {
                // fill message with X's to use full buffer size
                snprintf(msg, sizeof(msg), "Message %d: ", i);
                int header_len = strlen(msg);
                memset(msg + header_len, 'X', BUF_SIZE - header_len - 1);
                msg[BUF_SIZE - 1] = '\0';
                printf("[Client %s] Sending message %d\n", currTime("%T"), i);
                sendto(sockfd[1], msg, strlen(msg), 0, NULL, 0);
                printf("[Client %s] Done sending message %d\n", currTime("%T"), i);
            }
            
            // send end marker
            printf("[Client %s] Sending END message\n", currTime("%T"));
            sendto(sockfd[1], "END", 3, 0, NULL, 0);
            printf("[Client %s] Done sending END message\n", currTime("%T"));
            
            close(sockfd[1]);
            break;
                
        default: // parent

            close(sockfd[1]); // close client (child) side

            while (1) {
                num_bytes = recvfrom(sockfd[0], buf, sizeof(buf) - 1, 0, NULL, 0);
                if (num_bytes > 0) {
                    buf[num_bytes] = '\0';
                    
                    // check for end marker
                    if (strcmp(buf, "END") == 0) {
                        printf("[Server %s] Received END message, stopping\n", currTime("%T"));
                        break;
                    }

                    printf("[Server %s] Received message from client (size = %ld)\n", currTime("%T"), strlen(buf));
                    usleep(50000); // 50ms - slower receiver
                }
            }

            close(sockfd[0]);
    }

    exit(EXIT_SUCCESS);
}
