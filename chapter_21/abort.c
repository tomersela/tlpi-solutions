#include <signal.h>

#include "error_functions.h"
#include "tlpi_hdr.h"

void myabort(void);

void
myabort(void)
{
    struct sigaction act;
    sigset_t set;

    // unmask SIGABRT
    sigemptyset(&set);
    sigaddset(&set, SIGABRT);
    sigprocmask(SIG_UNBLOCK, &set, NULL);

    raise(SIGABRT);
    
    // register default handler
    act.sa_handler = SIG_DFL;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGABRT, &act, NULL);

    raise(SIGABRT);
}


static void
one_last_thing_before_i_die(int sig)
{
    const char message[] = "\nLast words are for fools who haven't said enough\n";
    write(STDOUT_FILENO, message, sizeof(message));
}


int main(int argc, char *argv[])
{
    sigset_t set;

    // register a custom handler
    struct sigaction act;
    act.sa_handler = &one_last_thing_before_i_die;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGABRT, &act, NULL);

    // mast SIGABRT
    sigaddset(&set, SIGABRT);
    sigprocmask(SIG_BLOCK, &set, NULL);

    // abort
    myabort();
}
