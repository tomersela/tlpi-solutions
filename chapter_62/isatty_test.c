#include "tlpi_hdr.h"
#include "isatty.h"

int
main(int argc, char *argv[])
{
    printf("stdin (fd 0): %s\n", isatty(0) ? "is a tty" : "not a tty");
    printf("stdout (fd 1): %s\n", isatty(1) ? "is a tty" : "not a tty");
    printf("stderr (fd 2): %s\n", isatty(2) ? "is a tty" : "not a tty");
    
    // test with an invalid fd
    printf("invalid fd (-1): %s\n", isatty(-1) ? "is a tty" : "not a tty");
    
    exit(EXIT_SUCCESS);
}
