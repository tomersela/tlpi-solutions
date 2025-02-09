#include "tlpi_hdr.h"

int
main(int argc, char* argv[])
{
    return execlp("./hello_cat.sh", "./hello_cat.sh", (char *) NULL);
}
