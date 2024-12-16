#include <fcntl.h>
#include <linux/fs.h>
#include <sys/ioctl.h>

#include "tlpi_hdr.h"

static void
usage(const char *prog_name)
{
    usageErr("%s [+-=][flags] file\n"
        "Flags:\n"
        "    a  Append-only\n"
        "    A  No access time updates\n"
        "    c  Compressed\n"
        "    C  No copy-on-write\n"
        "    d  No dump\n"
        "    i  Immutable\n"
        "    j  Data journaling\n"
        "    s  Secure deletion\n"
        "    D  Directory sync\n"
        "    S  Synchronous writes\n"
        "    t  No tail-merging\n"
        "    T  Top-level directory\n"
        "    x  Direct access (DAX)\n"
        , prog_name);
}


static int
parse_flags(const char *flags_str)
{
    int flags = 0;
    for (const char *p = flags_str; *p; p++) {
        switch (*p) {
            case 'a': flags |= FS_APPEND_FL; break;        // Append-only
            case 'A': flags |= FS_NOATIME_FL; break;       // No access time updates
            case 'c': flags |= FS_COMPR_FL; break;         // Compress
            case 'C': flags |= FS_NOCOW_FL; break;         // No copy-on-write
            case 'd': flags |= FS_NODUMP_FL; break;        // No dump
            case 'j': flags |= FS_JOURNAL_DATA_FL; break;  // Journal data
            case 'u': flags |= FS_UNRM_FL; break;          // Undelete
            case 'i': flags |= FS_IMMUTABLE_FL; break;     // Immutable
            case 'D': flags |= FS_DIRSYNC_FL; break;       // Directory sync
            case 'S': flags |= FS_SYNC_FL; break;          // Synchronous writes
            case 't': flags |= FS_NOTAIL_FL; break;        // No tail-merging
            case 'T': flags |= FS_TOPDIR_FL; break;        // Top-level directory
            case 'x': flags |= FS_DAX_FL; break;           // Direct access (DAX)
            default:
                fprintf(stderr, "Unknown flag: %c\n", *p);
                exit(EXIT_FAILURE);
        }
    }
    return flags;
}


int
main(int argc, char *argv[])
{
    if (argc != 3) {
        usage(argv[0]);
    }

    char op = argv[1][0];
    if (op != '+' && op != '-' && op != '=') {
        usage(argv[0]);
    }

    const char *flags_str = &argv[1][1];
    int flags_to_modify = parse_flags(flags_str);

    const char *file = argv[2];
    int fd = open(file, O_RDONLY);
    if (fd < 0) {
        errExit("open");
    }

    int current_flags;
    if (ioctl(fd, FS_IOC_GETFLAGS, &current_flags) < 0) {
        errExit("ioctl(FS_IOC_GETFLAGS)");
    }

    if (op == '+') {
        current_flags |= flags_to_modify;
    } else if (op == '-') {
        current_flags &= ~flags_to_modify;
    } else if (op == '=') {
        current_flags = flags_to_modify;
    }

    if (ioctl(fd, FS_IOC_SETFLAGS, &current_flags) < 0) {
        errExit("ioctl(FS_IOC_SETFLAGS)");
    }

    close(fd);
    return EXIT_SUCCESS;
}
