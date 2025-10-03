#include <signal.h>
#include <netdb.h>
#include "pftp.h"

// simple global state
static int normal_fd = -1;
static int priority_fd = -1;
static int listening_fd = -1;
static char cookie[MAX_COOKIE_LEN + 1];
static char filename[MAX_FILENAME_LEN + 1];
static FILE *output_file = NULL;
static long file_size = 0;
static long bytes_received = 0;

static void cleanup_and_exit(int sig);
static void setup_listening_socket(void);
static void connect_to_server(const char *host, int port);
static void send_connect_command(void);
static void accept_priority_connection(void);
static void verify_cookie(void);
static void request_file_transfer(void);
static void handle_server_data(void);
static void handle_priority_data(void);
static void print_progress(void);

int
main(int argc, char *argv[])
{
    if (argc != 4) {
        printf("usage: %s <server_host> <server_port> <filename>\n", argv[0]);
        printf("example: %s localhost 8080 medium.dat\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, cleanup_and_exit);

    strncpy(filename, argv[3], sizeof(filename) - 1);
    if (!validate_filename(filename)) {
        fprintf(stderr, "invalid filename: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    printf("[client] pftp client - downloading %s from %s:%s\n", filename, argv[1], argv[2]);

    // establish connections
    setup_listening_socket();
    connect_to_server(argv[1], atoi(argv[2]));
    send_connect_command();
    accept_priority_connection();
    verify_cookie();

    printf("[client] connected! starting file transfer...\n");
    request_file_transfer();

    // simple event loop - wait for file transfer to complete
    int transfer_active = 1;
    while (transfer_active) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(normal_fd, &readfds);
        FD_SET(priority_fd, &readfds);

        int maxfd = (normal_fd > priority_fd) ? normal_fd : priority_fd;
        struct timeval timeout = {1, 0}; // 1 second for progress updates

        int ready = select(maxfd + 1, &readfds, NULL, NULL, &timeout);
        if (ready == -1 && errno != EINTR) break;

        if (ready == 0) {
            print_progress();
            continue;
        }

        if (FD_ISSET(normal_fd, &readfds)) {
            handle_server_data();
            // check if transfer completed
            if (output_file == NULL) {
                transfer_active = 0;
            }
        }
        if (FD_ISSET(priority_fd, &readfds)) handle_priority_data();
    }

    printf("[client] transfer completed successfully!\n");
    cleanup_and_exit(0);
    return 0;
}

static void
setup_listening_socket(void)
{
    listening_fd = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(listening_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(0); // ephemeral port

    bind(listening_fd, (struct sockaddr *) &addr, sizeof(addr));
    listen(listening_fd, 1);

    // get assigned port
    socklen_t len = sizeof(addr);
    getsockname(listening_fd, (struct sockaddr *) &addr, &len);
    printf("[client] listening on ephemeral port %d\n", ntohs(addr.sin_port));
}

static void
connect_to_server(const char *host, int port)
{
    normal_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    // try IP address first, then hostname
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
        struct hostent *he = gethostbyname(host);
        if (!he) {
            fprintf(stderr, "hostname resolution failed: %s\n", host);
            exit(EXIT_FAILURE);
        }
        memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
    }

    if (connect(normal_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    printf("[client] connected to server %s:%d\n", host, port);
}

static void
send_connect_command(void)
{
    // get listening port
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    getsockname(listening_fd, (struct sockaddr *) &addr, &len);
    int listening_port = ntohs(addr.sin_port);

    // generate cookie
    generate_cookie(cookie, sizeof(cookie));

    // send CONNECT command
    char msg[256];
    snprintf(msg, sizeof(msg), "%s%s:%d\n", CMD_CONNECT, cookie, listening_port);
    send(normal_fd, msg, strlen(msg), 0);

    // wait for OK
    char response[BUFFER_SIZE];
    recv(normal_fd, response, sizeof(response) - 1, 0);
    if (strncmp(response, RESP_OK, strlen(RESP_OK)) != 0) {
        fprintf(stderr, "server error: %s", response);
        exit(EXIT_FAILURE);
    }

    printf("[client] connect command accepted\n");
}

static void
accept_priority_connection(void)
{
    printf("[client] waiting for server priority connection...\n");
    priority_fd = accept(listening_fd, NULL, NULL);
    if (priority_fd == -1) {
        perror("accept priority");
        exit(EXIT_FAILURE);
    }
    printf("[client] priority connection established\n");
}

static void
verify_cookie(void)
{
    char buffer[BUFFER_SIZE];
    recv(priority_fd, buffer, sizeof(buffer) - 1, 0);

    // parse "COOKIE:xxx"
    char received_cookie[MAX_COOKIE_LEN + 1];
    if (sscanf(buffer, "%*[^:]:%63s", received_cookie) != 1 ||
        strcmp(received_cookie, cookie) != 0) {
        fprintf(stderr, "cookie verification failed\n");
        exit(EXIT_FAILURE);
    }

    // send verification
    send(priority_fd, CMD_VERIFIED "\n", strlen(CMD_VERIFIED "\n"), 0);
    printf("[client] cookie verified\n");
}

static void
request_file_transfer(void)
{
    char msg[512];
    snprintf(msg, sizeof(msg), "%s%s\n", CMD_TRANSFER, filename);
    send(normal_fd, msg, strlen(msg), 0);
    printf("[client] requesting file: %s\n", filename);
}

static void
handle_server_data(void)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes = recv(normal_fd, buffer, sizeof(buffer), 0);
    if (bytes <= 0) {
        if (bytes == 0) {
            printf("[client] server disconnected\n");
        } else {
            perror("[client] recv server data");
        }
        cleanup_and_exit(1);
    }

    if (output_file == NULL) {
        // first response - should be SIZE: or ERROR:
        buffer[bytes] = '\0';

        if (strncmp(buffer, RESP_SIZE, strlen(RESP_SIZE)) == 0) {
            file_size = atol(buffer + strlen(RESP_SIZE));
            printf("[client] file size: %ld bytes\n", file_size);

            output_file = fopen(filename, "wb");
            if (!output_file) {
                perror("fopen output file");
                cleanup_and_exit(1);
            }
            return;
        } else if (strncmp(buffer, RESP_ERROR, strlen(RESP_ERROR)) == 0) {
            printf("[client] server error: %s", buffer);
            cleanup_and_exit(1);
        } else if (strncmp(buffer, MSG_COMPLETE, strlen(MSG_COMPLETE)) == 0) {
            printf("[client] transfer completed (empty file)\n");
            cleanup_and_exit(0);
        }
        return; // ignore other messages while waiting for SIZE
    }

    // file data - write to file
    fwrite(buffer, 1, bytes, output_file);
    bytes_received += bytes;

    // check completion by size (exact byte count)
    if (bytes_received >= file_size) {
        fclose(output_file);
        output_file = NULL;
        printf("[client] file received: %ld bytes\n", bytes_received);
    }
}

static void
handle_priority_data(void)
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes = recv(priority_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        printf("priority: %s", buffer);
    }
}

static void
print_progress(void)
{
    if (output_file && file_size > 0) {
        double percent = (double)bytes_received * 100.0 / file_size;
        printf("\r[client] progress: %ld/%ld bytes (%.1f%%)  ", bytes_received, file_size, percent);
        fflush(stdout);
    }
}

static void
cleanup_and_exit(int sig)
{
    if (sig == SIGINT && output_file) {
        printf("\n[client] interrupted! sending reset...\n");
        if (priority_fd != -1) {
            send(priority_fd, CMD_RESET "\n", strlen(CMD_RESET "\n"), 0);
            usleep(100000);
        }
    }

    if (output_file) fclose(output_file);
    if (normal_fd != -1) close(normal_fd);
    if (priority_fd != -1) close(priority_fd);
    if (listening_fd != -1) close(listening_fd);

    exit(sig == SIGINT ? 0 : sig);
}
