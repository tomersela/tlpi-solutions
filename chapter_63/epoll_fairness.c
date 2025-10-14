#include <sys/epoll.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

#define NUM_FDS 5
#define MAX_EVENTS 1

int main(int argc, char *argv[]) {
    int epfd, ready;
    int pipes[NUM_FDS][2];
    struct epoll_event ev, events[MAX_EVENTS];
    char data = 'x';
    
    epfd = epoll_create(NUM_FDS);
    if (epfd == -1)
        errExit("epoll_create");
    
    for (int i = 0; i < NUM_FDS; i++) {
        if (pipe(pipes[i]) == -1)
            errExit("pipe");
        
        // write to pipe to make read end ready
        if (write(pipes[i][1], &data, 1) == -1)
            errExit("write");
        
        ev.events = EPOLLIN;
        ev.data.fd = pipes[i][0];
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, pipes[i][0], &ev) == -1)
            errExit("epoll_ctl");
        
        printf("Created and added fd %d (read end of pipe %d)\n", pipes[i][0], i);
    }
    
    printf("\nAll %d descriptors are ready. Calling epoll_wait() with maxevents=1:\n", NUM_FDS);
    
    // call epoll_wait multiple times with maxevents=1
    for (int call = 1; call <= NUM_FDS + 2; call++) {
        ready = epoll_wait(epfd, events, MAX_EVENTS, 0);
        if (ready == -1)
            errExit("epoll_wait");
        
        if (ready > 0) {
            printf("Call %d: returned fd %d\n", call, events[0].data.fd);
        } else {
            printf("Call %d: no events ready\n", call);
        }
    }
    
    close(epfd);
    for (int i = 0; i < NUM_FDS; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    exit(EXIT_SUCCESS);
}
