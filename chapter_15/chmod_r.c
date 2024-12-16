#include <sys/stat.h>

#include "tlpi_hdr.h"

#define READ_PERMISSIONS (S_IRUSR | S_IRGRP | S_IROTH)
#define EXECUTE_PERMISSIONS (S_IXUSR | S_IXGRP | S_IXOTH)

static int
is_execute_enabled(mode_t mode)
{
    return mode & (EXECUTE_PERMISSIONS);
}

int
main(int argc, char* argv[])
{
    if (argc < 2) {
        usageErr("%s <file 1> [<file 2> <file 3> ...]", argv[0]);
    }

    struct stat st;

    char* path;

    for (int i = 1; i < argc; i++) {
        path = argv[i];
        
        if (stat(path, &st) < 0) { // get file metadata
            return -1;
        }

        
        if (S_ISDIR(st.st_mode) || is_execute_enabled(st.st_mode)) {
            // directory or a file with execution enabled for any permission group

            chmod(path, READ_PERMISSIONS | EXECUTE_PERMISSIONS);
        } else if (S_ISREG(st.st_mode)) {
            chmod(path, READ_PERMISSIONS);
        }
    }
}
