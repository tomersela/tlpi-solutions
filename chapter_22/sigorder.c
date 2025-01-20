#include <signal.h>
#include "tlpi_hdr.h"


#define RT_SIG 42

static void
handler(int sig, siginfo_t *si, void *ucontext)
{
    /* UNSAFE: This handler uses non-async-signal-safe functions (printf()); see Section 21.1.2) */
    /* SIGINT or SIGTERM can be used to terminate program */
        
    printf("caught signal %d\n", sig);
    printf(" si_signo=%d, si_code=%d (%s), ", si->si_signo, si->si_code,
        (si->si_code == SI_USER) ? "SI_USER" :
            (si->si_code == SI_QUEUE) ? "SI_QUEUE" : "other");
    printf("si_value=%d\n", si->si_value.sival_int);
    printf("si_pid=%ld, si_uid=%ld\n", (long) si->si_pid, (long) si->si_uid);
}


int
main(int argc, char *argv[])
{
    struct sigaction sa;
    sigset_t orig_mask, block_mask;
    int sig;
    union sigval sv;
    int realtime_first = 0;

    if (argc >= 2 && strcmp(argv[1], "rt") == 0) {
        realtime_first = 1;
    }

    sa.sa_sigaction = handler;
    sa.sa_flags = SA_SIGINFO;
    sigfillset(&sa.sa_mask);

    for (sig = 1; sig < NSIG; sig++)
        if (sig != SIGTSTP && sig != SIGQUIT)
            sigaction(sig, &sa, NULL);
    
    // suspend a standard signal and a realtime signal
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGUSR1);
    sigaddset(&block_mask, RT_SIG);
    if (sigprocmask(SIG_BLOCK, &block_mask, &orig_mask) == -1)
        errExit("sigprocmask - SIG_BLOCK");
    
    sv.sival_int = 0;
    if (realtime_first) {
        printf("Sending realtime signal - %d\n", RT_SIG);
        sigqueue(getpid(), RT_SIG, sv);
    }
    
    printf("Sending SIGUSR1\n");
    raise(SIGUSR1);

    if (!realtime_first) {
        printf("Sending realtime signal - %d\n", RT_SIG);
        sigqueue(getpid(), RT_SIG, sv);
    }

    // resume signal handling
    if (sigprocmask(SIG_SETMASK, &orig_mask, NULL) == -1)
        errExit("sigprocmask - SIG_UNBLOCK");
}
