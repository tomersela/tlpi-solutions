#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>

#include "tlpi_hdr.h"

#define BUFF_SIZE 1024

struct ProcList;

typedef struct {
    int pid;
    char *name;
    struct ProcList *children;
} Process;

struct ProcList {
    Process *proc;
    struct ProcList *next;
};

typedef struct ProcList ProcList;

static long
get_max_pid()
{
    long maxpid;
    FILE* f = fopen("/proc/sys/kernel/pid_max", "r");
    if(f == NULL)
        errExit("fopen");
    if(fscanf(f, "%ld", &maxpid) != 1)
        errExit("fscanf");

    return maxpid;
}

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
parse_proc_file(char *pid_dir, Process *processes)
{
    char path[_POSIX_PATH_MAX+20];
    snprintf(path, _POSIX_PATH_MAX, "/proc/%s/status", pid_dir);
    FILE* f = fopen(path, "r");
    if (f == NULL) {
        return; // skip errors
    }

    char buff[BUFF_SIZE];
    char proc_name[BUFF_SIZE] = {0};
    int pid = 0;
    int ppid = 0;
    int parse_cnt = 0;
    while (fgets(buff, BUFF_SIZE, f)) {
        if (strncmp(buff, "Name:", 5) == 0) {
            sscanf(buff + 5, " %s", proc_name), parse_cnt++;
        } else if (strncmp(buff, "Pid:",  4) == 0) {
            sscanf(buff + 4, " %d", &pid), parse_cnt++;
        } else if (strncmp(buff, "PPid:", 5) == 0) {
            sscanf(buff + 5, " %d", &ppid), parse_cnt++;
        }
    }
    
    if (parse_cnt == 3) {
        Process *proc = &processes[pid];
        proc->pid = pid;
        int max_name_len = strlen(proc_name) + 20;
        proc->name = malloc(max_name_len);
        snprintf(proc->name, max_name_len, "%s", proc_name);

        // update parent with child
        Process *parent_proc = &processes[ppid];
        ProcList *new_child = (ProcList*) malloc(sizeof(ProcList));
        new_child->proc = proc;
        if (parent_proc->children == NULL) {
            parent_proc->children = new_child;
        } else {
            new_child->next = parent_proc->children;
            parent_proc->children = new_child;
        }
    }
}

static Process *
load_processes()
{
    long maxpid = get_max_pid();
    Process *processes = (Process*) malloc(maxpid * sizeof(Process));

    DIR *procdir = opendir("/proc");
    if (procdir == NULL) {
        errExit("Unable to open the /proc folder");
    }

    struct dirent *dir;
    while ((dir = readdir(procdir))) {
        if (dir->d_type == DT_DIR && is_num(dir->d_name)) {
            // Only numerical named directories

            parse_proc_file(dir->d_name, processes);
        }
    }

    return processes;
}

static void
append_spaces(char *buffer, size_t buffer_size, int num_spaces) {
    size_t current_length = strlen(buffer);

    size_t available_space = buffer_size - current_length - 1;
    size_t spaces_to_add = (num_spaces <= available_space) ? num_spaces : available_space;

    for (size_t i = 0; i < spaces_to_add; i++) {
        buffer[current_length + i] = ' ';
    }

    buffer[current_length + spaces_to_add] = '\0';
}

static void
draw_tree(Process *process, char *prefix)
{
    char display_name[BUFF_SIZE];
    snprintf(display_name, BUFF_SIZE, "%s(%d)", process->name, process->pid);
    printf("%s", display_name);

    int name_len = strlen(display_name);

    char new_prefix[BUFF_SIZE];
    snprintf(new_prefix, BUFF_SIZE, "%s", prefix); // copy current prefix
    append_spaces(new_prefix, BUFF_SIZE, name_len); // add margin by proc name length

    ProcList *child = process->children;
    if (child == NULL) { // no children
        return;
    }
    
    char child_prefix[BUFF_SIZE];
    
    // draw the first child node
    ProcList *next_child;

    next_child = child->next;
    char* branch_symbol = next_child == NULL ? "───" : "─┬─";
    printf("%s", branch_symbol);
    char* child_prefix_addition = next_child == NULL ? "   " : " │ ";
    snprintf(child_prefix, BUFF_SIZE - strlen(new_prefix) - 3, "%s%s",
            new_prefix, child_prefix_addition);
    draw_tree(child->proc, child_prefix);
    child = next_child;

    // draw the rest of the nodes
    while (child != NULL) {
        printf("\n");
        next_child = child->next;
        char* branch_symbol = next_child == NULL ? " └─" : " ├─";
        printf("%s%s", new_prefix, branch_symbol);
        child_prefix_addition = next_child == NULL ? "   " : " │ ";
        snprintf(child_prefix, BUFF_SIZE - strlen(new_prefix) - 3, "%s%s",
            new_prefix, child_prefix_addition);
        draw_tree(child->proc, child_prefix);
        child = next_child;
    }
}

int
main(int argc, char* argv[])
{
    Process *processes = load_processes();
    draw_tree(&processes[1], "");
    printf("\n");
}
