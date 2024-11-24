#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

#define DEFAULT_LINE_NUM 10

#define BUFF_SIZE 262144

static void
print_last_n_lines(const char *file_path, int n)
{
    FILE *file = fopen(file_path, "r");
    if (!file) {
        errExit("fopen");
    }

    if (fseek(file, 0, SEEK_END) != 0) { // seek to the end of the file
        errExit("fseek");
    }

    long file_pos = ftell(file);
    int line_cnt = 0;

    char *buffer = (char *) malloc(BUFF_SIZE);
    if (!buffer) {
        errExit("malloc");
    }

    // find position of n lines before the end
    size_t bytes_read;
    while (file_pos > 0 && line_cnt <= n) {
         // read BUFF_SIZE, or less in case the remaining is less than BUFF_SIZE
        size_t to_read = (file_pos < (long) BUFF_SIZE) ? file_pos : BUFF_SIZE;
        file_pos -= to_read;
        fseek(file, file_pos, SEEK_SET);

        bytes_read = fread(buffer, 1, to_read, file);

        // count new-lines within the buffer
        for (ssize_t i = bytes_read - 1; i >= 0; i--) {
            if (buffer[i] == '\n') {
                line_cnt++;
                if (line_cnt > n) {
                    file_pos += i; // set position to start of the last `n` lines
                    break;
                }
            }
        }
    }

    long adjusted_pos = file_pos == 0 ?
        0 // reset position in case reached to beginning or
        : file_pos + 1; // skip the last newline otherwise

    fseek(file, adjusted_pos, SEEK_SET);

    // print the last n lines
    char line[BUFF_SIZE];
    while (fgets(line, sizeof(line), file)) {
        fputs(line, stdout);
    }

    free(buffer);
    fclose(file);
}

int
main(int argc, char *argv[])
{
    int opt;
    int n = DEFAULT_LINE_NUM;
    char *file_path = NULL;

    // Parse command-line options
    while ((opt = getopt(argc, argv, "n:b:")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg);
                if (n <= 0) {
                    errExit("Number of lines must be greater than 0.");
                }
                break;
            default:
                errExit("Usage: %s [-n num] [-b buffer_size] file", argv[0]);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Error: File path is required.\n");
        fprintf(stderr, "Usage: %s [-n num] [-b buffer_size] file\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    file_path = argv[optind];

    print_last_n_lines(file_path, n);
    return 0;
}
