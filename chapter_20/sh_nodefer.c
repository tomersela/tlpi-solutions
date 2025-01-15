#include <signal.h>

#include "tlpi_hdr.h"


static volatile sig_atomic_t int_cnt = 0;


static void
handler(int sig)
{
    const char stuck_message[] = "\nStuck in handler...\n";
    const char release_message[] = "\nReleasing first handler...\n";
    const char free_message[] = "\nI'm free!\n";

    int_cnt++;

    if (int_cnt >= 2) {
        write(STDOUT_FILENO, release_message, sizeof(release_message));
        return;
    }

    write(STDOUT_FILENO, stuck_message, sizeof(stuck_message));
    while (int_cnt < 2)
        continue;

    write(STDOUT_FILENO, free_message, sizeof(free_message));
}


int
main(int argc, char* argv[])
{

    sigset_t emptyMask;
    sigemptyset(&emptyMask);

    struct sigaction sigact;
    memcpy(&sigact.sa_mask, &emptyMask, sizeof(sigset_t));
    sigact.sa_flags = SA_NODEFER;
    sigact.sa_handler = handler;
    sigaction(SIGQUIT, &sigact, NULL);

    while (int_cnt < 2) {}
}
