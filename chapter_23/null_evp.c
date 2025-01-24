#include <signal.h>
#include <time.h>
#include "tlpi_hdr.h"


static volatile sig_atomic_t got_alarm = 0;

static void
handler(int sig, siginfo_t *si, void *uc)
{
    printf("Received signal %d (sival_int = %d) \n", sig, si->si_value.sival_int);
    got_alarm = 1;
}



int
main(int argc, char *argv[])
{
    timer_t tid;
    struct sigaction act;

    if (argc < 2 || (argc > 1 && strcmp(argv[1], "--help") == 0))
        usageErr("%s secs\n", argv[0]);

    // register handler for SIGALRM signal
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = handler;
    sigemptyset(&act.sa_mask);
    if (sigaction(SIGALRM, &act, NULL) == -1)
        errExit("sigaction");
    
    // create a new timer with evp set as NULL
    if (timer_create(CLOCK_REALTIME, NULL, &tid) == -1)
        errExit("timer_create");

    printf("Timer id = %ld\n", (long) tid);
    
    // arming the timer
    struct itimerspec ts;
    ts.it_value.tv_sec = getInt(argv[1], GN_GT_0, "secs");
    ts.it_value.tv_nsec = 0;
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;
    if (timer_settime(tid, 0, &ts, NULL) == -1)
        errExit("timer_settime");

    while (!got_alarm)
        pause();
}
