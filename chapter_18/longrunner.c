#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

static void
handle_signal(int sig) {
    printf("\nProgram received signal %d, exiting...\n", sig);
    exit(0);


}

int main() {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    printf("Program will run for 20 seconds.\n");

    sleep(20);

    printf("20 seconds elapsed, exiting program.\n");
    return 0;
}
