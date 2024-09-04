#include "../include/server.h"
#include "../include/client.h"
#include "../include/jni_Bridge.h"

static JavaVM *java_vm = NULL;

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    java_vm = vm;

    return JNI_VERSION_21;
}

JNIEnv *getJNIEnv()
{
    JNIEnv *env;
    if ((*java_vm)->GetEnv(java_vm, (void **)&env, JNI_VERSION_21) != JNI_OK)
    {
        if ((*java_vm)->AttachCurrentThread(java_vm, (void **)&env, NULL) != 0)
        {
            return NULL;
        }
    }
    return env;
}

JNIEXPORT jint JNICALL Java_jni_Bridge_startChatRoom(JNIEnv *env, jclass clazz, jstring username)
{
    const char *admin_username = (*env)->GetStringUTFChars(env, username, 0);

    error_t main_thread_error;
    init_error(&main_thread_error);

    char public_ip[INET_ADDRSTRLEN];
    char local_ip[INET_ADDRSTRLEN];
    const char *port = PORT;

    if (start_chat_room(admin_username, local_ip, &main_thread_error, callback_error) != 0)
    {
        (*env)->ReleaseStringUTFChars(env, username, admin_username);
        report_errors(&main_thread_error, callback_error);
        return 1;
    }

    if (join_chat_room(local_ip, port, "", admin_username, USER_TYPE_ADMIN, &main_thread_error, callback_error, callback_message, callback_server_error, callback_notification) != 0)
    {
        (*env)->ReleaseStringUTFChars(env, username, admin_username);
        report_errors(&main_thread_error, callback_error);
        return 1;
    }

    (*env)->ReleaseStringUTFChars(env, username, admin_username);

    if (get_public_ip(public_ip, sizeof(public_ip), &main_thread_error) != 0)
    {
        (*env)->ReleaseStringUTFChars(env, username, admin_username);
        report_errors(&main_thread_error, callback_error);
        return 1;
    }

    const char *secret_key = get_secret_key();

    // callback Java with IP, Port and Secret Key
    printf("IP: %s\nPort: %s\nSecret: %s", public_ip, port, secret_key);

    return 0;
}

JNIEXPORT void JNICALL Java_jni_Bridge_joinChatRoom(JNIEnv *env, jclass clazz, jstring ip_address, jstring port, jstring secret_key, jstring username)
{
    const char *server_ip_address = (*env)->GetStringUTFChars(env, ip_address, 0);
    const char *server_port = (*env)->GetStringUTFChars(env, port, 0);
    const char *server_secret_key = (*env)->GetStringUTFChars(env, secret_key, 0);
    const char *client_username = (*env)->GetStringUTFChars(env, username, 0);

    error_t main_thread_error;
    init_error(&main_thread_error);

    if (join_chat_room(server_ip_address, server_port, server_secret_key, client_username, USER_TYPE_REGULAR, &main_thread_error, callback_error, callback_message, callback_server_error, callback_notification) != 0)
    {
        (*env)->ReleaseStringUTFChars(env, ip_address, server_ip_address);
        (*env)->ReleaseStringUTFChars(env, port, server_port);
        (*env)->ReleaseStringUTFChars(env, secret_key, server_secret_key);
        (*env)->ReleaseStringUTFChars(env, username, client_username);
        report_errors(&main_thread_error, callback_error);
        return;
    }

    (*env)->ReleaseStringUTFChars(env, ip_address, server_ip_address);
    (*env)->ReleaseStringUTFChars(env, port, server_port);
    (*env)->ReleaseStringUTFChars(env, secret_key, server_secret_key);
    (*env)->ReleaseStringUTFChars(env, username, client_username);
}

JNIEXPORT void JNICALL Java_jni_Bridge_sendMessage(JNIEnv *env, jclass clazz, jstring message)
{
    const char *client_message = (*env)->GetStringUTFChars(env, message, 0);

    error_t main_thread_error;
    init_error(&main_thread_error);

    send_regular_message(client_message, &main_thread_error, callback_error);

    (*env)->ReleaseStringUTFChars(env, message, client_message);
}

void callback_error(const char *aggregated_message, int max_severity)
{
    JNIEnv *env = getJNIEnv();
    if (env == NULL)
    {
        printf("Failed to get JNIEnv\n");
        return;
    }

    jclass controller_class = (*env)->FindClass(env, "controller/Controller");
    if (controller_class == NULL)
    {
        printf("Failed to find Controller class\n");
        return;
    }

    if (max_severity == CRITICAL_ERROR)
    {
        jmethodID show_popup_method = (*env)->GetStaticMethodID(env, controller_class, "showCriticalError", "(Ljava/lang/String;)V");
        if (show_popup_method == NULL)
        {
            printf("Failed to find showCriticalError method\n");
            return;
        }

        jstring jmessage = (*env)->NewStringUTF(env, aggregated_message);
        (*env)->CallStaticVoidMethod(env, controller_class, show_popup_method, jmessage);
        (*env)->DeleteLocalRef(env, jmessage);
    }
    else
    {
        jmethodID log_error_method = (*env)->GetStaticMethodID(env, controller_class, "logNonCriticalError", "(Ljava/lang/String;)V");
        if (log_error_method == NULL)
        {
            printf("Failed to find logNonCriticalError method\n");
            return;
        }

        jstring jmessage = (*env)->NewStringUTF(env, aggregated_message);
        (*env)->CallStaticVoidMethod(env, controller_class, log_error_method, jmessage);
        (*env)->DeleteLocalRef(env, jmessage);
    }
}

void callback_message(const char *username, const char *message)
{
    JNIEnv *env = getJNIEnv();
    if (env == NULL)
    {
        printf("Failed to get JNIEnv\n");
        return;
    }

    jclass controller_class = (*env)->FindClass(env, "controller/Controller");
    if (controller_class == NULL)
    {
        printf("Failed to find Controller class\n");
        return;
    }

    jmethodID display_message_method = (*env)->GetStaticMethodID(env, controller_class, "displayMessage", "(Ljava/lang/String;Ljava/lang/String;)V");
    if (display_message_method == NULL)
    {
        printf("Failed to find displayMessage method\n");
        return;
    }

    jstring jusername = (*env)->NewStringUTF(env, username);
    jstring jmessage = (*env)->NewStringUTF(env, message);

    (*env)->CallStaticVoidMethod(env, controller_class, display_message_method, jusername, jmessage);

    (*env)->DeleteLocalRef(env, jusername);
    (*env)->DeleteLocalRef(env, jmessage);
}

void callback_server_error(error_type_t error_type, const char *message)
{
    JNIEnv *env = getJNIEnv();
    if (env == NULL)
    {
        printf("Failed to get JNIEnv\n");
        return;
    }

    jclass controller_class = (*env)->FindClass(env, "controller/Controller");
    if (controller_class == NULL)
    {
        printf("Failed to find Controller class\n");
        return;
    }

    jmethodID show_error_method = NULL;
    if (error_type == ERROR_USERNAME)
    {
        show_error_method = (*env)->GetStaticMethodID(env, controller_class, "showUsernameError", "(Ljava/lang/String;)V");
    }
    else if (error_type == ERROR_SECRET_KEY)
    {
        show_error_method = (*env)->GetStaticMethodID(env, controller_class, "showSecretKeyError", "(Ljava/lang/String;)V");
    }

    if (show_error_method != NULL)
    {
        jstring jmessage = (*env)->NewStringUTF(env, message);
        (*env)->CallStaticVoidMethod(env, controller_class, show_error_method, jmessage);
        (*env)->DeleteLocalRef(env, jmessage);
    }
    else
    {
        printf("Error %d: %s\n", error_type, message);
    }
}

void callback_notification(notification_type_t notification_type, const char *message)
{
    JNIEnv *env = getJNIEnv();
    if (env == NULL)
    {
        printf("Failed to get JNIEnv\n");
        return;
    }

    jclass controller_class = (*env)->FindClass(env, "controller/Controller");
    if (controller_class == NULL)
    {
        printf("Failed to find Controller class\n");
        return;
    }

    if (notification_type == NOTIFICATION_AUTH_SUCCESS)
    {
        jmethodID switch_to_main_panel = (*env)->GetStaticMethodID(env, controller_class, "switchToMainPanel", "()V");
        if (switch_to_main_panel == NULL)
        {
            printf("Failed to find switchToMainPanel method\n");
            return;
        }

        (*env)->CallStaticVoidMethod(env, controller_class, switch_to_main_panel);
    }
    else
    {
        printf("Notification %d: %s\n", notification_type, message);
    }
}