#include <ftw.h>
#include <sys/inotify.h>
#include <limits.h>
#include <string.h>

#include "tlpi_hdr.h"


#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

#define IN_FLAGS (IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MOVE | IN_DONT_FOLLOW | IN_MOVE_SELF)


static int inotify_fd;

struct path_info {
    int is_dir;
    char path[PATH_MAX];
};

static struct path_info **path_info_by_wd;

static int moved_from_cookie;
static int moved_from_parent_wd;
static char moved_from_path[PATH_MAX];
static int moved_to_cookie;
static int moved_to_parent_wd;
static char moved_to_path[PATH_MAX];

static int delete_wd;


static void
concat_path(char *target, char *path, char *sub_path)
{
    strncpy(target, path, PATH_MAX);
    strncat(target, "/", PATH_MAX - 1);
    strncat(target, sub_path, PATH_MAX - strlen(target));
}

static struct path_info *
get_path_info(struct inotify_event *event)
{
    return path_info_by_wd[event->wd];
}

static char *
get_path(struct inotify_event *event)
{
    return get_path_info(event)->path;
}


static void
compose_path(char *path, struct inotify_event *event)
{
    concat_path(path, get_path(event), event->name);
}


static void
add_watch(const char *path, int is_dir)
{
    int wd = inotify_add_watch(inotify_fd, path, IN_FLAGS);
    if (wd == -1)
        errExit("inotify_add_watch");
    
    struct path_info *info = (struct path_info *) malloc(sizeof(struct path_info));
    info->is_dir = is_dir;
    strncpy(info->path, path, PATH_MAX);

    path_info_by_wd[wd] = info;
}


static void
remove_watch(int wd)
{
    struct path_info *info = path_info_by_wd[wd];
    if (info == NULL)
        return;
    free(info);
    path_info_by_wd[wd] = NULL;
}


static int
watch_files_recursively(const char *path, const struct stat *sb, int flag, struct FTW *ftwb)
{
    if (flag == FTW_NS) {
        fprintf(stderr, "Failed to fetch stats for %s", path);
        return 0;
    }

    add_watch(path, S_ISDIR(sb->st_mode));

    return 0;
}

static void
handle_event(struct inotify_event *event)
{
    char path[PATH_MAX];

    if (event->mask & IN_CREATE) {
        compose_path(path, event);
        if (event->mask & IN_ISDIR) {
            printf("New directory created - %s\n", path);
        } else {
            printf("New file created - %s\n", path);
        }
        add_watch(path, event->mask & IN_ISDIR);

    } else if (event->mask & IN_MOVED_FROM) {
        compose_path(path, event);

        moved_from_cookie = event->cookie; // save cookie
        moved_from_parent_wd = event->wd; // save parent identifier (wd)
        strncpy(moved_from_path, path, PATH_MAX);
        
        // Note: removal of the watch is done in the handling of  IN_MOVE_SELF

    } else if (event->mask & IN_MOVED_TO) {
        compose_path(path, event);

        moved_to_cookie = event->cookie; // save cookie
        moved_to_parent_wd = event->wd; // save parent identifier (wd)
        strncpy(moved_to_path, path, PATH_MAX);
        
        if (event->cookie != moved_from_cookie) { // no correlation with a IN_MOVED_FROM event
            // file/dir moved from outside
            add_watch(path, event->mask & IN_ISDIR);
        }
        
    } else if (event->mask & IN_MOVE_SELF) {
        // this event is thrown after IN_MOVED_FROM & IN_MOVED_TO events.
        
        if (moved_from_cookie == moved_to_cookie) { // move from-to correlation!
            if ((moved_from_parent_wd) == moved_to_parent_wd) { // same parent directory -> rename
                printf("%s renamed to %s\n", moved_from_path, moved_to_path);
            } else {
                printf("%s moved to %s\n", moved_from_path, moved_to_path);
            }
        } else {
            if (moved_from_cookie == 0) { // IN_MOVED_TO occured without IN_MOVED_FROM after it
                printf("%s moved into the watched path\n", moved_to_path);
            } else {
                printf("%s moved out of the watched path\n", get_path(event));
                remove_watch(event->wd);
            }
        }

        // clear
        moved_from_cookie = 0;
        moved_from_parent_wd = 0;


    } else if (event->mask & IN_DELETE_SELF) {
        struct path_info *info = get_path_info(event);

        if (info->is_dir) {
            printf("Directory %s removed\n", info->path);
        } else {
            printf("File %s removed\n", info->path);
        }

        remove_watch(event->wd);
        delete_wd = event->wd;

    } else if (event->mask & IN_IGNORED) {
        if (delete_wd != 0 && event->wd != delete_wd) {
            if (path_info_by_wd[event->wd]) {
                printf("Watch for %s removed by the operating system\n", get_path(event));
            }
            remove_watch(event->wd);
        }

    } else if (event->mask & IN_UNMOUNT) {
        printf("Stopped watching %s since containing drive unmounted\n", get_path(event));
        remove_watch(event->wd);

    }
}


int
main(int argc, char* argv[])
{
    int max_watches;
    char buf[BUF_LEN];
    struct inotify_event *event;
    ssize_t event_read_cnt;
    char *ptr;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s <dir-path>\n", argv[0]);

    // get max user watches
    FILE* f = fopen("/proc/sys/fs/inotify/max_user_watches", "r");
    fscanf(f,"%d", &max_watches);
    path_info_by_wd = calloc(max_watches, sizeof(struct path_info *)); // initialize lookup table

    inotify_fd = inotify_init();

    if (nftw(argv[1], &watch_files_recursively, 20, 0) == -1)
        errExit("nftw");

    for (;;) {
        event_read_cnt = read(inotify_fd, buf, BUF_LEN);

        if (event_read_cnt == 0)
            fatal("read() from inotify fd returned 0!");
        
        if (event_read_cnt == -1)
            errExit("read");
        
        for (ptr = buf; ptr < buf + event_read_cnt; ) {
            event = (struct inotify_event *) ptr;

            handle_event(event);

            ptr += sizeof(struct inotify_event) + event->len;
        }
    }

    return EXIT_SUCCESS;
}
