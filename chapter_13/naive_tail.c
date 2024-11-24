#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

#define DEFAULT_LINE_NUM 10

#define BUFF_SIZE 4096

static void
print_last_n_lines(const char *file_path, int n)
{
    FILE *file = fopen(file_path, "r");
    if (!file) {
        errExit("Error opening file");
    }

    if (fseek(file, 0, SEEK_END) != 0) { // seek to the end of the file
        errExit("Error seeking in file");
    }

    long file_pos = ftell(file);
    int line_cnt = 0;
    char c;

    // find position of n lines before the end
    while (file_pos > 0) {
        fseek(file, --file_pos, SEEK_SET);
        c = fgetc(file);

        if (c == '\n') {
            line_cnt++;
            if (line_cnt > n) break;
        }
    }

    long adjusted_pos = file_pos == 0 ?
        0 // reset position in case reached to beginning or
        : file_pos + 1; // skip the last newline otherwise

    fseek(file, adjusted_pos, SEEK_SET);

    // print the last n lines
    char buffer[BUFF_SIZE];
    while (fgets(buffer, sizeof(buffer), file)) {
        fputs(buffer, stdout);
    }

    fclose(file);
}

int
main(int argc, char *argv[])
{
    int opt;
    int n = DEFAULT_LINE_NUM;
    char *file_path = NULL;

    while ((opt = getopt(argc, argv, "n:")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg);
                if (n <= 0) {
                    errExit("Number of lines must be greater than 0.\n");
                }
                break;
            default:
                usageErr("Usage: %s [-n num] file\n", argv[0]);
        }
    }

    if (optind >= argc) {
        usageErr("%s [-n num] file\n", argv[0]);
    }

    file_path = argv[optind];

    print_last_n_lines(file_path, n);

    return 0;
}
