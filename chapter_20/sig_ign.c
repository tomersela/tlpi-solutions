#include <signal.h>

#include "tlpi_hdr.h"


#define MAX_SIGNALS 5

static volatile sig_atomic_t sig_cnt = 0;


static void
handler(int sig)
{
    const char message[] = "\nGot SIGQUIT!\n";

    if (sig_cnt < MAX_SIGNALS) {
        write(STDOUT_FILENO, message, sizeof(message));
        sig_cnt++;
    }
}


int
main(int argc, char *argv[])
{
    signal(SIGQUIT, handler);

    while (sig_cnt < MAX_SIGNALS)
        continue;
    
    signal(SIGQUIT, SIG_IGN);

    printf("Type Ctrl+c to exit\n");

    for (;;) {}
}
