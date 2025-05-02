#define _POSIX_C_SOURCE 199309

#include <time.h>
#include <signal.h>
#include <sys/resource.h>

#include "tlpi_hdr.h"


#define CPU_TIME_LIMIT_SEC 5


static struct timespec process_start_time;

static void
utoa(unsigned long val, char *buf)
{
    char tmp[32];
    int i = 0;
    do {
        tmp[i++] = '0' + val % 10;
        val /= 10;
    } while (val);
    int j = 0;
    while (i--) {
        buf[j++] = tmp[i];
    }
    buf[j] = '\0';
}


static unsigned long
elapsed_time_ms()
{
    struct timespec now;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);

    time_t delta_sec = now.tv_sec - process_start_time.tv_sec;
    long delta_nsec = now.tv_nsec - process_start_time.tv_nsec;
    if (delta_nsec < 0) {
        delta_sec -= 1;
        delta_nsec += 1000000000L;
    }

    return delta_sec * 1000 + delta_nsec / 1000000;
}


static void
handler(int sig)
{
    const char prefix[] = "[SIGXCPU HANDLER] - ";
    const char suffix[] = "ms passed since process init\n";
    char ms_buf[32];

    utoa(elapsed_time_ms(), ms_buf);

    write(STDOUT_FILENO, prefix, sizeof(prefix));
    write(STDOUT_FILENO, ms_buf, strlen(ms_buf));
    write(STDOUT_FILENO, suffix, sizeof(suffix));
}


int
main(int argc, char *argv[])
{
    struct sigaction sa;
    clock_t elapsed = 0;
    struct rlimit limit = {
        1, // soft limit
        4 // hard limit
    };

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &process_start_time);
    
    printf("set handler for SIGXCPU\n");

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handler;

    if (sigaction(SIGXCPU, &sa, NULL) == -1)
        errExit("sigaction");


    if (setrlimit(RLIMIT_CPU,  &limit) == -1)
        errExit("setrlimit");

    printf("Child: About to consume %d seconds worth of CPU...\n", CPU_TIME_LIMIT_SEC);

    while (elapsed < CPU_TIME_LIMIT_SEC * CLOCKS_PER_SEC) {
        elapsed = clock();
    }

    printf("We should not get here!\n");
    exit(EXIT_SUCCESS);
}
