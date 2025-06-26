#include <sys/ipc.h>
#include <sys/stat.h>

#include <assert.h>

#include "tlpi_hdr.h"

key_t myftok(char *pathname, int proj);

key_t
myftok(char *pathname, int proj)
{
    struct stat sb;

    if (stat(pathname, &sb) == -1)
        return -1;
    
    return ((proj & 0xff) << 24) | ((sb.st_dev & 0xff) << 16) | (sb.st_ino & 0xffff);
}


int
main(int argc, char *argv[])
{
    if (argc < 3)
        usageErr("%s file-path proj");

    printf("ftok key:\t 0x%08x\n", ftok(argv[1], (char) argv[2][0]));
    printf("myftok key:\t 0x%08x\n", myftok(argv[1], (char) argv[2][0]));
}
