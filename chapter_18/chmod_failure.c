#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int fd, res;
    mkdir("test", S_IRUSR | S_IWUSR | S_IXUSR);
    chdir("test");
    fd = open("myfile", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    symlink("myfile", "../mylink");
    if ((res = chmod("../mylink", S_IRUSR)) == -1)
        errExit("chmod");
    close(fd);
}
