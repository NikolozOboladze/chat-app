#ifndef SOCKETS_H
#define SOCKETS_H

#include "errors.h"

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <minwindef.h>
typedef SOCKET socket_t;
#define SOCKET_ERR SOCKET_ERROR
#define INVALID_SOCK INVALID_SOCKET
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <errno.h>
typedef int socket_t;
#define SOCKET_ERR (-1)
#define INVALID_SOCK (-1)
#endif

typedef enum
{
    CONTEXT_CLIENT,
    CONTEXT_SERVER
} context_t;

int socket_init(error_t *error);
int socket_cleanup(error_t *error);

socket_t socket_create(int domain, int type, int protocol, error_t *error);
int socket_bind(socket_t sock, const struct sockaddr *addr, socklen_t addrlen, error_t *error);
int socket_listen(socket_t sock, error_t *error);
socket_t socket_accept(socket_t sock, struct sockaddr *addr, error_t *error);
int socket_connect(socket_t sock, const struct sockaddr *addr, socklen_t addrlen, error_t *error);
int socket_send(socket_t sock, const void *buf, size_t len, int flags, const char *client_username, context_t context, error_severity_t severity, error_t *error);
int socket_recv(socket_t sock, void *buf, size_t len, int flags, const char *client_username, context_t context, error_t *error);
int socket_close(socket_t sock, error_t *error);

int get_last_socket_error();
const char *map_platform_error(int platform_error);

void cross_platform_sleep(int seconds);

#endif