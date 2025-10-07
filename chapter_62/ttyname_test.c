#include "tlpi_hdr.h"
#include "tty_functions.h"

int
main(int argc, char *argv[])
{
    char *tty_name;
    
    printf("Testing ttyname() implementation:\n\n");
    
    // test stdin
    printf("stdin (fd 0): ");
    tty_name = ttyname(0);
    if (tty_name != NULL)
        printf("terminal name: %s\n", tty_name);
    else
        printf("not a terminal\n");
    
    // test stdout
    printf("stdout (fd 1): ");
    tty_name = ttyname(1);
    if (tty_name != NULL)
        printf("terminal name: %s\n", tty_name);
    else
        printf("not a terminal\n");
    
    // test stderr
    printf("stderr (fd 2): ");
    tty_name = ttyname(2);
    if (tty_name != NULL)
        printf("terminal name: %s\n", tty_name);
    else
        printf("not a terminal\n");
    
    // test invalid fd
    printf("invalid fd (-1): ");
    tty_name = ttyname(-1);
    if (tty_name != NULL)
        printf("terminal name: %s\n", tty_name);
    else
        printf("not a terminal\n");
    
    exit(EXIT_SUCCESS);
}
