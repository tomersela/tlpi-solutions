#define _GNU_SOURCE

#include <sys/times.h>
#include <sched.h>

#include "tlpi_hdr.h"

#define CPU_TIME_LIMIT_SEC 3
#define PRINT_INTERVAL_TICKS 0.25
#define YIELD_INTERVAL_TICKS 1


int
main(int argc, char *argv[])
{
    struct sched_param sp;
    struct tms t_start, t_now;
    clock_t last_print = 0;
    clock_t last_yield = 0;
    long ticks_per_sec = sysconf(_SC_CLK_TCK);
    double elapsed = 0;

    setbuf(stdout, NULL);               /* Make stdout unbuffered */

    printf("parent process PID: %d\n", getpid());

    // set FIFO policy
    sp.sched_priority = 10; // relatively low priority to avoid locking the system
    if (sched_setscheduler(0, SCHED_FIFO, &sp) == -1)
        errExit("sched_setscheduler");

    // create child process
    if (fork() == -1)
        errExit("fork");
    
    pid_t pid = getpid();

    if (argc > 1) { // pin process to CPU 1
        cpu_set_t set;
        CPU_ZERO(&set);
        CPU_SET(1, &set);
        sched_setaffinity(pid, CPU_SETSIZE, &set);
    }
    
    // consume CPU for 3 seconds...
    times(&t_start);
    while (elapsed < CPU_TIME_LIMIT_SEC) {
        times(&t_now);
        clock_t elapsed_cpu_ticks = (t_now.tms_utime - t_start.tms_utime) + (t_now.tms_stime - t_start.tms_stime);
        elapsed = ((double) elapsed_cpu_ticks) / ticks_per_sec;

        if (elapsed - ((double)last_print / ticks_per_sec) >= PRINT_INTERVAL_TICKS) {
            printf("PID %d - CPU time used: %.2f seconds\n", pid, elapsed);
            last_print = elapsed_cpu_ticks;
        }

        if (elapsed - ((double)last_yield / ticks_per_sec) >= YIELD_INTERVAL_TICKS) {
            printf("PID %d - yielding CPU\n", pid);
            sched_yield();
            last_yield = elapsed_cpu_ticks;
        }
    }

    printf("PID %d - done consuming CPU time\n", pid);

    exit(EXIT_SUCCESS);
}
