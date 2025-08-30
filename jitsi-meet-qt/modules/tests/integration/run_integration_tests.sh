#!/bin/bash

# Module Integration Test Runner for Linux/macOS

echo "========================================"
echo "Jitsi Meet Qt Module Integration Tests"
echo "========================================"

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
BIN_DIR="$SCRIPT_DIR/bin"

# Create directories if they don't exist
mkdir -p "$BUILD_DIR"
mkdir -p "$BIN_DIR"

echo
echo "Building integration tests..."

# Check if CMake is available
if command -v cmake >/dev/null 2>&1; then
    echo "Using CMake build system..."
    cd "$BUILD_DIR"
    cmake .. -DCMAKE_BUILD_TYPE=Debug
    if [ $? -ne 0 ]; then
        echo "CMake configuration failed!"
        exit 1
    fi
    
    cmake --build . --config Debug
    if [ $? -ne 0 ]; then
        echo "CMake build failed!"
        exit 1
    fi
    
    echo
    echo "Running integration tests with CMake..."
    ctest --output-on-failure
    TEST_RESULT=$?
else
    # Fallback to qmake
    echo "CMake not found, using qmake..."
    cd "$SCRIPT_DIR"
    qmake integration_tests.pro
    if [ $? -ne 0 ]; then
        echo "qmake failed!"
        exit 1
    fi
    
    make
    if [ $? -ne 0 ]; then
        echo "Build failed!"
        exit 1
    fi
    
    echo
    echo "Running integration tests..."
    "$BIN_DIR/ModuleIntegrationTest"
    TEST_RESULT=$?
fi

echo
if [ $TEST_RESULT -eq 0 ]; then
    echo "========================================"
    echo "All integration tests PASSED!"
    echo "========================================"
else
    echo "========================================"
    echo "Some integration tests FAILED!"
    echo "========================================"
fi

# Generate test report
echo
echo "Generating test report..."
REPORT_FILE="/tmp/module_integration_test_report.json"
if [ -f "$REPORT_FILE" ]; then
    echo "Test report available at: $REPORT_FILE"
    cat "$REPORT_FILE"
fi

exit $TEST_RESULT