#define _GNU_SOURCE
#include <syslog.h>
#include <getopt.h>

#include "tlpi_hdr.h"


static void
usage(const char *progname)
{
    usageErr("%s [-p|--priority priority] message\n"
        "  -p, --priority   Priority as facility.level or level only (default: user.notice)\n", progname);
}


static int
parse_facility(const char *facility_str)
{
    if (strcmp(facility_str, "auth") == 0) return LOG_AUTH;
    if (strcmp(facility_str, "authpriv") == 0) return LOG_AUTHPRIV;
    if (strcmp(facility_str, "cron") == 0) return LOG_CRON;
    if (strcmp(facility_str, "daemon") == 0) return LOG_DAEMON;
    if (strcmp(facility_str, "kern") == 0) return LOG_KERN;
    if (strcmp(facility_str, "lpr") == 0) return LOG_LPR;
    if (strcmp(facility_str, "mail") == 0) return LOG_MAIL;
    if (strcmp(facility_str, "news") == 0) return LOG_NEWS;
    if (strcmp(facility_str, "syslog") == 0) return LOG_SYSLOG;
    if (strcmp(facility_str, "user") == 0) return LOG_USER;
    if (strcmp(facility_str, "uucp") == 0) return LOG_UUCP;
    if (strcmp(facility_str, "local0") == 0) return LOG_LOCAL0;
    if (strcmp(facility_str, "local1") == 0) return LOG_LOCAL1;
    if (strcmp(facility_str, "local2") == 0) return LOG_LOCAL2;
    if (strcmp(facility_str, "local3") == 0) return LOG_LOCAL3;
    if (strcmp(facility_str, "local4") == 0) return LOG_LOCAL4;
    if (strcmp(facility_str, "local5") == 0) return LOG_LOCAL5;
    if (strcmp(facility_str, "local6") == 0) return LOG_LOCAL6;
    if (strcmp(facility_str, "local7") == 0) return LOG_LOCAL7;

    fprintf(stderr, "Unknown facility: %s\n", facility_str);
    exit(EXIT_FAILURE);
}


static int
parse_level(const char *level_str)
{
    if (strcmp(level_str, "emerg") == 0) return LOG_EMERG;
    if (strcmp(level_str, "alert") == 0) return LOG_ALERT;
    if (strcmp(level_str, "crit") == 0) return LOG_CRIT;
    if (strcmp(level_str, "err") == 0) return LOG_ERR;
    if (strcmp(level_str, "warning") == 0) return LOG_WARNING;
    if (strcmp(level_str, "notice") == 0) return LOG_NOTICE;
    if (strcmp(level_str, "info") == 0) return LOG_INFO;
    if (strcmp(level_str, "debug") == 0) return LOG_DEBUG;

    fprintf(stderr, "Unknown level: %s\n", level_str);
    exit(EXIT_FAILURE);
}


static int
parse_priority(const char *input) {
    char *input_copy;
    int facility, level;

    if ((input_copy = strdup(input)) == NULL) {
        errExit("strdup");
    }

    char *dot = strchr(input_copy, '.');

    if (dot) { // facility.level
        *dot = '\0';
        const char *facility_part = input_copy;
        const char *level_part = dot + 1;
    
        level = parse_level(level_part);
        facility = parse_facility(facility_part);
        
    } else { // only level provided
        level = parse_level(input_copy);
        facility = LOG_USER;
    }

    free(input_copy);
    return facility | level;
}


int main(int argc, char *argv[]) {
    int priority = LOG_USER | LOG_NOTICE;
    char *priority_arg = NULL;

    static struct option long_options[] = {
        {"priority", required_argument, 0, 'p'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "p:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'p':
                priority_arg = optarg;
                break;
            default:
                usage(argv[0]);
        }
    }

    if (optind >= argc) {
        usage(argv[0]);
    }

    if (priority_arg) {
        priority = parse_priority(priority_arg);
    }

    // concat all remaining arguments into one message string
    size_t message_len = 0;
    for (int i = optind; i < argc; i++) {
        message_len += strlen(argv[i]) + 1;
    }

    char *message = malloc(message_len);
    if (message == NULL)
        errExit("malloc");
    
    message[0] = '\0';
    for (int i = optind; i < argc; i++) {
        strcat(message, argv[i]);
        if (i < argc - 1) {
            strcat(message, " ");
        }
    }

    openlog("mylogger", LOG_PID | LOG_CONS, 0);
    syslog(priority, "%s", message);
    closelog();
    free(message);

    exit(EXIT_SUCCESS);
}
