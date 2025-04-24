#include <sched.h>
#include <getopt.h>
#include <sys/resource.h>

#include "tlpi_hdr.h"

#define ADJUSTMENT_ARG_OPTION 1000

static void
print_usage(const char *progname)
{
    printf("Usage: %s [-n number] [--adjustment number] [-h|--help]\n", progname);
    printf("\nOptions:\n");
    printf("  -n NUMBER             Set adjustment using -n\n");
    printf("  --adjustment=NUMBER   Set adjustment using --adjustment\n");
    printf("  -h, --help            Show this help message and exit\n");
}


static int
clamp(int x)
{
    return x < -20 ? -20 : (x > 19 ? 19 : x);
}


static int
get_nice_value()
{
    errno = 0;
    int nice_value = getpriority(PRIO_PROCESS, 0);
    if (nice_value == -1 && errno != 0)
        errExit("getpriority");
    return nice_value;
}


static int
parse_adjustment_value(char *arg)
{
    char *endptr;

    int adjustment_value = strtol(optarg, &endptr, 0);
    if (errno != 0)
        errExit("strtol");

    if (*endptr != '\0') {
        fprintf(stderr, "nice: invalid adjustment '%s'\n", optarg);
        exit(EXIT_FAILURE);
    }

    return adjustment_value;
}


int
main(int argc, char *argv[])
{
    int opt;
    int option_index = 0;
    int adjustment_value = 10;

    if (argc == 1) {
        printf("%d\n", get_nice_value());
        exit(EXIT_SUCCESS);
    }

    static struct option long_options[] = {
        {"adjustment", required_argument, NULL, ADJUSTMENT_ARG_OPTION},
        {"help",       no_argument,       NULL, 'h'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "n:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'n':
                adjustment_value = parse_adjustment_value(optarg);
                break;
            case ADJUSTMENT_ARG_OPTION: {
                adjustment_value = parse_adjustment_value(optarg);
                break;
            }
            case 'h':
                print_usage(argv[0]);
                exit(EXIT_SUCCESS);
            default:
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (argc < optind + 1) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    int adjusted = clamp(get_nice_value() + adjustment_value);

    if (setpriority(PRIO_PROCESS, 0, adjusted) == -1) {
        if (errno = EACCES) {
            fprintf(stderr, "nice: cannot set niceness: Permission denied\n");
        } else {
            errExit("setpriority");
        }
    }

    execvp(argv[optind], &argv[optind]);

    exit(EXIT_SUCCESS);
}
