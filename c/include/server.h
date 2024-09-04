#ifndef SERVER_H
#define SERVER_H

#include "common.h"
#include "threads.h"

#define PORT "6666"

#define SECRET_KEY_CHAR_SET "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()-_=+[]{}|;:,.<>?/"

typedef struct client_node
{
    user_info_t client_info;
    struct client_node *next;
} client_node_t;

typedef struct
{
    socket_t *listening_socket;
    void (*callback_error_func)(const char *, int);
} accept_client_thread_args_t;

typedef struct
{
    socket_t client_socket;
    void (*callback_error_func)(const char *, int);
} handle_client_thread_args_t;


int start_chat_room(const char *admin_username, char *local_ip, error_t *error, void (*callback_error_func)(const char *, int));
int close_chat_room(error_t *error);
thread_ret_t THREAD_CALL accept_client_thread(void *arg);
thread_ret_t THREAD_CALL handle_client_thread(void *arg);
void broadcast_message(const char *message, const char *sender_username, error_t *error, void (*callback_error_func)(const char *, int));
int add_client(user_info_t *client_info, error_t *error);
void update_client_info(socket_t client_socket, const char *username, user_type_t user_type);
void remove_client(socket_t client_socket, error_t *error);
void remove_all_clients(error_t *error);
int is_username_taken(const char *username);
void generate_secret_key(char *key_buffer, size_t buffer_size);
const char *get_secret_key(void);
int get_local_ip(char *ip_buffer, size_t buffer_size);

void send_error(socket_t client_socket, error_type_t error_type, const char *error_message, error_t *error, void (*callback_error_func)(const char *, int));
void send_notification(socket_t client_socket, notification_type_t notification_type, const char *notification_message, const char *receiver_username, error_t *error, void (*callback_error_func)(const char *, int));

#endif