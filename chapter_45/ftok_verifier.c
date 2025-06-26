#include <sys/ipc.h>
#include <sys/stat.h>
#include <assert.h>

#include "tlpi_hdr.h"


int
main(int argc, char *argv[])
{
    key_t key;
    struct stat sb;
    char proj = 'x';

    if (argc < 2)
        usageErr("%s file-path");

    if ((key = ftok("./Makefile", proj)) == -1)
        errExit("ftok");
    
    if (stat(argv[1], &sb) == -1)
        errExit("stat");
    
    int computed_key = ((proj & 0xff) << 24) | ((sb.st_dev & 0xff) << 16) | (sb.st_ino & 0xffff);

    
    printf("proj:\t\t 0x%08x\n", proj);
    printf("sb.st_dev:\t 0x%08lx\n", sb.st_dev);
    printf("sb.st_ino:\t 0x%08lx\n", sb.st_ino);

    printf("\n");
    printf("ftok key: \t 0x%04x\n", key);
    printf("computed: \t 0x%04x\n", computed_key);

    assert(key == computed_key);

}
