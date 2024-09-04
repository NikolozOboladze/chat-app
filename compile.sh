JAVA_HOME="C:/Program Files/Java/jdk-21"
C_SOURCE_FILES="c/src/bridge.c c/src/server.c c/src/client.c c/src/errors.c c/src/sockets.c c/src/common.c"

# JAVA_BRIDGE_DIR="java/src/jni"
# C_INCLUDE_DIR="c/include"

# cd "$JAVA_BRIDGE_DIR"

# javac -h "../../../$C_INCLUDE_DIR" Bridge.java && rm Bridge.class

# cd -

gcc -shared -o build/c/main.dll $C_SOURCE_FILES -I"$JAVA_HOME/include" -I"$JAVA_HOME/include/win32" -lws2_32 -liphlpapi

find java -name "*.java" | xargs javac -d build/java