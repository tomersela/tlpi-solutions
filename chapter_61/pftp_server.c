#include <signal.h>
#include "pftp.h"

// simple global state for single client
static int server_fd = -1;
static int normal_fd = -1;
static int priority_fd = -1;
static char cookie[MAX_COOKIE_LEN + 1];
static FILE *transfer_file = NULL;

static void cleanup_and_exit(int sig);
static void setup_server(int port);
static void handle_new_connection(void);
static void handle_normal_data(void);
static void handle_priority_data(void);
static void continue_file_transfer(void);
static void handle_connect_command(const char *buffer);
static void handle_transfer_command(const char *filename);

int
main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s port\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, cleanup_and_exit);
    mkdir(FILES_DIR, 0755);
    setup_server(atoi(argv[1]));

    // main loop
    for (;;) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);

        int maxfd = server_fd;
        if (normal_fd != -1) { FD_SET(normal_fd, &readfds); maxfd = normal_fd; }
        if (priority_fd != -1) { FD_SET(priority_fd, &readfds); if (priority_fd > maxfd) maxfd = priority_fd; }

        // no timeout when transferring - go full speed
        struct timeval timeout = {0, 0}, *timeout_ptr = NULL;
        if (transfer_file) timeout_ptr = &timeout;

        int ready = select(maxfd + 1, &readfds, NULL, NULL, timeout_ptr);
        if (ready == -1 && errno != EINTR) break;

        if (ready == 0 && transfer_file) {
            continue_file_transfer();
            continue;
        }

        if (FD_ISSET(server_fd, &readfds)) handle_new_connection();
        if (normal_fd != -1 && FD_ISSET(normal_fd, &readfds)) handle_normal_data();
        if (priority_fd != -1 && FD_ISSET(priority_fd, &readfds)) handle_priority_data();
    }

    cleanup_and_exit(0);
    return 0;
}

static void
setup_server(int port)
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *) &addr, sizeof(addr));
    listen(server_fd, 1);

    printf("[server] pftp server listening on port %d\n", port);
}

static void
handle_new_connection(void)
{
    if (normal_fd == -1) {
        normal_fd = accept(server_fd, NULL, NULL);
        printf("[server] client connected\n");
    } else {
        int fd = accept(server_fd, NULL, NULL);
        close(fd); // reject - already have client
    }
}

static void
handle_normal_data(void)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes = recv(normal_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
        close(normal_fd); normal_fd = -1;
        if (priority_fd != -1) { close(priority_fd); priority_fd = -1; }
        if (transfer_file) { fclose(transfer_file); transfer_file = NULL; }
        printf("[server] client disconnected\n");
        return;
    }

    buffer[bytes] = '\0';
    if (buffer[bytes-1] == '\n') buffer[bytes-1] = '\0';

    if (strncmp(buffer, CMD_CONNECT, strlen(CMD_CONNECT)) == 0) {
        handle_connect_command(buffer);
    } else if (strncmp(buffer, CMD_TRANSFER, strlen(CMD_TRANSFER)) == 0) {
        const char *filename = buffer + strlen(CMD_TRANSFER);
        handle_transfer_command(filename);
    }
}

static void
handle_connect_command(const char *buffer)
{
    char recv_cookie[64], port_str[16];
    if (!parse_connect_command(buffer, recv_cookie, port_str)) return;

    strcpy(cookie, recv_cookie);
    int client_port = atoi(port_str);

    // get client IP and connect back
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    getpeername(normal_fd, (struct sockaddr*)&client_addr, &len);
    client_addr.sin_port = htons(client_port);

    priority_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(priority_fd, (struct sockaddr*)&client_addr, sizeof(client_addr)) == 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "%s%s\n", CMD_COOKIE, cookie);
        send(priority_fd, msg, strlen(msg), 0);
        send(normal_fd, MSG_OK, strlen(MSG_OK), 0);
        printf("[server] priority connection established\n");
    } else {
        close(priority_fd); priority_fd = -1;
        send(normal_fd, ERR_CANNOT_ESTABLISH_PRIORITY, strlen(ERR_CANNOT_ESTABLISH_PRIORITY), 0);
    }
}

static void
handle_transfer_command(const char *filename)
{
    if (!validate_filename(filename)) {
        send(normal_fd, ERR_INVALID_FILENAME, strlen(ERR_INVALID_FILENAME), 0);
        return;
    }

    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", FILES_DIR, filename);
    transfer_file = fopen(filepath, "rb");
    if (!transfer_file) {
        send(normal_fd, ERR_FILE_NOT_FOUND, strlen(ERR_FILE_NOT_FOUND), 0);
        return;
    }

    struct stat st;
    if (stat(filepath, &st) == 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "%s%ld\n", RESP_SIZE, st.st_size);
        send(normal_fd, msg, strlen(msg), 0);
        printf("[server] starting transfer: %s (%ld bytes)\n", filename, st.st_size);
    } else {
        fclose(transfer_file);
        transfer_file = NULL;
        send(normal_fd, ERR_CANNOT_DETERMINE_SIZE, strlen(ERR_CANNOT_DETERMINE_SIZE), 0);
    }
}

static void
continue_file_transfer(void)
{
    char buffer[BUFFER_SIZE];
    size_t bytes = fread(buffer, 1, sizeof(buffer), transfer_file);
    if (bytes == 0) {
        // end of file - just close, no message needed
        fclose(transfer_file);
        transfer_file = NULL;
        printf("[server] transfer completed\n");
    } else {
        send(normal_fd, buffer, bytes, 0);
    }
}

static void
handle_priority_data(void)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes = recv(priority_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
        close(priority_fd); priority_fd = -1;
        return;
    }

    buffer[bytes] = '\0';
    if (buffer[bytes-1] == '\n') buffer[bytes-1] = '\0';

    if (strcmp(buffer, CMD_VERIFIED) == 0) {
        printf("[server] priority connection verified\n");
    } else if (strcmp(buffer, CMD_RESET) == 0) {
        if (transfer_file) {
            fclose(transfer_file);
            transfer_file = NULL;
            send(normal_fd, MSG_RESET_CANCELLED, strlen(MSG_RESET_CANCELLED), 0);
            send(priority_fd, MSG_RESET_OK, strlen(MSG_RESET_OK), 0);
            printf("[server] transfer reset\n");
        }
    }
}

static void
cleanup_and_exit(int sig)
{
    if (transfer_file) fclose(transfer_file);
    if (normal_fd != -1) close(normal_fd);
    if (priority_fd != -1) close(priority_fd);
    if (server_fd != -1) close(server_fd);
    exit(0);
}
