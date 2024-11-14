#include <stdio.h>
#include <pwd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <ctype.h>

#include "tlpi_hdr.h"

#define PROC_DIR "/proc"
#define STATUS_FILE "status"

static uid_t           /* Return UID corresponding to 'name', or -1 on error */
userIdFromName(const char *name)
{
    struct passwd *pwd;
    uid_t u;
    char *endptr;

    if (name == NULL || *name == '\0')  /* On NULL or empty string */
        return -1;                      /* return an error */

    u = strtol(name, &endptr, 10);      /* As a convenience to caller */
    if (*endptr == '\0')                /* allow a numeric string */
        return u;

    pwd = getpwnam(name);
    if (pwd == NULL)
        return -1;

    return pwd->pw_uid;
}


static char *
trim_whitespace(char *str) {
    char *start = str;
    char *end;

    // Move the start pointer to the first non-whitespace character
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    // If the string is all whitespace
    if (*start == '\0') {
        return start;
    }

    // Move the end pointer to the last non-whitespace character
    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    // Write the null terminator after the last non-whitespace character
    *(end + 1) = '\0';

    return start;
}


static void
process_proc_file(const char *path, uid_t userid, char *pid) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        // ignore
        return;
    }

    int ruid = -1;
    char line[256];
    char name[256];
    while (fgets(line, sizeof(line), file)) {
        char *key = strtok(line, ":");
        char *value = strtok(NULL, "\n");

        if (key && value) {
            if (strcmp(key, "Name") == 0) {
                strcpy(name, value);
            } else if (strcmp(key, "Uid") == 0) {
                sscanf(value, " %d", &ruid);
            }
        }
    }

    if (ruid == userid) {
        printf("%s: %s\n", pid, trim_whitespace(name));
    }

    fclose(file);
}


int
main(int argc, char* argv[])
{
    if (argc != 2) {
		fprintf(stderr, "Usage: %s username\n", argv[0]);
		exit(EXIT_FAILURE);
	}

    char *username = argv[1];
    uid_t userid = userIdFromName(username);
    
    DIR *procdir = opendir(PROC_DIR);
    if (procdir == NULL) {
        errExit("Unable to open the /proc folder");
    }

    struct dirent *entry;
    while ((entry = readdir(procdir))) {
        if (entry->d_type == DT_DIR) {
            int is_pid = 1;
            for (int i = 0; entry->d_name[i] != '\0'; i++) {
                if (!isdigit(entry->d_name[i])) {
                    is_pid = 0;
                    break;
                }
            }
            if (is_pid) {
                char status_path[275];
                snprintf(status_path, sizeof(status_path), "%s/%s/%s", PROC_DIR, entry->d_name, STATUS_FILE);
                process_proc_file(status_path, userid, entry->d_name);
            }
        }
    }
}
