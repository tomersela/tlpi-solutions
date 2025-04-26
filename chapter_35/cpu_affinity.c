#define _GNU_SOURCE

#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <sys/wait.h>

#include "tlpi_hdr.h"


#define BUF_SIZE (1024 * 1024 * 500) // 500MB


static void
set_cpu(int cpu)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) == -1)
        errExit("sched_setaffinity");
}


static void
run_test(int parent_cpu, int child_cpu)
{
    int pipefd[2];
    char *buffer = NULL;
    if (pipe(pipefd) == -1)
        errExit("pipe");
        
    if ((buffer = malloc(BUF_SIZE)) == NULL)
        errExit("malloc");
    
    memset(buffer, 'A', BUF_SIZE);

    switch (fork()) {
        case -1:
            errExit("fork");

        case 0: // child
            set_cpu(child_cpu);

            close(pipefd[1]); // close write end

            size_t total_read = 0;
            while (total_read < BUF_SIZE) {
                ssize_t bytes = read(pipefd[0], buffer + total_read, BUF_SIZE - total_read);
                if (bytes <= 0)
                    errExit("read");
                
                total_read += bytes;
            }

            close(pipefd[0]);
            free(buffer);
            break;

        default: // parent
            set_cpu(parent_cpu);

            close(pipefd[0]); // close read end

            struct timespec start, end;
            clock_gettime(CLOCK_MONOTONIC, &start);

            size_t total_written = 0;
            while (total_written < BUF_SIZE) {
                ssize_t bytes = write(pipefd[1], buffer + total_written, BUF_SIZE - total_written);
                if (bytes <= 0)
                    errExit("write");
                
                total_written += bytes;
            }

            clock_gettime(CLOCK_MONOTONIC, &end);

            close(pipefd[1]);
            free(buffer);
            wait(NULL);

            double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
            printf("Parent CPU %d, Child CPU %d: Time to transfer %dMB = %.6f seconds\n",
                parent_cpu, child_cpu, BUF_SIZE / (1024 * 1024), elapsed);
    }

    exit(EXIT_SUCCESS);
}


int
main(int argc, char *argv[])
{

    if (argc != 3)
        usageErr("%s <parent_cpu> <child_cpu>\n", argv[0]);

    int parent_cpu = getInt(argv[1], 0, "parent_cpu");
    int child_cpu = getInt(argv[2], 0, "parent_cpu");

    printf("Parent on CPU %d, Child on CPU %d\n", parent_cpu, child_cpu);
    run_test(parent_cpu, child_cpu);

    return 0;
}
