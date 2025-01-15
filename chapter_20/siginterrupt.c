#include <signal.h>

#include "tlpi_hdr.h"

int mysiginterrupt(int sig, int flag);

int
mysiginterrupt(int sig, int flag)
{
    struct sigaction oldact;
    if (sigaction(sig, NULL, &oldact) == -1)
        return -1;
    
    if (flag == 0)
        oldact.sa_flags |= SA_RESTART;
    else
        oldact.sa_flags &= ~SA_RESTART;
    
    return sigaction(sig, &oldact, NULL);
}



static void
handler(int sig)
{
    const char message[] = "\nCaught SIGINT.\n";
    write(STDOUT_FILENO, message, sizeof(message));
}


int main() {
    struct sigaction sa;
    char buffer[100];

    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    printf("Setting mysiginterrupt to 1 (system calls will be interrupted).\n");
    mysiginterrupt(SIGINT, 1);

    printf("Waiting for input. Press Ctrl+C to send SIGINT.\n");
    if (read(STDIN_FILENO, buffer, sizeof(buffer)) == -1) {
        perror("read interrupted");
    } else {
        printf("input received: %s\n", buffer);
    }

    printf("\nSetting mysiginterrupt to 0 (system calls will restart).\n");
    mysiginterrupt(SIGINT, 0);

    printf("Waiting for input again. Press Ctrl+C to send SIGINT.\n");
    if (read(STDIN_FILENO, buffer, sizeof(buffer)) == -1) {
        perror("read interrupted");
    } else {
        printf("input received: %s\n", buffer);
    }

    printf("Exiting program.\n");
    return 0;
}
