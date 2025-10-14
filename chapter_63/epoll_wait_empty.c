/* Exercise 63-5 */

/* epoll_wait_empty.c

   Demonstrates what happens when calling epoll_wait() on an empty epoll instance.
   
   Usage: ./epoll_wait_empty [timeout_ms]
*/
#include <sys/epoll.h>
#include "tlpi_hdr.h"

#define MAX_EVENTS 10

int
main(int argc, char *argv[])
{
    int epfd, ready;
    int timeout = -1;
    struct epoll_event events[MAX_EVENTS];

    if (argc > 1)
        timeout = getInt(argv[1], 0, "timeout");

    epfd = epoll_create(1);
    if (epfd == -1)
        errExit("epoll_create");

    ready = epoll_wait(epfd, events, MAX_EVENTS, timeout);
    if (ready == -1)
        errExit("epoll_wait");

    printf("ready = %d\n", ready);

    close(epfd);
    exit(EXIT_SUCCESS);
}
