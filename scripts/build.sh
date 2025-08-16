#!/bin/bash

# C++ File System Build Script
# This script builds the project from the new organized directory structure

echo "Building C++ File System..."

# Create build directories if they don't exist
mkdir -p build/obj
mkdir -p build/bin
mkdir -p build/disk

# Change to source directory
cd src

# Compile source files
echo "Compiling source files..."

# Compile filesystem components
g++ -c -I../src/filesystem -I../src/shell -I../src/basic -I../src/disk \
    filesystem/FileSys.cpp -o ../build/obj/FileSys.o

g++ -c -I../src/filesystem -I../src/shell -I../src/basic -I../src/disk \
    shell/Shell.cpp -o ../build/obj/Shell.o

g++ -c -I../src/filesystem -I../src/shell -I../src/basic -I../src/disk \
    basic/BasicFileSys.cpp -o ../build/obj/BasicFileSys.o

g++ -c -I../src/filesystem -I../src/shell -I../src/basic -I../src/disk \
    disk/Disk.cpp -o ../build/obj/Disk.o

g++ -c -I../src/filesystem -I../src/shell -I../src/basic -I../src/disk \
    main.cpp -o ../build/obj/main.o

# Link executable
echo "Linking executable..."
g++ -o ../build/bin/filesys \
    ../build/obj/main.o \
    ../build/obj/FileSys.o \
    ../build/obj/Shell.o \
    ../build/obj/BasicFileSys.o \
    ../build/obj/Disk.o

# Make executable
chmod +x ../build/bin/filesys

echo "Build complete! Executable: build/bin/filesys"
echo "Run with: ./build/bin/filesys"
