#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include "tlpi_hdr.h"


static double
time_diff(struct timeval start, struct timeval end)
{
    return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
}


int
main(int argc, char *argv[])
{
    if (argc < 2) {
        usageErr("%s command [args...]\n", argv[0]);
    }

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    pid_t pid = fork();
    if (pid < 0) {
        errExit("fork");
    }

    if (pid == 0) { // child
        execvp(argv[1], &argv[1]);
        errExit("execvp");
    }

    // parent
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        errExit("waitpid");
    }

    gettimeofday(&end_time, NULL);

    struct rusage usage;
    if (getrusage(RUSAGE_CHILDREN, &usage) != 0) {
        errExit("getrusage");
    }

    double real = time_diff(start_time, end_time); // wall clock time
    double user = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6;
    double sys  = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec / 1e6;

    printf("real\t%dm%.3fs\n", (int)(real / 60), real - ((int)(real / 60)) * 60);
    printf("user\t%dm%.3fs\n", (int)(user / 60), user - ((int)(user / 60)) * 60);
    printf("sys\t%dm%.3fs\n", (int)(sys / 60), sys - ((int)(sys / 60)) * 60);

    exit(EXIT_SUCCESS);
}
