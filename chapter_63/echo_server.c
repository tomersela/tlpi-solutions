/* Exercise 63-2 */

/* echo_server.c

   This program implements a daemon that provides both TCP and UDP "echo" 
   services. It uses poll() to monitor both a TCP listening socket and 
   a UDP socket, as well as active TCP client connections.

   For UDP: reads datagrams and sends copies back to the originating address.
   For TCP: accepts connections and echoes back any data received.

   NOTE: this program must be run under a root login, in order to allow the
   "echo" port (7) to be bound. Alternatively, for test purposes, you can edit
   id_echo.h and replace the SERVICE name with a suitable unreserved port
   number (e.g., "51000").
*/
#include <syslog.h>
#include <poll.h>
#include "id_echo.h"
#include "become_daemon.h"

#define MAX_CLIENTS 1024       // maximum number of concurrent TCP clients

int
main(int argc, char *argv[])
{
    int tcpListenFd, udpFd;
    struct pollfd fds[MAX_CLIENTS + 2];  // +2 for TCP listen + UDP sockets
    int nfds = 2;                        // number of fds being monitored
    ssize_t numRead;
    socklen_t len;
    struct sockaddr_storage claddr;
    char buf[BUF_SIZE];
    char addrStr[IS_ADDR_STR_LEN];

    if (becomeDaemon(0) == -1)
        errExit("becomeDaemon");

    // create TCP listening socket
    tcpListenFd = inetListen(SERVICE, 10, NULL);
    if (tcpListenFd == -1) {
        syslog(LOG_ERR, "Could not create TCP server socket (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // create UDP socket
    udpFd = inetBind(SERVICE, SOCK_DGRAM, NULL);
    if (udpFd == -1) {
        syslog(LOG_ERR, "Could not create UDP server socket (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // initialize poll array - fixed positions for TCP listen and UDP
    fds[0].fd = tcpListenFd;
    fds[0].events = POLLIN;
    fds[1].fd = udpFd;  
    fds[1].events = POLLIN;

    syslog(LOG_INFO, "TCP/UDP echo server started");

    // main server loop
    for (;;) {
        if (poll(fds, nfds, -1) == -1) {
            syslog(LOG_ERR, "poll failed (%s)", strerror(errno));
            continue;
        }

        // check TCP listening socket for new connections
        if (fds[0].revents & POLLIN) {
            len = sizeof(struct sockaddr_storage);
            int newClientFd = accept(tcpListenFd, (struct sockaddr *) &claddr, &len);
            if (newClientFd == -1) {
                syslog(LOG_WARNING, "accept failed (%s)", strerror(errno));
            } else if (nfds < MAX_CLIENTS + 2) {
                // add new client to poll array
                fds[nfds].fd = newClientFd;
                fds[nfds].events = POLLIN;
                nfds++;
                syslog(LOG_INFO, "TCP client connected from %s (fd %d)",
                        inetAddressStr((struct sockaddr *) &claddr, len,
                                       addrStr, IS_ADDR_STR_LEN), newClientFd);
            } else {
                // too many clients, close connection
                close(newClientFd);
                syslog(LOG_WARNING, "Maximum clients reached, connection rejected");
            }
        }

        // check UDP socket for datagrams
        if (fds[1].revents & POLLIN) {
            len = sizeof(struct sockaddr_storage);
            numRead = recvfrom(udpFd, buf, BUF_SIZE, 0,
                               (struct sockaddr *) &claddr, &len);
            if (numRead == -1) {
                syslog(LOG_WARNING, "UDP recvfrom failed (%s)", strerror(errno));
            } else {
                // echo the datagram back
                if (sendto(udpFd, buf, numRead, 0, (struct sockaddr *) &claddr, len)
                                != numRead) {
                    syslog(LOG_WARNING, "Error echoing UDP response to %s (%s)",
                            inetAddressStr((struct sockaddr *) &claddr, len,
                                           addrStr, IS_ADDR_STR_LEN),
                            strerror(errno));
                }
            }
        }

        // check TCP client connections for data
        for (int i = 2; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                numRead = read(fds[i].fd, buf, BUF_SIZE);
                if (numRead == -1) {
                    syslog(LOG_WARNING, "read from TCP client fd %d failed (%s)",
                            fds[i].fd, strerror(errno));
                    // close and remove this client
                    close(fds[i].fd);
                    fds[i] = fds[nfds - 1];  // move last element to current position
                    nfds--;
                    i--;  // recheck current position with moved element
                } else if (numRead == 0) {
                    // client closed connection
                    syslog(LOG_INFO, "TCP client fd %d disconnected", fds[i].fd);
                    close(fds[i].fd);
                    fds[i] = fds[nfds - 1];  // move last element to current position
                    nfds--;
                    i--;  // recheck current position with moved element
                } else {
                    // echo data back to client
                    if (write(fds[i].fd, buf, numRead) != numRead) {
                        syslog(LOG_WARNING, "write to TCP client fd %d failed (%s)",
                                fds[i].fd, strerror(errno));
                        // close and remove this client
                        close(fds[i].fd);
                        fds[i] = fds[nfds - 1];  // move last element to current position
                        nfds--;
                        i--;  // recheck current position with moved element
                    }
                }
            }
        }
    }
}
