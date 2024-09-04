
cd "$(dirname "$0")/build"

if [ -d "c" ]; then
    echo "Cleaning c directory..."
    rm -rf c/*
fi

if [ -d "java" ]; then
    echo "Cleaning java directory..."
    rm -rf java/*
fi

echo "Cleanup complete."
