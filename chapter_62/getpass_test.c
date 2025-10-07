#include "tlpi_hdr.h"
#include "getpass.h"

int
main(int argc, char *argv[])
{
    char *pass;

    pass = getpass("Please enter your password: ");
    printf("Top secret: %s\n", pass);
}
