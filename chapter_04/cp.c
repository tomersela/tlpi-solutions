#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>
#include "tlpi_hdr.h"


#define BUF_SIZE 1024


int
main(int argc, char *argv[])
{

    int src_fd, dst_fd;
    char *src_filename, *dst_filename;
    char buf[BUF_SIZE];
    int read_size, hole_size;

    if (argc < 3 || (argc > 1 && strcmp(argv[1], "--help") == 0))
        usageErr("%s src_file dst_file\n", argv[0]);

    src_filename = argv[1];
    dst_filename = argv[2];

    src_fd = open(src_filename, O_RDONLY, 0);
    if (src_fd == -1)
            errExit(src_filename);

    dst_fd = open(dst_filename, O_WRONLY | O_CREAT | O_TRUNC,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (dst_fd == -1)
            errExit("open dst_file");
    
    while ((read_size = read(src_fd, buf, BUF_SIZE)) > 0) {
        for (int i = 0; i < read_size; i++) {
            if (buf[i] == '\0')
                hole_size++;
            else if (hole_size > 0) {
                lseek(dst_fd, hole_size, SEEK_CUR);
                write(dst_fd, buf + i, 1);
                hole_size = 0;
            }   
            else
                write(dst_fd, buf + i, 1);
        }
    }

    if (read_size == -1)
        errExit("read");

    if (close(src_fd) == -1)
        errExit("close");
    
    if (close(dst_fd) == -1)
        errExit("close");
}
