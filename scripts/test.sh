#!/bin/bash

# C++ File System Test Script
# This script runs all test cases

echo "Running C++ File System Tests..."

# Check if executable exists
if [ ! -f "build/bin/filesys" ]; then
    echo "Error: Executable not found. Please run build.sh first."
    exit 1
fi

# Run all test scripts
echo "Running basic functionality test..."
./build/bin/filesys -s tests/scripts/test_script.txt

echo "Running large file test..."
./build/bin/filesys -s tests/scripts/test_large_files.txt

echo "Running edge cases test..."
./build/bin/filesys -s tests/scripts/test_edge_cases.txt

echo "Running persistence test..."
./build/bin/filesys -s tests/scripts/test_persistence1.txt

echo "Running newline handling test..."
./build/bin/filesys -s tests/scripts/test_newline.txt

echo "All tests completed!"
