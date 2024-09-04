#include "../include/client.h"

static socket_t *client_socket = NULL;
static atomic_int client_running = ATOMIC_VAR_INIT(0);

int join_chat_room(const char *ip_address, const char *port, const char *secret_key, const char *username, user_type_t user_type, error_t *main_error, void (*callback_error_func)(const char *, int), void (*callback_message_func)(const char *, const char *), void (*callback_server_error_func)(error_type_t, const char *), void (*callback_notification_func)(notification_type_t, const char *))
{
    if (atomic_load(&client_running))
    {
        send_auth_message(*client_socket, user_type, secret_key, username, main_error, callback_error_func);
        if (main_error->count > 0)
        {
            atomic_store(&client_running, 0);
            return 1;
        }
        return 0;
    }

    if (user_type != USER_TYPE_ADMIN)
    {
        if (strlen(secret_key) != SECRET_KEY_LENGTH)
        {
            callback_server_error_func(ERROR_SECRET_KEY, "Secret key must be exactly 24 characters long");
            return 1;
        }

        if (strlen(username) >= USERNAME_BUFFER_SIZE)
        {
            callback_server_error_func(ERROR_USERNAME, "Username exceeds buffer size");
            return 1;
        }
    }

    int result_code;

    client_socket = (socket_t *)malloc(sizeof(socket_t));
    if (client_socket == NULL)
    {
        add_error(main_error, MALLOC_ERROR, CRITICAL_ERROR, "Failed to allocate memory for client socket", "join_chat_room");
        return 1;
    }

    if (user_type != USER_TYPE_ADMIN)
    {
        result_code = socket_init(main_error);
        if (result_code != 0)
        {
            free(client_socket);
            client_socket = NULL;
            return 1;
        }
    }

    result_code = create_and_connect_client_socket(ip_address, port, client_socket, main_error);
    if (result_code != 0)
    {
        free(client_socket);
        client_socket = NULL;
        return 1;
    }

    client_receive_thread_args_t *thread_args = (client_receive_thread_args_t *)malloc(sizeof(client_receive_thread_args_t));
    if (thread_args == NULL)
    {
        add_error(main_error, MALLOC_ERROR, CRITICAL_ERROR, "Failed to allocate memory for client thread args", "join_chat_room");
        socket_close(*client_socket, main_error);
        socket_cleanup(main_error);
        free(client_socket);
        client_socket = NULL;
        return 1;
    }

    thread_args->client_socket = client_socket;
    thread_args->callback_error_func = callback_error_func;
    thread_args->callback_message_func = callback_message_func;
    thread_args->callback_server_error_func = callback_server_error_func;
    thread_args->callback_notification_func = callback_notification_func;
    thread_args->user_type = user_type;

    atomic_store(&client_running, 1);

    thread_t recv_thread;
    if (thread_create(&recv_thread, client_receive_thread, thread_args) != 0)
    {
        atomic_store(&client_running, 0);
        add_error(main_error, THREAD_CREATE_ERROR, CRITICAL_ERROR, "Failed to create client receive thread", "join_chat_room");
        socket_close(*client_socket, main_error);
        socket_cleanup(main_error);
        free(client_socket);
        client_socket = NULL;
        free(thread_args);
        return 1;
    }

    thread_detach(recv_thread);

    send_auth_message(*client_socket, user_type, secret_key, username, main_error, callback_error_func);
    if (main_error->count > 0)
    {
        atomic_store(&client_running, 0);
        return 1;
    }

    return 0;
}

thread_ret_t THREAD_CALL client_receive_thread(void *arg)
{
    client_receive_thread_args_t *thread_args = (client_receive_thread_args_t *)arg;
    socket_t *client_socket = thread_args->client_socket;
    void (*callback_error_func)(const char *, int) = thread_args->callback_error_func;
    void (*callback_message_func)(const char *, const char *) = thread_args->callback_message_func;
    void (*callback_server_error_func)(error_type_t, const char *) = thread_args->callback_server_error_func;
    void (*callback_notification_func)(notification_type_t, const char *) = thread_args->callback_notification_func;
    user_type_t user_type = thread_args->user_type;

    while (atomic_load(&client_running))
    {
        error_t error_struct;
        init_error(&error_struct);

        char message_buffer[MAX_BUFFER_SIZE];
        int bytes_received;

        bytes_received = socket_recv(*client_socket, message_buffer, sizeof(message_buffer), 0, "", CONTEXT_CLIENT, &error_struct);

        if (bytes_received == SOCKET_ERR)
        {
            // for now, we don't have non-critical errors for socket_recv on the client side so in case of an error, we disconnect
            report_errors(&error_struct, callback_error_func);
            if (user_type != USER_TYPE_ADMIN)
            {
                break;
            }
        }
        else if (bytes_received == 0)
        {
            // server disconnected gracefully.
            // ??? maybe call it back up to java a different way ???
            if (user_type != USER_TYPE_ADMIN)
            {
                add_error(&error_struct, SERVER_DISCONNECTED, CRITICAL_ERROR, "The server has disconnected", "client_receive_thread");
                report_errors(&error_struct, callback_error_func);
            }
            break;
        }
        else
        {
            message_buffer[bytes_received] = '\0';

            int msg_type;
            sscanf(message_buffer, "%d:", &msg_type);

            if (msg_type == MSG_TYPE_MESSAGE)
            {
                char received_username[USERNAME_BUFFER_SIZE];
                char received_encoded_message[ENCODED_MESSAGE_BUFFER_SIZE];

                sscanf(message_buffer, "%*d:%80[^:]:%3000[^:]", received_username, received_encoded_message);

                received_username[USERNAME_BUFFER_SIZE - 1] = '\0';
                received_encoded_message[ENCODED_MESSAGE_BUFFER_SIZE - 1] = '\0';

                char decoded_message[MESSAGE_BUFFER_SIZE];
                decode_message(received_encoded_message, decoded_message, sizeof(decoded_message));

                callback_message_func(received_username, decoded_message);
            }
            else if (user_type != USER_TYPE_ADMIN)
            {
                if (msg_type == MSG_TYPE_ERROR)
                {
                    error_type_t error_type;
                    char error_message[ERROR_NOTIFICATION_BUFFER_SIZE];

                    sscanf(message_buffer, "%*d:%d:%250[^:]", &error_type, error_message);

                    error_message[ERROR_NOTIFICATION_BUFFER_SIZE - 1] = '\0';

                    callback_server_error_func(error_type, error_message);
                }
                else if (msg_type == MSG_TYPE_NOTIFICATION)
                {
                    notification_type_t notification_type;
                    char notification_message[ERROR_NOTIFICATION_BUFFER_SIZE];

                    sscanf(message_buffer, "%*d:%d:%250[^:]", &notification_type, notification_message);

                    notification_message[ERROR_NOTIFICATION_BUFFER_SIZE - 1] = '\0';

                    callback_notification_func(notification_type, notification_message);
                }
            }
        }
    }

    error_t disconnection_error;
    init_error(&disconnection_error);
    socket_close(*client_socket, &disconnection_error);
    socket_cleanup(&disconnection_error);
    free(client_socket);
    client_socket = NULL;

    if (disconnection_error.count > 0)
    {
        report_errors(&disconnection_error, callback_error_func);
    }

    free(thread_args);

#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

int create_and_connect_client_socket(const char *server_address, const char *port, socket_t *socket, error_t *main_error)
{
    struct addrinfo hints, *address;
    int result_code;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    result_code = getaddrinfo(server_address, port, &hints, &address);
    if (result_code != 0)
    {
        add_error(main_error, GETADDRINFOERROR, CRITICAL_ERROR, "getaddrinfo failed", "create_and_connect_client_socket");
        socket_cleanup(main_error);
        return 1;
    }

    *socket = socket_create(address->ai_family, address->ai_socktype, address->ai_protocol, main_error);
    if (*socket == INVALID_SOCK)
    {
        freeaddrinfo(address);
        socket_cleanup(main_error);
        return 1;
    }

    result_code = socket_connect(*socket, address->ai_addr, address->ai_addrlen, main_error);
    if (result_code != 0)
    {
        freeaddrinfo(address);
        socket_close(*socket, main_error);
        socket_cleanup(main_error);
        return 1;
    }

    freeaddrinfo(address);

    return 0;
}

int get_public_ip(char *ip_buffer, size_t buffer_size, error_t *main_error)
{
    const char *services[] = {PUBLIC_IP_SERVICE_ONE, PUBLIC_IP_SERVICE_TWO, PUBLIC_IP_SERVICE_THREE};
    const char *requests[] = {HTTP_GET_REQUEST_ONE, HTTP_GET_REQUEST_TWO, HTTP_GET_REQUEST_THREE};

    socket_t ip_socket = INVALID_SOCK;

    for (int i = 0; i < 3; i++)
    {
        const char *http_request = requests[i];

        if (create_and_connect_client_socket(services[i], HTTP_PORT, &ip_socket, main_error) != 0)
        {
            continue;
        }

        if (socket_send(ip_socket, http_request, strlen(http_request), 0, "", CONTEXT_CLIENT, 1, main_error) == SOCKET_ERR)
        {
            socket_close(ip_socket, main_error);
            continue;
        }

        char response[MESSAGE_BUFFER_SIZE];

        int bytes_received = socket_recv(ip_socket, response, sizeof(response), 0, "", CONTEXT_CLIENT, main_error);

        if (bytes_received == SOCKET_ERR)
        {
            socket_close(ip_socket, main_error);
            continue;
        }

        response[bytes_received] = '\0';
        socket_close(ip_socket, main_error);

        char *ip_start = strstr(response, "\r\n\r\n");
        // if there is no response body
        if (ip_start == NULL)
        {
            add_error(main_error, ERR_NO_RESPONSE_BODY, CRITICAL_ERROR, "No response body in HTTP response", "get_public_ip");
            continue;
        }

        // move into the response body
        ip_start += 4;

        size_t ip_length = strlen(ip_start);
        if (ip_length >= buffer_size)
        {
            add_error(main_error, ERR_IP_TOO_LONG, CRITICAL_ERROR, "Received IP is too long", "get_public_ip");
            continue;
        }

        strncpy(ip_buffer, ip_start, buffer_size - 1);
        ip_buffer[buffer_size - 1] = '\0';

        return 0;
    }

    return 1;
}

void send_auth_message(socket_t client_socket, user_type_t user_type, const char *secret_key, const char *username, error_t *error, void (*callback_error_func)(const char *, int))
{
    char buffer[AUTH_MESSAGE_BUFFER_SIZE];
    int result_code;

    if (user_type == USER_TYPE_ADMIN)
    {
        snprintf(buffer, sizeof(buffer), "%d:%d:ADMIN:%s", MSG_TYPE_AUTH, user_type, username);
    }
    else
    {
        char encoded_secret_key[ENCODED_SECRET_KEY_BUFFER_SIZE];
        encode_message(secret_key, encoded_secret_key, sizeof(encoded_secret_key));

        snprintf(buffer, sizeof(buffer), "%d:%d:%s:%s", MSG_TYPE_AUTH, user_type, encoded_secret_key, username);
    }

    result_code = socket_send(client_socket, buffer, strlen(buffer), 0, "", CONTEXT_CLIENT, CRITICAL_ERROR, error);

    if (result_code == SOCKET_ERR)
    {
        report_errors(error, callback_error_func);
    }
}

void send_regular_message(const char *message, error_t *error, void (*callback_error_func)(const char *, int))
{
    char encoded_message[ENCODED_MESSAGE_BUFFER_SIZE];
    encode_message(message, encoded_message, sizeof(encoded_message));

    send_message(*client_socket, encoded_message, "", "", CONTEXT_CLIENT, error, callback_error_func);
}
