#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>


static const char *
current_time()
{
    static char buf[64];
    time_t t = time(NULL);
    strftime(buf, sizeof(buf), "%T", localtime(&t));
    return buf;
}


static void
do_test(const char *file, int proc, int lock_start, int write_start)
{
    int fd;
    struct flock fl;
    char buf[10];

    printf("PID %d (P%d): Starting test at %s\n", getpid(), proc, current_time());

    fd = open(file, O_RDWR | O_CREAT, 0644);
    if (fd == -1) { perror("open"); exit(1); }

    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = lock_start;
    fl.l_len = 10;

    printf("PID %d (P%d): Getting lock on bytes %d-%d\n", getpid(), proc, lock_start, lock_start+9);
    if (fcntl(fd, F_SETLKW, &fl) == -1) { perror("fcntl"); exit(1); }
    printf("PID %d (P%d): Got lock\n", getpid(), proc);

    sleep(1);

    printf("PID %d (P%d): Attempting write to bytes %d-%d\n", getpid(), proc, write_start, write_start+9);
    if (lseek(fd, write_start, SEEK_SET) == -1) { perror("lseek"); exit(1); }

    sprintf(buf, "P%d_data", proc);
    if (write(fd, buf, 10) == -1) {
        if (errno == EDEADLK)
            printf("PID %d (P%d): DEADLOCK DETECTED!\n", getpid(), proc);
        else if (errno == EAGAIN)
            printf("PID %d (P%d): Write blocked\n", getpid(), proc);
        else
            perror("write");
    } else {
        printf("PID %d (P%d): Write succeeded\n", getpid(), proc);
    }

    sleep(3);
    close(fd);
}


int
main()
{
    const char *file = "testfile";
    int fd;

    // Create and setup file
    fd = open(file, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd == -1) { perror("create"); exit(1); }
    write(fd, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ", 36);
    close(fd);

    // Set mandatory locking permissions
    if (chmod(file, S_IRUSR | S_IWUSR | S_IRGRP | S_ISGID) == -1) {
        perror("chmod"); exit(1);
    }

    printf("Testing mandatory locking...\n");
    system("uname -r");

    // Create three processes with circular dependency
    pid_t p1 = fork();
    if (p1 == 0) {
        do_test(file, 2, 10, 0); // locks 10-19, writes to 0-9
        exit(0);
    }

    pid_t p2 = fork();
    if (p2 == 0) {
        do_test(file, 3, 20, 10); // locks 20-29, writes to 10-19
        exit(0);
    }

    do_test(file, 1, 0, 20); // locks 0-9, writes to 20-29

    wait(NULL);
    wait(NULL);
    unlink(file);
    return 0;
}
