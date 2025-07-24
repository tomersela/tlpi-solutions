#define _GNU_SOURCE
#include <sys/mman.h>
#include <fcntl.h>

#include "tlpi_hdr.h"


int
main(int argc, char *argv[])
{
    char *map;
    long page_size = sysconf(_SC_PAGESIZE);
    long file_size = 3 * page_size;
    
    if (argc != 2)
        usageErr("%s <file>\n", argv[0]);
    
    int fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1)
        errExit("open");
    if (ftruncate(fd, file_size))
        errExit("ftruncate");


    // allocate 3 contiguous pages
    map = mmap(NULL, file_size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (map == MAP_FAILED)
        errExit("mmap");
    
    // map the first page of the file to third memory page
    mmap(map + (page_size * 2), page_size, PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, fd, 0);

    // map the second page of the file to second memory page
    mmap(map + page_size, page_size, PROT_READ | PROT_WRITE, MAP_FIXED| MAP_SHARED, fd, page_size);
    
    // map the third page of the file to first memory page
    mmap(map, page_size, PROT_READ | PROT_WRITE, MAP_FIXED| MAP_SHARED, fd, page_size * 2);

    for (int offset = 0; offset < page_size * 3; offset++) {
        char page_index = '0' + (offset / page_size) + 1;
        map[offset] = page_index; // saves the page index (1-3)
    }
    
    munmap(map, file_size);
    close(fd);
    exit(EXIT_SUCCESS);
}
