#define _GNU_SOURCE
#include <sched.h>

#include "tlpi_hdr.h"


static void
print_usage(const char *progname)
{
    fprintf(stderr, "Usage: %s [r|f] priority command [args...]\n", progname);
    exit(EXIT_FAILURE);
}


int
main(int argc, char *argv[])
{
    if (argc < 4)
    print_usage(argv[0]);

    int policy;
    if (argv[1][0] == 'r') {
        policy = SCHED_RR;
    } else if (argv[1][0] == 'f') {
        policy = SCHED_FIFO;
    } else {
        print_usage(argv[0]);
    }

    int priority = atoi(argv[2]);
    if (priority < 1 || priority > 99) {
        fprintf(stderr, "Error: Priority must be between 1 and 99.\n");
        exit(EXIT_FAILURE);
    }

    struct sched_param param;
    param.sched_priority = priority;

    // set policy and priority
    if (sched_setscheduler(0, policy, &param) == -1) {
        perror("sched_setscheduler");
        exit(EXIT_FAILURE);
    }

    // drop privileges
    uid_t uid = getuid();
    gid_t gid = getgid();
    if (setgid(gid) != 0 || setuid(uid) != 0) {
        perror("Failed to drop privileges");
        exit(EXIT_FAILURE);
    }

    // execute the command
    execvp(argv[3], &argv[3]);
    errExit("execvp");
}
