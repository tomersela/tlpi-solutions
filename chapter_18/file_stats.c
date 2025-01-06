#include <ftw.h>

#include "tlpi_hdr.h"

static int regular_cnt = 0, dir_cnt = 0, symlink_cnt = 0, socket_cnt = 0,
    fifo_cnt = 0, char_dev_cnt = 0, block_dev_cnt = 0, non_statable_cnt = 0;

static int
file_stats(const char *path, const struct stat *sb, int flag, struct FTW *ftwb)
{
    if (flag == FTW_NS) {
        non_statable_cnt++;
        return 0;
    }

    switch (sb->st_mode & S_IFMT) {
        case S_IFREG:
            regular_cnt++;
            break;
        case S_IFDIR:
            dir_cnt++;
            break;
        case S_IFCHR:
            char_dev_cnt++;
            break;
        case S_IFBLK:
            block_dev_cnt++;
            break;
        case S_IFLNK:
            symlink_cnt++;
            break;
        case S_IFIFO:
            fifo_cnt++;
            break;
        case S_IFSOCK:
            socket_cnt++;
            break;
    }

    return 0;
}

static void
print_stats(const char *msg, int cnt, int total_cnt)
{
    printf("%-15s   %6d %6.1f%%\n", msg, cnt, cnt * 100.0 / total_cnt);
}

int
main(int argc, char *argv[])
{
    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s <dir-path>\n", argv[0]);

    if (nftw(argv[1], &file_stats, 20, FTW_PHYS) == -1) // skip symlinks
        errExit("nftw");

    int total_cnt = regular_cnt + dir_cnt + symlink_cnt + socket_cnt +
                fifo_cnt + char_dev_cnt + block_dev_cnt + non_statable_cnt;

    if (total_cnt == 0) {
        printf("No files found\n");
    } else {
        printf("Total files:      %6d\n", total_cnt);
        print_stats("Regular:", regular_cnt, total_cnt);
        print_stats("Directory:", dir_cnt, total_cnt);
        print_stats("Char device:", char_dev_cnt, total_cnt);
        print_stats("Block device:", block_dev_cnt, total_cnt);
        print_stats("Symbolic link:", symlink_cnt, total_cnt);
        print_stats("FIFO:", fifo_cnt, total_cnt);
        print_stats("Socket:", socket_cnt, total_cnt);
        print_stats("Non-statable:", non_statable_cnt, total_cnt);
    }

    exit(EXIT_SUCCESS);
}
