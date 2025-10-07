#include <termios.h>
#include <dirent.h>
#include <sys/stat.h>

#include "tlpi_hdr.h"
#include "isatty.h"
#include "ttyname.h"


static char *
search_dir_for_tty(const char *dir_path, const dev_t target_dev)
{
    static char tty_path[PATH_MAX];
    DIR *dir;
    struct dirent *entry;
    struct stat sb;
    char full_path[PATH_MAX];
    
    dir = opendir(dir_path);
    if (dir == NULL)
        return NULL;
    
    while ((entry = readdir(dir)) != NULL) {
        // skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
            
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);
        
        if (stat(full_path, &sb) == -1)
            continue;
            
        // check for a character device with matching device number
        if (S_ISCHR(sb.st_mode) && sb.st_rdev == target_dev) {
            closedir(dir);
            strcpy(tty_path, full_path);
            return tty_path;
        }
    }
    
    closedir(dir);
    return NULL;
}


char
*ttyname(int fd)
{
    struct stat sb;
    char *result;
    
    // check if fd is a terminal
    if (!isatty(fd))
        return NULL;
        
    // get device number of fd
    if (fstat(fd, &sb) == -1)
        return NULL;
        
    // search /dev first
    result = search_dir_for_tty("/dev", sb.st_rdev);
    if (result != NULL)
        return result;
        
    // search /dev/pts for pseudo-terminals
    result = search_dir_for_tty("/dev/pts", sb.st_rdev);

    return result;
}
