#ifndef ERRORS_H
#define ERRORS_H

#include <stdio.h>
#include <string.h>

#define MAX_ERRORS 10
#define AGGREGATED_ERROR_SIZE 1024

#define MALLOC_ERROR "MALLOC_ERROR"
#define THREAD_CREATE_ERROR "THREAD_CREATE_ERROR"
#define SERVER_DISCONNECTED "SERVER_DISCONNECTED"
#define ERR_SECRET_KEY_LENGTH "ERR_SECRET_KEY_LENGTH"
#define ERR_USERNAME_TOO_LONG "ERR_USERNAME_TOO_LONG"

#define ERR_LOCAL_IP_FAILURE "ERR_LOCAL_IP_FAILURE"
#define ERR_NO_RESPONSE_BODY "ERR_NO_RESPONSE_BODY"
#define ERR_IP_TOO_LONG "ERR_IP_TOO_LONG"

#ifdef _WIN32
#define SOCKET_WSASYSNOTREADY "SOCKET_WSASYSNOTREADY"
#define SOCKET_WSAVERNOTSUPPORTED "SOCKET_WSAVERNOTSUPPORTED"
#define SOCKET_WSAVERSIONUNUSABLE "SOCKET_WSAVERSIONUNUSABLE"
#endif

#define GETADDRINFOERROR "GETADDRINFOERROR"

#define SOCKET_ECONNRESET "SOCKET_ECONNRESET"
#define SOCKET_EINTR "SOCKET_EINTR"
#define SOCKET_EADDRINUSE "SOCKET_EADDRINUSE"
#define SOCKET_EADDRNOTAVAIL "SOCKET_EADDRNOTAVAIL"
#define SOCKET_ENETDOWN "SOCKET_ENETDOWN"
#define SOCKET_ENETUNREACH "SOCKET_ENETUNREACH"
#define SOCKET_ETIMEDOUT "SOCKET_ETIMEDOUT"
#define SOCKET_ECONNREFUSED "SOCKET_ECONNREFUSED"
#define SOCKET_EHOSTUNREACH "SOCKET_EHOSTUNREACH"
#define SOCKET_UNKNOWN_ERROR "SOCKET_UNKNOWN_ERROR"

typedef enum
{
    NON_CRITICAL_ERROR,
    CRITICAL_ERROR
} error_severity_t;

typedef enum
{
    ERROR_NONE,
    ERROR_SECRET_KEY,
    ERROR_USERNAME,
    ERROR_GENERAL
} error_type_t;

typedef struct
{
    const char *code;
    int severity;
    const char *message;
    const char *location;
} error_detail_t;

typedef struct
{
    error_detail_t errors[MAX_ERRORS];
    int count;
    char aggregated_message[AGGREGATED_ERROR_SIZE];
    int max_severity;
} error_t;

void init_error(error_t *error);
void add_error(error_t *error, const char *err_code, error_severity_t severity, const char *message, const char *location);
void report_errors(error_t *error, void (*callback)(const char *, int));

#endif