#include "../include/common.h"

void send_message(socket_t client_socket, const char *message, const char *sender_username, const char *receiver_username, context_t context, error_t *error, void (*callback_error_func)(const char *, int))
{
    char buffer[MAX_BUFFER_SIZE];
    int result_code;

    if (context == CONTEXT_SERVER)
    {
        snprintf(buffer, sizeof(buffer), "%d:%s:%s", MSG_TYPE_MESSAGE, sender_username, message);
    }
    else
    {
        snprintf(buffer, sizeof(buffer), "%d:%s", MSG_TYPE_MESSAGE, message);
    }

    result_code = socket_send(client_socket, buffer, strlen(buffer), 0, receiver_username, context, NON_CRITICAL_ERROR, error);

    if (result_code == SOCKET_ERR)
    {
        report_errors(error, callback_error_func);
    }
}

void encode_message(const char *input, char *output, size_t output_size)
{
    size_t j = 0;
    for (size_t i = 0; input[i] != '\0' && j < output_size - 1; ++i)
    {
        if (input[i] == ':')
        {
            if (j + 3 < output_size - 1)
            {
                output[j++] = '|';
                output[j++] = 'C';
                output[j++] = '|';
            }
            else
            {
                break;
            }
        }
        else
        {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
}

void decode_message(const char *input, char *output, size_t output_size)
{
    size_t j = 0;
    for (size_t i = 0; input[i] != '\0' && j < output_size - 1; ++i)
    {
        if (input[i] == '|' && input[i + 1] == 'C' && input[i + 2] == '|')
        {
            output[j++] = ':';
            i += 2;
        }
        else
        {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
}