MAIN_CLASS="App"

cd "$(dirname "$0")"

# Run the Java application
java -Djava.library.path="build/c" -cp ./build/java $MAIN_CLASS