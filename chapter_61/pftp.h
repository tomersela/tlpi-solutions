#ifndef PFTP_H
#define PFTP_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <dirent.h>
#include <sys/stat.h>
#include "tlpi_hdr.h"

// protocol constants
#define BUFFER_SIZE 8192   // 8KB - good balance of speed vs interruptible
#define PROGRESS_INTERVAL 10240  // show progress every 10KB
#define FILES_DIR "files"
#define MAX_FILENAME_LEN 255
#define MAX_COOKIE_LEN 63

// protocol command strings
#define CMD_CONNECT "CONNECT:"
#define CMD_TRANSFER "TRANSFER:"
#define CMD_COOKIE "COOKIE:"
#define CMD_RESET "RESET"
#define CMD_STATUS "STATUS"
#define CMD_VERIFIED "VERIFIED"

// protocol response strings
#define RESP_OK "OK"
#define RESP_ERROR "ERROR:"
#define RESP_SIZE "SIZE:"
#define RESP_COMPLETE "COMPLETE"
#define RESP_STATUS "STATUS:"
#define RESP_RESET "RESET:"

// error messages
#define ERR_INVALID_CONNECT "ERROR:Invalid CONNECT format\n"
#define ERR_INVALID_PORT "ERROR:Invalid port number\n"
#define ERR_CANNOT_CREATE_PRIORITY "ERROR:Cannot create priority socket\n"
#define ERR_INVALID_CLIENT_IP "ERROR:Invalid client IP\n"
#define ERR_CANNOT_ESTABLISH_PRIORITY "ERROR:Cannot establish priority connection\n"
#define ERR_CANNOT_SEND_COOKIE "ERROR:Cannot send verification cookie\n"
#define ERR_UNKNOWN_COMMAND "ERROR:Unknown command\n"
#define ERR_PRIORITY_NOT_ESTABLISHED "ERROR:Priority connection not established\n"
#define ERR_TRANSFER_IN_PROGRESS "ERROR:Transfer already in progress\n"
#define ERR_INVALID_FILENAME "ERROR:Invalid filename\n"
#define ERR_FILE_NOT_FOUND "ERROR:File not found\n"
#define ERR_CANNOT_DETERMINE_SIZE "ERROR:Cannot determine file size\n"
#define ERR_FILE_READ_ERROR "ERROR:File read error\n"
#define ERR_UNKNOWN_PRIORITY_CMD "ERROR:Unknown priority command\n"

// response messages
#define MSG_OK "OK\n"
#define MSG_COMPLETE "COMPLETE\n"
#define MSG_STATUS_IDLE "STATUS:Idle\n"
#define MSG_STATUS_NO_TRANSFER "STATUS:No transfer in progress\n"
#define MSG_RESET_CANCELLED "RESET:Transfer cancelled\n"
#define MSG_RESET_OK "OK:Transfer reset\n"

// shared function prototypes
int validate_filename(const char *filename);
void generate_cookie(char *cookie, size_t cookie_size);
int parse_connect_command(const char *message, char *cookie, char *port_str);

#endif // PFTP_H
