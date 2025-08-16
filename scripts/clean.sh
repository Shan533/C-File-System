#!/bin/bash

# C++ File System Clean Script
# This script removes all build artifacts

echo "Cleaning build artifacts..."

# Remove object files
rm -rf build/obj/*.o

# Remove executable
rm -f build/bin/filesys

# Remove disk file
rm -f build/disk/DISK

echo "Clean complete!"
