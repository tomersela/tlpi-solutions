#include <time.h>
#include <fcntl.h>
#include <limits.h>

#include "tlpi_hdr.h"

#define MAX_RECORDS 1000000 // maximum of 6-digit numbers (0 to 999999)

int
main(int argc, char *argv[])
{
    if (argc != 3) {
        usageErr("Usage: %s <number_of_files> <directory>\n", argv[0]);
    }

    int number_of_files = atoi(argv[1]);
    const char *directory = argv[2];

    if (number_of_files <= 0 || number_of_files > MAX_RECORDS) {
        errExit("Number of files must be between 1 and %d.\n", MAX_RECORDS);
    }

    int *used_numbers = calloc(MAX_RECORDS, sizeof(int));
    if (!used_numbers) {
        errExit("calloc");
    }

    int *generated_numbers = malloc(number_of_files * sizeof(int));
    if (!generated_numbers) {
        errExit("malloc");
    }

    int *sorted_numbers = malloc(number_of_files * sizeof(int));
    if (!sorted_numbers) {
        errExit("malloc");
    }

    srand(time(NULL)); // seed random

    int generated_cnt = 0;
    while (generated_cnt < number_of_files) {
        int random_number = rand() % MAX_RECORDS; // generate a random number between 0 and 999999
        if (!used_numbers[random_number]) { // ensure unique numbers
            used_numbers[random_number] = 1;
            generated_numbers[generated_cnt++] = random_number;
        }
    }

    int sorted_idx = 0;
    for (int i = 0; i < MAX_RECORDS; i++) {
        if (used_numbers[i] > 0) {
            sorted_numbers[sorted_idx] = i;
            sorted_idx++;
        }
    }

    free(used_numbers); // no longer needed

    char file_path[_POSIX_PATH_MAX];
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start); // measure creation - start

    // create files
    for (int i = 0; i < number_of_files; i++) {
        snprintf(file_path, _POSIX_PATH_MAX, "%s/x%06d", directory, generated_numbers[i]);

        int fd = open(file_path, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            errExit("open");
        }

        if (write(fd, "x", 1) == -1) { // write 1 byte
            errExit("write");
        }
        close(fd);
    }
    
    free(generated_numbers);

    clock_gettime(CLOCK_MONOTONIC, &end);  // measure creation - end
    double creation_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Create: %.2f seconds\n", creation_time);

    clock_gettime(CLOCK_MONOTONIC, &start); // measure deletion - start

    // delete files in sorted order
    for (int i = 0; i < number_of_files; i++) {
        snprintf(file_path, _POSIX_PATH_MAX, "%s/x%06d", directory, sorted_numbers[i]);
        if (unlink(file_path) == -1) {
            errExit("unlink");
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end); // measure creation - end
    double deletion_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Delete: %.2f seconds\n", deletion_time);

    free(sorted_numbers);

    return 0;
}
