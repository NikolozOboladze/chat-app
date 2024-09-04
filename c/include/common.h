#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdatomic.h>
#include "sockets.h"

#define SECRET_KEY_LENGTH 24
#define SECRET_KEY_BUFFER_SIZE (SECRET_KEY_LENGTH + 1)
#define ENCODED_SECRET_KEY_BUFFER_SIZE ((SECRET_KEY_LENGTH * 3) + 1)

// USERNAME_BUFFER_SIZE calculation:
// 20: The maximum number of characters for a username
// x4: To account for multi-byte characters (e.g., UTF-8), assuming the worst-case scenario where each character is 4 bytes
// 1: To account for the null terminator
#define USERNAME_BUFFER_SIZE (20 * 4 + 1)

// MESSAGE_BUFFER_SIZE calculation:
// 250: The maximum number of characters for a message
// x4: To account for multi-byte characters (e.g., UTF-8), assuming the worst-case scenario where each character is 4 bytes
// 1: To account for the null terminator
#define MESSAGE_BUFFER_SIZE (250 * 4 + 1)
#define ENCODED_MESSAGE_BUFFER_SIZE ((MESSAGE_BUFFER_SIZE - 1) * 3 + 1)

// AUTH_MESSAGE_BUFFER_SIZE calculation:
// 1: The maximum number of characters to represent an integer message type
// 1: The maximum number of characters to represent an integer user type
// 3: The three colons (:) used as delimiters in the formatted string
// ENCODED_SECRET_KEY_BUFFER_SIZE: The maximum length of the encoded secret key string, minus the null terminator
// USERNAME_BUFFER_SIZE - 1: The maximum length of the username string, minus the null terminator
// 1: The null terminator for the entire formatted string
#define AUTH_MESSAGE_BUFFER_SIZE (1 + 1 + 3 + (ENCODED_SECRET_KEY_BUFFER_SIZE - 1) + (USERNAME_BUFFER_SIZE - 1) + 1)

// ERROR_NOTIFICATION_BUFFER_SIZE calculation:
// 1: The character for the message type
// 1: The character for the error type or notification type
// 2: The two colons (:) used as delimiters in the formatted string
// 250: The maximum length of the error or notification message
// 1: The null terminator for the entire formatted string
#define ERROR_NOTIFICATION_BUFFER_SIZE (1 + 1 + 2 + 250 + 1)

// MAX_BUFFER_SIZE calculation:
// 1: The maximum number of characters to represent an integer message type
// 2: The two colons (:) used as delimiters in the formatted string
// USERNAME_BUFFER_SIZE - 1: The maximum length of the username string, minus the null terminator
// ENCODED_MESSAGE_BUFFER_SIZE - 1: The maximum length of the encoded message string, minus the null terminator
// 1: The null terminator for the entire formatted string
#define MAX_BUFFER_SIZE (1 + 2 + (USERNAME_BUFFER_SIZE - 1) + (ENCODED_MESSAGE_BUFFER_SIZE - 1) + 1)

typedef enum
{
    USER_TYPE_REGULAR,
    USER_TYPE_ADMIN
} user_type_t;

typedef enum
{
    MSG_TYPE_ERROR,
    MSG_TYPE_AUTH,
    MSG_TYPE_MESSAGE,
    MSG_TYPE_NOTIFICATION,
} message_type_t;

typedef enum
{
    NOTIFICATION_AUTH_SUCCESS,
    NOTIFICATION_DISCONNECT,
    NOTIFICATION_KICK,
    NOTIFICATION_BAN,
    NOTIFICATION_GENERAL
} notification_type_t;

typedef struct
{
    socket_t socket;
    struct sockaddr_in address;
    char username[USERNAME_BUFFER_SIZE];
    user_type_t user_type;
} user_info_t;

void send_message(socket_t client_socket, const char *message, const char *sender_username, const char *receiver_username, context_t context, error_t *error, void (*callback_error_func)(const char *, int));
void encode_message(const char *input, char *output, size_t output_size);
void decode_message(const char *input, char *output, size_t output_size);

#endif