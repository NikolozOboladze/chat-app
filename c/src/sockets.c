#include "../include/sockets.h"

int socket_cleanup(error_t *error)
{
#ifdef _WIN32
    int retry_count = 3;
    int result_code;

    while (retry_count > 0)
    {
        result_code = WSACleanup();
        if (result_code == 0)
        {
            return 0;
        }
        retry_count--;
        cross_platform_sleep(1);
    }

    if (result_code == SOCKET_ERR)
    {
        add_error(error, map_platform_error(get_last_socket_error()), CRITICAL_ERROR, "WSACleanup failed after retries", "socket_cleanup");
        return 1;
    }
#else
    (void)error;
#endif
    return 0;
}

int socket_init(error_t *error)
{
#ifdef _WIN32
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);

    int result_code = WSAStartup(wVersionRequested, &wsaData);

    if (result_code != 0)
    {
        add_error(error, map_platform_error(result_code), CRITICAL_ERROR, "WSAStartup failed", "socket_init");
        return 1;
    }

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        add_error(error, SOCKET_WSAVERSIONUNUSABLE, CRITICAL_ERROR, "WSAStartup failed, could not find a usable version of Winsock.dll", "socket_init");
        socket_cleanup(error);
        return 1;
    }
#else
    (void)error;
#endif

    return 0;
}

socket_t socket_create(int domain, int type, int protocol, error_t *error)
{
    socket_t sock = socket(domain, type, protocol);

    if (sock == INVALID_SOCK)
    {
        add_error(error, map_platform_error(get_last_socket_error()), CRITICAL_ERROR, "Socket creation failed", "socket_create");
    }

    return sock;
}

int socket_bind(socket_t sock, const struct sockaddr *addr, socklen_t addrlen, error_t *error)
{
    int result_code = bind(sock, addr, addrlen);

    if (result_code == SOCKET_ERR)
    {
        add_error(error, map_platform_error(get_last_socket_error()), CRITICAL_ERROR, "Socket bind failed", "socket_bind");
    }

    return result_code;
}

int socket_listen(socket_t sock, error_t *error)
{
    int result_code = listen(sock, SOMAXCONN);

    if (result_code == SOCKET_ERR)
    {
        add_error(error, map_platform_error(get_last_socket_error()), CRITICAL_ERROR, "Socket listen failed", "socket_listen");
    }

    return result_code;
}

socket_t socket_accept(socket_t sock, struct sockaddr *addr, error_t *error)
{
    socket_t client_socket = accept(sock, addr, NULL);

    if (client_socket == INVALID_SOCK)
    {
        const char *err = map_platform_error(get_last_socket_error());

        if (strcmp(err, SOCKET_EINTR) == 0)
        {
            add_error(error, err, NON_CRITICAL_ERROR, "Socket accept interrupted by a signal", "socket_accept");
        }
        else
        {
            add_error(error, err, CRITICAL_ERROR, "Socket accept failed", "socket_accept");
        }
    }

    return client_socket;
}

int socket_connect(socket_t sock, const struct sockaddr *addr, socklen_t addrlen, error_t *error)
{
    int retry_count = 3;
    int result_code;
    const char *last_error = NULL;

    while (retry_count > 0)
    {
        result_code = connect(sock, addr, addrlen);
        if (result_code == 0)
        {
            return 0;
        }

        last_error = map_platform_error(get_last_socket_error());

        if (strcmp(last_error, SOCKET_ECONNREFUSED) == 0 || strcmp(last_error, SOCKET_ETIMEDOUT) == 0 ||
            strcmp(last_error, SOCKET_EHOSTUNREACH) == 0 || strcmp(last_error, SOCKET_ENETUNREACH) == 0)
        {
            retry_count--;
            cross_platform_sleep(1);
        }
        else
        {
            break;
        }
    }

    add_error(error, last_error, CRITICAL_ERROR, "Socket connect failed", "socket_connect");
    return result_code;
}

int socket_send(socket_t sock, const void *buf, size_t len, int flags, const char *client_username, context_t context, error_severity_t severity, error_t *error)
{
    int result_code = send(sock, buf, len, flags);

    if (result_code == SOCKET_ERR)
    {
        const char *err = map_platform_error(get_last_socket_error());
        char error_message[256];

        if (context == CONTEXT_SERVER)
        {
            if (client_username[0] == '\0')
            {
                snprintf(error_message, sizeof(error_message), "Failed to send to a client");
            }
            else
            {
                snprintf(error_message, sizeof(error_message), "Failed to send to client \"%s\"", client_username);
            }
        }
        else
        {
            snprintf(error_message, sizeof(error_message), "Failed to send to the server");
        }

        add_error(error, err, severity, error_message, "socket_send");
    }

    return result_code;
}

int socket_recv(socket_t sock, void *buf, size_t len, int flags, const char *client_username, context_t context, error_t *error)
{
    int result_code = recv(sock, buf, len, flags);

    if (result_code == SOCKET_ERR)
    {
        const char *err = map_platform_error(get_last_socket_error());
        char error_message[256];

        // note: not all errors may be critical for the client
        //       this can be adjusted based on specific error types if needed
        error_severity_t severity = (context == CONTEXT_CLIENT) ? CRITICAL_ERROR : NON_CRITICAL_ERROR;

        if (context == CONTEXT_SERVER)
        {
            if (strcmp(err, SOCKET_ECONNRESET) == 0)
            {
                if (client_username[0] == '\0')
                {
                    snprintf(error_message, sizeof(error_message), "A client disconnected");
                }
                else
                {
                    snprintf(error_message, sizeof(error_message), "Client \"%s\" got disconnected", client_username);
                }
            }
            else
            {
                if (client_username[0] == '\0')
                {
                    snprintf(error_message, sizeof(error_message), "A client encountered an error");
                }
                else
                {
                    snprintf(error_message, sizeof(error_message), "Client \"%s\" encountered an error", client_username);
                }
            }
        }
        else
        {
            if (strcmp(err, SOCKET_ECONNRESET) == 0)
            {
                snprintf(error_message, sizeof(error_message), "The server disconnected");
            }
            else
            {
                snprintf(error_message, sizeof(error_message), "The server encountered an error");
            }
        }

        add_error(error, err, severity, error_message, "socket_recv");
    }

    return result_code;
}

int socket_close(socket_t sock, error_t *error)
{
    int retry_count = 3;
    int result_code;

    while (retry_count > 0)
    {
#ifdef _WIN32
        result_code = closesocket(sock);
#else
        result_code = close(sock);
#endif

        if (result_code != SOCKET_ERR)
        {
            return result_code;
        }

        if (retry_count == 1)
        {
            add_error(error, map_platform_error(get_last_socket_error()), CRITICAL_ERROR, "Socket close failed after retries", "socket_close");
        }

        retry_count--;
        cross_platform_sleep(1);
    }

    return result_code;
}

int get_last_socket_error()
{
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

const char *map_platform_error(int platform_error)
{
    switch (platform_error)
    {
#ifdef _WIN32
    case WSAECONNRESET:
        return SOCKET_ECONNRESET;
    case WSAEINTR:
        return SOCKET_EINTR;
    case WSAEADDRINUSE:
        return SOCKET_EADDRINUSE;
    case WSAEADDRNOTAVAIL:
        return SOCKET_EADDRNOTAVAIL;
    case WSAENETDOWN:
        return SOCKET_ENETDOWN;
    case WSAENETUNREACH:
        return SOCKET_ENETUNREACH;
    case WSAETIMEDOUT:
        return SOCKET_ETIMEDOUT;
    case WSAECONNREFUSED:
        return SOCKET_ECONNREFUSED;
    case WSAEHOSTUNREACH:
        return SOCKET_EHOSTUNREACH;
    case WSASYSNOTREADY:
        return SOCKET_WSASYSNOTREADY;
    case WSAVERNOTSUPPORTED:
        return SOCKET_WSAVERNOTSUPPORTED;
#else
    case ECONNRESET:
        return SOCKET_ECONNRESET;
    case EINTR:
        return SOCKET_EINTR;
    case EADDRINUSE:
        return SOCKET_EADDRINUSE;
    case EADDRNOTAVAIL:
        return SOCKET_EADDRNOTAVAIL;
    case ENETDOWN:
        return SOCKET_ENETDOWN;
    case ENETUNREACH:
        return SOCKET_ENETUNREACH;
    case ETIMEDOUT:
        return SOCKET_ETIMEDOUT;
    case ECONNREFUSED:
        return SOCKET_ECONNREFUSED;
    case EHOSTUNREACH:
        return SOCKET_EHOSTUNREACH;
#endif
    default:
        return SOCKET_UNKNOWN_ERROR;
    }
}

void cross_platform_sleep(int seconds)
{
#ifdef _WIN32
    Sleep(seconds * 1000);
#else
    sleep(seconds);
#endif
}