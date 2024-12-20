#include <sys/xattr.h>

#include "tlpi_hdr.h"


static void
usage(const char *prog_name)
{
    usageErr("%s -n <ea_name> [-v <ea_value>] [-x] <filename>\n", prog_name);
}

int
main(int argc, char *argv[])
{
    const char *filename = NULL;
    const char *ea_name = NULL;
    const char *ea_value = "";
    int delete_flag = 0;
    int opt;

    while ((opt = getopt(argc, argv, "n:v:x:")) != -1) {
        switch (opt) {
            case 'n':
                ea_name = optarg;
                break;
            case 'v':
                ea_value = optarg;
                break;
            case 'x':
                ea_name = optarg;
                delete_flag = 1;
                break;
            default:
                usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (optind < argc) {
        filename = argv[optind];
    }

    if (!filename || !ea_name) {
        fprintf(stderr, "Error: Both -n <ea_name> and <filename> are required.\n");
        usage(argv[0]);
    }

    if (delete_flag) {
        if (removexattr(filename, ea_name) == -1) {
            errExit("removexattr");
            return EXIT_FAILURE;
        }
    } else {
        if (setxattr(filename, ea_name, ea_value, strlen(ea_value), 0) == -1) {
            errExit("setxattr");
        }
    }

    return EXIT_SUCCESS;
}
