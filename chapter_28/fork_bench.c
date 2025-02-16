#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>

#include "tlpi_hdr.h"


static void
usage_error(char *pname)
{
    fprintf(stderr, "Usage: %s (fork|vfork) iterations alloc-size\n\n", pname);
    exit(EXIT_FAILURE);
}


static void
bench_fork(pid_t (*method)(), int iterations)
{
    pid_t child_pid;
    for (int i = 0; i < iterations; i++) {
        switch (child_pid = method()) {
        case -1:
            errExit("fork");
            
        case 0:
            exit(EXIT_SUCCESS);

        default:
            if (waitpid(child_pid, NULL, 0) == -1)
                errExit("waitpid");
            break;
        }
    }
}


int
main(int argc, char *argv[])
{
    int iterations;
    size_t alloc_size;
    char *method;

    if (argc < 4)
        usage_error(argv[0]);

    
    method = argv[1];
    iterations = strtol(argv[2], NULL, 0);
    alloc_size = strtol(argv[3], NULL, 0);

    if (malloc(alloc_size) == NULL)
        errExit("malloc");

    char *mem = malloc(alloc_size);
    for (size_t i = 0; i < alloc_size; i += 4096) { // write to each page
        mem[i] = 1;
    }

    struct timespec start_real, end_real;
    clock_t start_cpu, end_cpu;

    // Start measuring time
    clock_gettime(CLOCK_MONOTONIC, &start_real);
    start_cpu = clock();

    if (strcmp(method, "fork") == 0) {
        bench_fork(fork, iterations);
    } else if (strcmp(method, "vfork") == 0) {
        bench_fork(vfork, iterations);
    } else {
        fprintf(stderr, "Invalid method - %s\n", method);
        usage_error(argv[0]);
    }

    // Stop measuring time
    end_cpu = clock();
    clock_gettime(CLOCK_MONOTONIC, &end_real);

    // Calculate elapsed time
    double elapsed_time = (end_real.tv_sec - start_real.tv_sec) +
                          (end_real.tv_nsec - start_real.tv_nsec) / 1e9;
    
    // Calculate CPU time
    double cpu_time = ((double) (end_cpu - start_cpu)) / CLOCKS_PER_SEC;

    // Print results
    printf("Elapsed time: %f seconds\n", elapsed_time);
    printf("CPU time: %f seconds\n", cpu_time);

    exit(EXIT_SUCCESS);
}
