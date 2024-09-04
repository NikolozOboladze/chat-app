#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"
#include "threads.h"

#define PUBLIC_IP_SERVICE_ONE "api.ipify.org"
#define PUBLIC_IP_SERVICE_TWO "ifconfig.me"
#define PUBLIC_IP_SERVICE_THREE "checkip.amazonaws.com"
#define HTTP_GET_REQUEST_ONE "GET / HTTP/1.1\r\nHost: api.ipify.org\r\nConnection: close\r\n\r\n"
#define HTTP_GET_REQUEST_TWO "GET / HTTP/1.1\r\nHost: ifconfig.me\r\nConnection: close\r\n\r\n"
#define HTTP_GET_REQUEST_THREE "GET / HTTP/1.1\r\nHost: checkip.amazonaws.com\r\nConnection: close\r\n\r\n"
#define HTTP_PORT "80"

typedef struct
{
    socket_t *client_socket;
    void (*callback_error_func)(const char *, int);
    void (*callback_message_func)(const char *, const char *);
    void (*callback_server_error_func)(error_type_t, const char *);
    void (*callback_notification_func)(notification_type_t, const char *);
    user_type_t user_type;
} client_receive_thread_args_t;

int join_chat_room(const char *ip_address, const char *port, const char *secret_key, const char *username, user_type_t user_type, error_t *error, void (*callback_error_func)(const char *, int), void (*callback_message_func)(const char *, const char *), void (*callback_server_error_func)(error_type_t, const char *), void (*callback_notification_func)(notification_type_t, const char *));
int create_and_connect_client_socket(const char *server_address, const char *port, socket_t *sock, error_t *error);
int get_public_ip(char *ip_buffer, size_t buffer_size, error_t *error);

void send_auth_message(socket_t client_socket, user_type_t user_type, const char *secret_key, const char *username, error_t *error, void (*callback_error_func)(const char *, int));
void send_regular_message(const char *message, error_t *error, void (*callback_error_func)(const char *, int));

thread_ret_t THREAD_CALL client_receive_thread(void *arg);

#endif