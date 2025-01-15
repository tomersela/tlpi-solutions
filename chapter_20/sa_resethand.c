#include <signal.h>

#include "tlpi_hdr.h"


static volatile sig_atomic_t int_cnt = 0;


static void
handler(int sig)
{
    const char message[] = "\nYou only live once, but if you do it right, once is enough.\n";
    write(STDOUT_FILENO, message, sizeof(message));
}


int
main(int argc, char* argv[])
{

    sigset_t emptyMask;
    sigemptyset(&emptyMask);

    struct sigaction sigact;
    memcpy(&sigact.sa_mask, &emptyMask, sizeof(sigset_t));
    sigact.sa_flags = SA_RESETHAND;
    sigact.sa_handler = handler;
    sigaction(SIGQUIT, &sigact, NULL);

    for (;;) {}
}
