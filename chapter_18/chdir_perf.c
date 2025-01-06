#include <time.h>
#include <limits.h>
#include <fcntl.h>

#include "tlpi_hdr.h"


#define ROOT_PATH "/"
#define TEN_MILLION (10 * 1000 * 1000)


int
main(int argc, char* argv[])
{
    clock_t start, end;
    int root_fd, curr_path_fd;
    char current_path[PATH_MAX];

    root_fd = open(ROOT_PATH, O_RDONLY, 0);
    curr_path_fd = open(".", O_RDONLY, 0);

    start = clock();
    
    for (int i = 0; i < TEN_MILLION; i++) {
        fchdir(root_fd);
        fchdir(curr_path_fd);
    }
    
    end = clock();
    printf("Execution time for fchdir: %f seconds\n", ((double)(end - start)) / CLOCKS_PER_SEC);

    close(root_fd);
    close(curr_path_fd);


    if (getcwd(current_path, sizeof(current_path)) == NULL)
        errExit("getcwd");
    
    start = clock();
    
    for (int i = 0; i < TEN_MILLION; i++) {
        chdir(ROOT_PATH);
        chdir(current_path);
    }
    
    end = clock();
    printf("Execution time for chdir: %f seconds\n", ((double)(end - start)) / CLOCKS_PER_SEC);    
}
