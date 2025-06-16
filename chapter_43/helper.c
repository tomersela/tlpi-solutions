#define _BSD_SOURCE
#define _GNU_SOURCE

#include <time.h>
#include <sched.h>

#include "tlpi_hdr.h"
#include "helper.h"

// show usage message for bandwidth measurement programs
void
show_usage(char *prog_name)
{
    usageErr("Usage: %s num-blocks block-size\n", prog_name);
}

// pin process to specific CPU core
void
set_cpu_affinity(int core)
{
    cpu_set_t cpuset;
    
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);
    
    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) == -1)
        errExit("sched_setaffinity");
}

// get current time in nanoseconds using monotonic clock
long long
get_current_time_ns(void)
{
    struct timespec ts;
    
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1)
        errExit("clock_gettime");
    
    return (long long) ts.tv_sec * 1000000000LL + ts.tv_nsec;
}
