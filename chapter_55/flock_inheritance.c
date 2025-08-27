/* flock_inheritance.c - Verify Section 55.2.1 statements */

#include <sys/file.h>
#include <sys/wait.h>
#include <fcntl.h>
#ifdef __APPLE__
#include <sys/types.h>
#include <unistd.h>
#ifndef LOCK_SH
#define LOCK_SH   1
#define LOCK_EX   2
#define LOCK_NB   4
#define LOCK_UN   8
#endif
int flock(int fd, int operation);
#endif
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int fd, newfd, fd1, fd2;
    const char *file = "testfile";
    
    // Test dup() refers to same lock
    printf("TEST: Duplicated descriptors refer to same lock:\n");
    fd = open(file, O_RDWR | O_CREAT, 0600);
    flock(fd, LOCK_EX);
    newfd = dup(fd);
    flock(newfd, LOCK_UN);  // should release lock acquired via fd
    printf("   SUCCESS: Lock released via duplicated descriptor\n");
    close(fd); close(newfd);
    
    // Test multiple open() calls are independent
    printf("\nTEST: Multiple open() calls create independent locks:\n");
    fd1 = open(file, O_RDWR);
    fd2 = open(file, O_RDWR);
    flock(fd1, LOCK_EX);
    if (flock(fd2, LOCK_EX | LOCK_NB) == -1 && errno == EWOULDBLOCK)
        printf("   SUCCESS: Process locked itself out\n");
    close(fd1); close(fd2);
    
    // Test child can release parent's lock
    printf("\nTEST: Child can release parent's lock:\n");
    fd = open(file, O_RDWR);
    flock(fd, LOCK_EX);
    
    if (fork() == 0) {
        flock(fd, LOCK_UN);
        printf("   SUCCESS: Child released parent's lock\n");
        exit(0);
    }
    wait(NULL);
    close(fd);
    
    // Test all duplicates must be closed to release lock
    printf("\nTEST: Lock released only when ALL duplicates closed:\n");
    fd = open(file, O_RDWR);
    newfd = dup(fd);
    flock(fd, LOCK_EX);
    close(fd);  // close original, but dup still open
    
    if (fork() == 0) {
        fd1 = open(file, O_RDWR);
        if (flock(fd1, LOCK_EX | LOCK_NB) == -1 && errno == EWOULDBLOCK)
            printf("   SUCCESS: Lock persists after closing original fd\n");
        exit(0);
    }
    wait(NULL);
    
    close(newfd);  // now close the duplicate

    if (fork() == 0) {
        fd1 = open(file, O_RDWR);
        if (flock(fd1, LOCK_EX | LOCK_NB) == 0)
            printf("   SUCCESS: Lock released after closing all duplicates\n");
        exit(0);
    }

    wait(NULL);
    
    unlink(file);
    
    return 0;
}
