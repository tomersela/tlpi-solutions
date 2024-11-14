#include <dirent.h>

#include "tlpi_hdr.h"

#define BUFF_SIZE 1024

static Boolean
is_num(const char *str)
{
    for (const char *c = str; *c != '\0'; c++) {
        if (!('0' <= *c && *c <= '9')) {
            return false;
        }
    }
    return true;
}

static void
print_proc_name(char* fd_dir)
{
    char path[_POSIX_PATH_MAX+20];

    snprintf(path, _POSIX_PATH_MAX, "/proc/%s/status", fd_dir);
    FILE* f = fopen(path, "r");
    if (f == NULL) {
        return; // skip errors
    }

    char buff[BUFF_SIZE];
    char proc_name[BUFF_SIZE] = {0};
    int pid = 0;
    int parse_cnt = 0;
    while (fgets(buff, BUFF_SIZE, f)) {
        if (strncmp(buff, "Name:", 5) == 0) {
            sscanf(buff + 5, " %s", proc_name), parse_cnt++;
        } else if (strncmp(buff, "Pid:",  4) == 0) {
            sscanf(buff + 4, " %d", &pid), parse_cnt++;
        }


        if (parse_cnt == 2) {
            printf("%s (%d)\n", proc_name, pid);
            return;
        }
    }
}

static void
iterate_fd_dir(char* pid_dir, char* file_path)
{
    char path[_POSIX_PATH_MAX + 20];
    char link_path_buff[_POSIX_PATH_MAX + 20];
    snprintf(path, _POSIX_PATH_MAX, "/proc/%s/fd", pid_dir);
    DIR *fd_dir = opendir(path);
    if (fd_dir == NULL) {
        if (errno == EACCES) {
            errExit("Access denied to folder %s", path);
        }
        printf("%d\n", errno);
        return; // skip
    }

    struct dirent *entry;
    while ((entry = readdir(fd_dir))) {
        if (entry->d_type == DT_LNK && is_num(entry->d_name)) {
            // Only numeric symink files

            snprintf(path, _POSIX_PATH_MAX + 20, "/proc/%s/fd/%s", pid_dir, entry->d_name);
            int link_len = readlink(path, link_path_buff, _POSIX_PATH_MAX);
            if (link_len == -1 || link_len >= _POSIX_PATH_MAX) {
                continue; // skip
            }
            link_path_buff[link_len] = '\0';
            
            if (!strcmp(file_path, link_path_buff)) {
                print_proc_name(pid_dir);
            }
        }
    }
}

int
main(int argc, char* argv[])
{
    if (argc != 2) {
		fprintf(stderr, "Usage: %s username\n", argv[0]);
		exit(EXIT_FAILURE);
	}

    char* file_path = argv[1];

    DIR *procdir = opendir("/proc");
    if (procdir == NULL) {
        errExit("Unable to open the /proc folder");
    }

    struct dirent *dir;
    while ((dir = readdir(procdir))) {
        if (dir->d_type == DT_DIR && is_num(dir->d_name)) {
            // Only numerical named directories

            iterate_fd_dir(dir->d_name, file_path);
        }
    }

    printf("\n");
}
