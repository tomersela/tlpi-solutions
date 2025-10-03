#include <fcntl.h>
#include <sys/stat.h>

#include "tlpi_hdr.h"
#include "sendfile.h"


ssize_t us_sendfile(int out_fd, int in_fd, off_t *offset, size_t count);


static int
create_test_file(const char *filename, const char *content, size_t size)
{
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1) return -1;
    write(fd, content, size);
    close(fd);
    return 0;
}


static void
test_basic_copy(void)
{
    printf("[Test] Basic file copy (no offset):\n");

    create_test_file("test_input.txt", "Hello, World! This is a test file for sendfile.\n", 48);

    int in_fd = open("test_input.txt", O_RDONLY);
    if (in_fd == -1) errExit("open input");

    int out_fd = open("test_output1.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (out_fd == -1) errExit("open output");

    ssize_t result = us_sendfile(out_fd, in_fd, NULL, 48);
    printf("  Expected 48, got %ld - %s\n", result, (result == 48) ? "PASS" : "FAIL");

    close(in_fd);
    close(out_fd);

    unlink("test_input.txt");
    unlink("test_output1.txt");
    printf("  SUCCESS\n\n");
}


static void
test_with_offset(void)
{
    printf("[Test] Copy with offset:\n");

    create_test_file("test_input.txt", "Hello, World! This is a test file for sendfile.\n", 48);

    off_t offset = 7; // start from "World!"

    int in_fd = open("test_input.txt", O_RDONLY);
    if (in_fd == -1) errExit("open input");

    int out_fd = open("test_output2.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (out_fd == -1) errExit("open output");

    ssize_t result = us_sendfile(out_fd, in_fd, &offset, 20);
    printf("  Expected 20, got %ld - %s\n", result, (result == 20) ? "PASS" : "FAIL");
    printf("  Expected offset 27, got %ld - %s\n", offset, (offset == 27) ? "PASS" : "FAIL");

    close(in_fd);
    close(out_fd);

    unlink("test_input.txt");
    unlink("test_output2.txt");
    printf("  SUCCESS\n\n");
}


static void
test_partial_copy(void)
{
    printf("[Test] Partial copy (limit count):\n");

    create_test_file("test_input.txt", "Hello, World! This is a test file for sendfile.\n", 48);

    int in_fd = open("test_input.txt", O_RDONLY);
    if (in_fd == -1) errExit("open input");

    int out_fd = open("test_output3.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (out_fd == -1) errExit("open output");

    ssize_t result = us_sendfile(out_fd, in_fd, NULL, 12); // only first 12 bytes
    printf("  Expected 12, got %ld - %s\n", result, (result == 12) ? "PASS" : "FAIL");

    close(in_fd);
    close(out_fd);

    unlink("test_input.txt");
    unlink("test_output3.txt");
    printf("  SUCCESS\n\n");
}


static void
test_file_position_preservation(void)
{
    printf("[Test] File position preservation with offset:\n");

    create_test_file("test_input.txt", "Hello, World! This is a test file for sendfile.\n", 48);

    off_t offset = 10;

    int in_fd = open("test_input.txt", O_RDONLY);
    if (in_fd == -1) errExit("open input");

    // move to position 5
    lseek(in_fd, 5, SEEK_SET);
    off_t pos_before = lseek(in_fd, 0, SEEK_CUR);

    int out_fd = open("test_output4.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (out_fd == -1) errExit("open output");

    // use us_sendfile with offset - should NOT change file position
    ssize_t result = us_sendfile(out_fd, in_fd, &offset, 10);

    off_t pos_after = lseek(in_fd, 0, SEEK_CUR);
    printf("  Expected 10, got %ld - %s\n", result, (result == 10) ? "PASS" : "FAIL");
    printf("  Position unchanged: %s\n", (pos_before == pos_after) ? "PASS" : "FAIL");

    close(in_fd);
    close(out_fd);

    unlink("test_input.txt");
    unlink("test_output4.txt");
    printf("  SUCCESS\n\n");
}


static void
test_large_file(void)
{
    printf("[Test] Large file (multiple buffer reads):\n");

    // create a larger test file
    char buffer[8192];
    for (int i = 0; i < 8192; i++) {
        buffer[i] = 'A' + (i % 26);
    }

    int in_fd = open("test_large.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (in_fd == -1) errExit("open large input");
    write(in_fd, buffer, 8192);
    write(in_fd, buffer, 8192);
    close(in_fd);

    in_fd = open("test_large.txt", O_RDONLY);
    if (in_fd == -1) errExit("open large input for reading");

    int out_fd = open("test_output5.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (out_fd == -1) errExit("open large output");

    ssize_t result = us_sendfile(out_fd, in_fd, NULL, 16384);
    printf("  Expected 16384, got %ld - %s\n", result, (result == 16384) ? "PASS" : "FAIL");

    close(in_fd);
    close(out_fd);

    unlink("test_large.txt");
    unlink("test_output5.txt");
    printf("  SUCCESS\n\n");
}


int
main(int argc, char *argv[])
{
    test_basic_copy();
    test_with_offset();
    test_partial_copy();
    test_file_position_preservation();
    test_large_file();

    return 0;
}
