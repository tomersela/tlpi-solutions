#include <signal.h>
#include <string.h>

#include "tlpi_hdr.h"


unsigned int alarm(unsigned int seconds);


unsigned int
alarm(unsigned int seconds)
{
    struct itimerval itv;
    struct itimerval old_itv;
    itv.it_value.tv_sec = seconds;
    itv.it_value.tv_usec = 0;
    itv.it_interval.tv_sec = 0;
    itv.it_interval.tv_usec = 0;

    if (setitimer(ITIMER_REAL, &itv, &old_itv) == -1)
        errExit("setitimer");

    return old_itv.it_value.tv_sec;
}



static volatile sig_atomic_t int_cnt = 0;


static void
handler(int sig)
{
    int_cnt = 1;
    const char message[] = "Wake up!\n";
    write(STDOUT_FILENO, message, sizeof(message));
}


int
main(int argc, char *argv[])
{
    struct sigaction act;

    if (argc < 2 || (argc > 1 && strcmp(argv[1], "--help") == 0))
        usageErr("%s secs\n", argv[0]);

    
    act.sa_handler = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGALRM, &act, NULL);

    alarm(getInt(argv[1], GN_GT_0, "secs"));

    while (int_cnt < 1) {}

    exit(EXIT_SUCCESS);
}
