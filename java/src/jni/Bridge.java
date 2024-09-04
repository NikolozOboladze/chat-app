package jni;

public class Bridge {

    public static native void initializeBridge();

    public static native int startChatRoom(String username);

    public static native void joinChatRoom(String ipAddress, String port, String secretKey, String username);

    public static native void sendMessage(String message);

    public static native void kickUser(String username);

    public static native void banUser(String username);

    public static native void leaveChatRoom();

    public static native void closeChatRoom();
    
}
