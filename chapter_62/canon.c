#include <termios.h>

#include "tlpi_hdr.h"


int
main(int argc, char* argv[])
{
    struct termios t;
    int time, min;

    if (tcgetattr(STDIN_FILENO, &t) == -1) {
        fprintf(stderr ,"Standard input isn't connected to a terminal\n");
        exit(EXIT_FAILURE);
    }

    if ((t.c_lflag & ICANON)) {
        printf("Standard input is in canonical mode\n");
    } else {
        time = t.c_cc[VTIME];
        min = t.c_cc[VMIN];
        printf("Standard input is in noncanonical mode (TIME = %d, MIN = %d)\n", time, min);
    }
}
