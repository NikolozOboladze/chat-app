#include "../include/server.h"

static socket_t *listening_socket = NULL;
static atomic_int server_running = ATOMIC_VAR_INIT(0);
static client_node_t *client_list = NULL;
static rwlock_t client_list_rwlock;
static char global_secret_key[SECRET_KEY_BUFFER_SIZE];

int start_chat_room(const char *admin_username, char *local_ip, error_t *main_error, void (*callback_error_func)(const char *, int))
{
    if (strlen(admin_username) >= USERNAME_BUFFER_SIZE)
    {
        add_error(main_error, ERR_USERNAME_TOO_LONG, CRITICAL_ERROR, "Username exceeds buffer size", "start_chat_room");
        return 1;
    }

    rwlock_init(&client_list_rwlock);

    generate_secret_key(global_secret_key, sizeof(global_secret_key));

    struct addrinfo *address = NULL, hints;
    int result_code;

    listening_socket = (socket_t *)malloc(sizeof(socket_t));
    if (listening_socket == NULL)
    {
        add_error(main_error, MALLOC_ERROR, CRITICAL_ERROR, "Failed to allocate memory for listening socket", "start_chat_room");
        return 1;
    }

    result_code = socket_init(main_error);
    if (result_code != 0)
    {
        free(listening_socket);
        listening_socket = NULL;
        return 1;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (get_local_ip(local_ip, INET_ADDRSTRLEN) != 0)
    {
        add_error(main_error, ERR_LOCAL_IP_FAILURE, CRITICAL_ERROR, "Failed to retrieve local IP address", "start_chat_room");
        return 1;
    }

    result_code = getaddrinfo(local_ip, PORT, &hints, &address);
    if (result_code != 0)
    {
        add_error(main_error, GETADDRINFOERROR, CRITICAL_ERROR, "getaddrinfo failed", "start_chat_room");
        socket_cleanup(main_error);
        free(listening_socket);
        listening_socket = NULL;
        return 1;
    }

    *listening_socket = socket_create(address->ai_family, address->ai_socktype, address->ai_protocol, main_error);
    if (*listening_socket == INVALID_SOCK)
    {
        freeaddrinfo(address);
        socket_cleanup(main_error);
        free(listening_socket);
        listening_socket = NULL;
        return 1;
    }

    result_code = socket_bind(*listening_socket, address->ai_addr, (int)address->ai_addrlen, main_error);
    if (result_code == SOCKET_ERR)
    {
        socket_close(*listening_socket, main_error);
        freeaddrinfo(address);
        socket_cleanup(main_error);
        free(listening_socket);
        listening_socket = NULL;
        return 1;
    }

    freeaddrinfo(address);

    result_code = socket_listen(*listening_socket, main_error);
    if (result_code == SOCKET_ERR)
    {
        socket_close(*listening_socket, main_error);
        socket_cleanup(main_error);
        free(listening_socket);
        listening_socket = NULL;
        return 1;
    }

    accept_client_thread_args_t *thread_args = (accept_client_thread_args_t *)malloc(sizeof(accept_client_thread_args_t));
    if (thread_args == NULL)
    {
        add_error(main_error, MALLOC_ERROR, CRITICAL_ERROR, "Failed to allocate memory for accept thread args", "start_chat_room");
        socket_close(*listening_socket, main_error);
        socket_cleanup(main_error);
        free(listening_socket);
        listening_socket = NULL;
        return 1;
    }

    thread_args->listening_socket = listening_socket;
    thread_args->callback_error_func = callback_error_func;

    atomic_store(&server_running, 1);

    thread_t accept_thread;
    if (thread_create(&accept_thread, accept_client_thread, thread_args) != 0)
    {
        atomic_store(&server_running, 0);
        add_error(main_error, THREAD_CREATE_ERROR, CRITICAL_ERROR, "Failed to create accept client thread", "start_chat_room");
        socket_close(*listening_socket, main_error);
        socket_cleanup(main_error);
        free(listening_socket);
        listening_socket = NULL;
        free(thread_args);
        return 1;
    }

    thread_detach(accept_thread);

    return 0;
}

thread_ret_t THREAD_CALL accept_client_thread(void *arg)
{
    accept_client_thread_args_t *thread_args = (accept_client_thread_args_t *)arg;

    socket_t *listening_socket = thread_args->listening_socket;
    void (*callback_error_func)(const char *, int) = thread_args->callback_error_func;

    while (atomic_load(&server_running))
    {
        struct sockaddr_in client_addr;

        error_t accept_error;
        init_error(&accept_error);

        socket_t client_socket = socket_accept(*listening_socket, (struct sockaddr *)&client_addr, &accept_error);
        if (client_socket != INVALID_SOCK)
        {
            user_info_t *client_info = (user_info_t *)malloc(sizeof(user_info_t));
            if (client_info == NULL)
            {
                socket_close(client_socket, &accept_error);
                add_error(&accept_error, MALLOC_ERROR, CRITICAL_ERROR, "Failed to allocate memory for user_info_t struct", "accept_client_thread");
                report_errors(&accept_error, callback_error_func);
                break;
            }

            client_info->socket = client_socket;
            client_info->address = client_addr;
            client_info->username[0] = '\0';
            client_info->user_type = USER_TYPE_REGULAR;

            if (add_client(client_info, &accept_error) != 0)
            {
                socket_close(client_socket, &accept_error);
                free(client_info);
                report_errors(&accept_error, callback_error_func);
                break;
            }

            handle_client_thread_args_t *client_thread_args = (handle_client_thread_args_t *)malloc(sizeof(handle_client_thread_args_t));
            if (client_thread_args == NULL)
            {
                remove_client(client_socket, &accept_error);
                free(client_info);
                add_error(&accept_error, MALLOC_ERROR, CRITICAL_ERROR, "Failed to allocate memory for client thread args", "accept_client_thread");
                report_errors(&accept_error, callback_error_func);
                break;
            }

            client_thread_args->client_socket = client_socket;
            client_thread_args->callback_error_func = callback_error_func;

            thread_t handle_thread;
            if (thread_create(&handle_thread, handle_client_thread, client_thread_args) != 0)
            {
                remove_client(client_socket, &accept_error);
                free(client_info);
                free(client_thread_args);
                add_error(&accept_error, THREAD_CREATE_ERROR, CRITICAL_ERROR, "Failed to create handle client thread", "accept_client_thread");
                report_errors(&accept_error, callback_error_func);
                break;
            }
            else
            {
                thread_detach(handle_thread);
            }
        }
        else
        {
            report_errors(&accept_error, callback_error_func);
            if (accept_error.max_severity == CRITICAL_ERROR)
            {
                break;
            }
        }
    }

    atomic_store(&server_running, 0);

    error_t cleanup_error;
    init_error(&cleanup_error);
    remove_all_clients(&cleanup_error);
    socket_close(*listening_socket, &cleanup_error);
    socket_cleanup(&cleanup_error);
    free(listening_socket);
    listening_socket = NULL;

    if (cleanup_error.count > 0)
    {
        report_errors(&cleanup_error, callback_error_func);
    }

    free(thread_args);

#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

thread_ret_t THREAD_CALL handle_client_thread(void *arg)
{
    handle_client_thread_args_t *thread_args = (handle_client_thread_args_t *)arg;

    socket_t client_socket = thread_args->client_socket;
    void (*callback_error_func)(const char *, int) = thread_args->callback_error_func;

    char client_username[USERNAME_BUFFER_SIZE];
    client_username[0] = '\0';

    while (atomic_load(&server_running))
    {
        error_t error_struct;
        init_error(&error_struct);

        char message_buffer[MAX_BUFFER_SIZE];
        int bytes_received = socket_recv(client_socket, message_buffer, sizeof(message_buffer), 0, client_username, CONTEXT_SERVER, &error_struct);

        if (bytes_received == SOCKET_ERR)
        {
            if (strcmp(error_struct.errors[error_struct.count - 1].code, SOCKET_ECONNRESET) == 0)
            {
                // client got disconnected
                report_errors(&error_struct, callback_error_func);
                break;
            }
            else
            {
                // other error occurred
                report_errors(&error_struct, callback_error_func);
                continue;
            }
        }
        else if (bytes_received == 0)
        {
            // client disconnected gracefully
            break;
        }
        else
        {
            message_buffer[bytes_received] = '\0';

            int msg_type;
            sscanf(message_buffer, "%d:", &msg_type);

            if (msg_type == MSG_TYPE_AUTH)
            {
                user_type_t user_type;
                char received_secret_key[ENCODED_SECRET_KEY_BUFFER_SIZE];
                char received_username[USERNAME_BUFFER_SIZE];

                sscanf(message_buffer, "%*d:%d:%72[^:]:%80[^:]", &user_type, received_secret_key, received_username);

                received_secret_key[ENCODED_SECRET_KEY_BUFFER_SIZE - 1] = '\0';
                received_username[USERNAME_BUFFER_SIZE - 1] = '\0';

                if (user_type == USER_TYPE_ADMIN)
                {
                    update_client_info(client_socket, received_username, user_type);
                    strncpy(client_username, received_username, sizeof(client_username) - 1);
                    client_username[sizeof(client_username) - 1] = '\0';
                }
                else
                {
                    char decoded_secret_key[SECRET_KEY_BUFFER_SIZE];
                    decode_message(received_secret_key, decoded_secret_key, sizeof(decoded_secret_key));

                    if (strcmp(decoded_secret_key, global_secret_key) != 0)
                    {
                        send_error(client_socket, ERROR_SECRET_KEY, "Incorrect secret key", &error_struct, callback_error_func);
                        continue;
                    }

                    if (is_username_taken(received_username))
                    {
                        send_error(client_socket, ERROR_USERNAME, "Username already taken", &error_struct, callback_error_func);
                        continue;
                    }

                    update_client_info(client_socket, received_username, user_type);
                    strncpy(client_username, received_username, sizeof(client_username) - 1);
                    client_username[sizeof(client_username) - 1] = '\0';

                    send_notification(client_socket, NOTIFICATION_AUTH_SUCCESS, "", client_username, &error_struct, callback_error_func);
                }
            }
            else if (msg_type == MSG_TYPE_MESSAGE)
            {
                char encoded_message[ENCODED_MESSAGE_BUFFER_SIZE];

                sscanf(message_buffer, "%*d:%3000[^:]", encoded_message);

                encoded_message[ENCODED_MESSAGE_BUFFER_SIZE - 1] = '\0';

                broadcast_message(encoded_message, client_username, &error_struct, callback_error_func);
            }
        }
    }

    error_t disconnection_error;
    init_error(&disconnection_error);
    remove_client(client_socket, &disconnection_error);

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

void broadcast_message(const char *message, const char *sender_username, error_t *error, void (*callback_error_func)(const char *, int))
{
    rwlock_readerlock(&client_list_rwlock);

    client_node_t *current_client = client_list;
    while (current_client != NULL)
    {
        const char *receiver_username = current_client->client_info.username;
        send_message(current_client->client_info.socket, message, sender_username, receiver_username, CONTEXT_SERVER, error, callback_error_func);
        current_client = current_client->next;
    }

    rwlock_readerunlock(&client_list_rwlock);
}

int add_client(user_info_t *client_info, error_t *error)
{
    client_node_t *new_node = (client_node_t *)malloc(sizeof(client_node_t));
    if (new_node == NULL)
    {
        add_error(error, MALLOC_ERROR, CRITICAL_ERROR, "Failed to allocate memory for client_node_t struct", "add_client");
        return 1;
    }
    new_node->client_info = *client_info;

    rwlock_writerlock(&client_list_rwlock);

    new_node->next = client_list;
    client_list = new_node;

    rwlock_writerunlock(&client_list_rwlock);

    return 0;
}

void update_client_info(socket_t client_socket, const char *username, user_type_t user_type)
{
    rwlock_writerlock(&client_list_rwlock);
    client_node_t *current_client = client_list;

    while (current_client != NULL)
    {
        if (current_client->client_info.socket == client_socket)
        {
            strcpy(current_client->client_info.username, username);
            current_client->client_info.user_type = user_type;
            break;
        }
        current_client = current_client->next;
    }

    rwlock_writerunlock(&client_list_rwlock);
}

void remove_client(socket_t client_socket, error_t *error)
{
    rwlock_writerlock(&client_list_rwlock);

    client_node_t *current_client = client_list;
    client_node_t *previous_client = NULL;
    while (current_client != NULL)
    {
        if (current_client->client_info.socket == client_socket)
        {
            if (previous_client == NULL)
            {
                client_list = current_client->next;
            }
            else
            {
                previous_client->next = current_client->next;
            }
            socket_close(client_socket, error);
            free(current_client);
            break;
        }
        previous_client = current_client;
        current_client = current_client->next;
    }

    rwlock_writerunlock(&client_list_rwlock);
}

void remove_all_clients(error_t *error)
{
    rwlock_writerlock(&client_list_rwlock);

    client_node_t *current_client = client_list;
    while (current_client != NULL)
    {
        socket_t client_socket = current_client->client_info.socket;
        remove_client(client_socket, error);

        current_client = client_list;
    }

    rwlock_writerunlock(&client_list_rwlock);
}

int is_username_taken(const char *username)
{
    rwlock_readerlock(&client_list_rwlock);
    client_node_t *current_client = client_list;

    while (current_client != NULL)
    {
        if (strcmp(current_client->client_info.username, username) == 0)
        {
            rwlock_readerunlock(&client_list_rwlock);
            return 1;
        }
        current_client = current_client->next;
    }

    rwlock_readerunlock(&client_list_rwlock);
    return 0;
}

void generate_secret_key(char *key_buffer, size_t buffer_size)
{
    const size_t char_set_length = strlen(SECRET_KEY_CHAR_SET);

    srand(time(NULL));

    for (size_t i = 0; i < SECRET_KEY_LENGTH; ++i)
    {
        key_buffer[i] = SECRET_KEY_CHAR_SET[rand() % char_set_length];
    }

    key_buffer[SECRET_KEY_LENGTH] = '\0';
}

const char *get_secret_key(void)
{
    return global_secret_key;
}

void send_error(socket_t client_socket, error_type_t error_type, const char *error_message, error_t *error, void (*callback_error_func)(const char *, int))
{
    char buffer[ERROR_NOTIFICATION_BUFFER_SIZE];
    int result_code;

    snprintf(buffer, sizeof(buffer), "%d:%d:%s", MSG_TYPE_ERROR, error_type, error_message);

    result_code = socket_send(client_socket, buffer, strlen(buffer), 0, "", CONTEXT_SERVER, NON_CRITICAL_ERROR, error);

    if (result_code == SOCKET_ERR)
    {
        report_errors(error, callback_error_func);
    }
}

void send_notification(socket_t client_socket, notification_type_t notification_type, const char *notification_message, const char *receiver_username, error_t *error, void (*callback_error_func)(const char *, int))
{
    char buffer[ERROR_NOTIFICATION_BUFFER_SIZE];
    int result_code;

    snprintf(buffer, sizeof(buffer), "%d:%d:%s", MSG_TYPE_NOTIFICATION, notification_type, notification_message);

    result_code = socket_send(client_socket, buffer, strlen(buffer), 0, receiver_username, CONTEXT_SERVER, NON_CRITICAL_ERROR, error);

    if (result_code == SOCKET_ERR)
    {
        report_errors(error, callback_error_func);
    }
}

int get_local_ip(char *ip_buffer, size_t buffer_size)
{
#ifdef _WIN32
    PIP_ADAPTER_ADDRESSES adapter_addresses, aa;
    ULONG out_buf_len = sizeof(IP_ADAPTER_ADDRESSES);
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;

    adapter_addresses = (IP_ADAPTER_ADDRESSES *)malloc(out_buf_len);
    if (adapter_addresses == NULL)
    {
        return 1;
    }

    if (GetAdaptersAddresses(AF_UNSPEC, flags, NULL, adapter_addresses, &out_buf_len) == ERROR_BUFFER_OVERFLOW)
    {
        free(adapter_addresses);
        adapter_addresses = (IP_ADAPTER_ADDRESSES *)malloc(out_buf_len);
        if (adapter_addresses == NULL)
        {
            return 1;
        }
    }

    DWORD dwRetVal = GetAdaptersAddresses(AF_UNSPEC, flags, NULL, adapter_addresses, &out_buf_len);
    if (dwRetVal == NO_ERROR)
    {
        for (aa = adapter_addresses; aa != NULL; aa = aa->Next)
        {
            if (aa->OperStatus == IfOperStatusUp && aa->FirstUnicastAddress != NULL)
            {
                for (PIP_ADAPTER_UNICAST_ADDRESS ua = aa->FirstUnicastAddress; ua != NULL; ua = ua->Next)
                {
                    struct sockaddr_in *sa_in = (struct sockaddr_in *)ua->Address.lpSockaddr;
                    if (sa_in->sin_family == AF_INET)
                    {
                        strncpy(ip_buffer, inet_ntoa(sa_in->sin_addr), buffer_size);
                        ip_buffer[buffer_size - 1] = '\0';
                        free(adapter_addresses);
                        return 0;
                    }
                }
            }
        }
    }

    free(adapter_addresses);
    return 1;

#else
    struct ifaddrs *ifaddr, *ifa;
    int family, s;

    if (getifaddrs(&ifaddr) == -1)
    {
        return 1;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
        {
            continue;
        }

        family = ifa->ifa_addr->sa_family;
        if (family == AF_INET)
        {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                            ip_buffer, buffer_size, NULL, 0, NI_NUMERICHOST);
            if (s == 0 && strcmp(ifa->ifa_name, "lo") != 0)
            {
                freeifaddrs(ifaddr);
                return 0;
            }
        }
    }

    freeifaddrs(ifaddr);
    return 1;
#endif
}