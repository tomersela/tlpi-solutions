#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void sigchld_handler(int sig) {
    printf("SIGCHLD received\n");
}

int main() {
    struct sigaction sa;
    sigset_t block_mask, old_mask;

    // Set up SIGCHLD handler
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGCHLD, &sa, NULL);

    // Block SIGCHLD
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &block_mask, &old_mask);

    pid_t child_pid = fork();
    if (child_pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (child_pid == 0) {
        // Child process exits immediately
        printf("Child process exiting...\n");
        _exit(EXIT_SUCCESS);
    } else {
        // Parent waits for child to terminate
        sleep(1);  // Give child time to exit
        printf("Parent calling wait()...\n");
        wait(NULL);

        // Unblock SIGCHLD
        printf("Parent unblocking SIGCHLD...\n");
        sigprocmask(SIG_SETMASK, &old_mask, NULL);

        // Sleep to see if signal handler is called
        sleep(1);
        printf("Parent exiting\n");
    }

    return 0;
}
