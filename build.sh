#!/bin/bash
set -e  # Exit immediately if a command fails
set -o pipefail  # Fail if any command in a pipeline fails

# Default arguments
PRESET="release"
RUN_TESTS=false
CLEAN_BUILD=false

# Parse arguments
for arg in "$@"; do
    case $arg in
        release|debug|profile)
            PRESET="$arg"
            ;;
        --test)
            RUN_TESTS=true
            ;;
        --clean)
            CLEAN_BUILD=true
            ;;
        *)
            echo "Unknown argument: $arg"
            echo "Usage: $0 [release|debug|profile] [--test] [--clean]"
            exit 1
            ;;
    esac
done

BUILD_DIR="build/$PRESET"

print_separator() {
    echo "===================================="
}

if [ "$CLEAN_BUILD" = true ]; then
    print_separator
    echo "Cleaning the build directory: $BUILD_DIR"
    echo ""
    rm -rf "$BUILD_DIR"
fi

print_separator
echo "Configuring the project using ninja-$PRESET preset..."
echo ""
cmake --preset ninja-$PRESET
echo ""

print_separator
echo "Building the project using $PRESET preset..."
echo ""
cmake --build --preset $PRESET
echo ""

if [ "$RUN_TESTS" = true ]; then
    print_separator
    echo "Running tests using $PRESET preset..."
    echo ""
    ctest --preset $PRESET --output-on-failure
    echo ""
fi

print_separator
echo "All done!"
