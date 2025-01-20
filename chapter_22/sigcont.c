#include <signal.h>
#include <time.h>

#include "tlpi_hdr.h"


static void
sigcont_handler(int sig)
{
    const char message[] = "\nThe show must go on!\n";
    write(STDOUT_FILENO, message, sizeof(message));
}


int
main(int argc, char *argv[])
{
    int num_secs;
    struct sigaction sa;
    sigset_t orig_mask, block_mask;

    if (argc > 1 && strcmp(argv[1], "--help") == 0)
        usageErr("%s [delay-secs]\n", argv[0]);

    // register handler for SIGCONT
    sa.sa_handler = sigcont_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGCONT, &sa, NULL) == -1)
        errExit("sigaction");

    // block SIGCONT
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGCONT);
    if (sigprocmask(SIG_BLOCK, &block_mask, &orig_mask) == -1)
        errExit("sigprocmask - SIG_BLOCK");
    
    num_secs = getInt(argv[1], GN_GT_0, "delay-secs");
    printf("%s: Sleeping for %d seconds...\n", argv[0], num_secs);
    sleep(num_secs);

    // unblock SIGCONT
    if (sigprocmask(SIG_UNBLOCK, &block_mask, &orig_mask) == -1)
        errExit("sigprocmask - SIG_UNBLOCK");

}
