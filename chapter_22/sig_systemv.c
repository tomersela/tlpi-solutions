#include <signal.h>

#include "tlpi_hdr.h"

// added "my" since the definition exists in signal.h
void (*mysigset(int sig, void (*handler)(int)))(int);
int mysighold(int sig);
int mysigrelse(int sig);
int mysigignore(int sig);
int mysigpause(int sig);


void (*
mysigset(int sig, void (*handler)(int))
)(int)
{
    struct sigaction act, oldact;

    act.sa_flags = 0;
    act.sa_handler = handler;
    sigaction(sig, &act, &oldact);
    return oldact.sa_handler;
}

int
mysighold(int sig)
{
    sigset_t blocked_mask;

    sigemptyset(&blocked_mask);
    sigaddset(&blocked_mask, sig);
    return sigprocmask(SIG_BLOCK, &blocked_mask, NULL);
}

int
mysigrelse(int sig)
{
    sigset_t blocked_mask;

    sigemptyset(&blocked_mask);
    sigaddset(&blocked_mask, sig);
    return sigprocmask(SIG_UNBLOCK, &blocked_mask, NULL);
}

int
mysigignore(int sig)
{
    struct sigaction act;

    act.sa_flags = 0;
    act.sa_handler = SIG_IGN;
    return sigaction(sig, &act, NULL);
}

int
mysigpause(int sig)
{
    sigset_t mask;

    sigprocmask(SIG_BLOCK, NULL, &mask); // get current mask
    sigdelset(&mask, sig); // unmask sig
    return sigsuspend(&mask); // suspend until receiving sig
}


static void
handler(int sig)
{
    const char message[] = "The show must go on!\n";
    write(STDOUT_FILENO, message, sizeof(message));
}


static void
sigusr2_handler(int sig)
{
    const char message[] = "Welcome SIGUSR2!\n";
    write(STDOUT_FILENO, message, sizeof(message));
}


int
main(int argc, char *argv[])
{
    printf("Ignoring SIGINT.\n");
    mysigignore(SIGINT); // ignore interrupts
    printf("Sleeping for 5 seconds...\n");
    sleep(5);

    mysigset(SIGUSR1, handler); // set handler
    
    printf("\nBlocking SIGUSR1.\n");
    mysighold(SIGUSR1); // mask SIGUSR1 events
    
    printf("Raising SIGUSR1.\n");
    raise(SIGUSR1);
    
    printf("Sleeping for 5 seconds...\n");
    sleep(5);

    printf("Releasing SIGUSR1.\n");
    mysigrelse(SIGUSR1);

    mysigset(SIGUSR2, sigusr2_handler);
    printf("Waiting for SIGUSR2 (%d).\n", SIGUSR2);
    mysigpause(SIGUSR2);

    printf("Received SIGUSR2.\n");
}
