# C++ File System Implementation

A complete implementation of a simple, Unix-like file system on a simulated disk.

## 🚀 Features

### ✅ **Fully Implemented Commands**
- **`mkdir <directory>`** - Create empty subdirectory
- **`ls`** - List directory contents (directories show with `/`)
- **`cd <directory>`** - Change to specified subdirectory
- **`home`** - Switch to root directory
- **`rmdir <directory>`** - Remove empty subdirectory
- **`create <filename>`** - Create empty file
- **`append <filename> <data>`** - Append data to file
- **`stat <name>`** - Display file/directory statistics
- **`cat <filename>`** - Print file contents
- **`tail <filename> <n>`** - Print last n bytes of file
- **`rm <filename>`** - Remove file

### 🔧 **Advanced Features**
- **Multi-block file support** - Files can span multiple 128-byte blocks
- **Smart block allocation** - Fills last block before allocating new ones
- **Data persistence** - File system state persists across sessions
- **Comprehensive error handling** - All required error messages implemented
- **Efficient resource management** - Proper cleanup and block reclamation

## 🏗️ Architecture

The project follows a layered architecture:

```
Shell (CLI) → FileSys (Commands) → BasicFileSys (Low-level) → Disk (Storage)
```

### **Block Structure**
- **Block Size**: 128 bytes
- **Total Blocks**: 1,024 (0-1023)
- **Block Types**:
  - Block 0: Superblock (bitmap)
  - Block 1: Root directory
  - Other blocks: Dynamic allocation

### **Data Structures**
- **Directory Block**: Magic number + entry count + name/block pairs
- **Inode Block**: Magic number + file size + data block pointers
- **Data Block**: Raw file data (128 bytes)

## 📋 Requirements

- **Compiler**: C++ compatible compiler (g++, clang++)
- **Make**: GNU Make
- **Platform**: Linux/Unix

## 🚀 Quick Start

### **1. Clone the Repository**
```bash
git clone https://github.com/Shan533/C-File-System.git
cd C-File-System
```

### **2. Navigate to Source Directory**
```bash
cd "file system skeleton code"
```

### **3. Build the Project**
```bash
make
```

### **4. Run the File System**
```bash
# Interactive mode
./filesys

# Script mode
./filesys -s test_script.txt
```

## 📖 Usage Examples

### **Basic Operations**
```bash
# Create and navigate directories
mkdir testdir
cd testdir
ls
home

# Create and manipulate files
create myfile
append myfile "Hello, World!"
cat myfile
stat myfile
tail myfile 5

# Clean up
rm myfile
cd ..
rmdir testdir
```

### **Multi-block File Example**
```bash
# Create a large file that spans multiple blocks
create bigfile
append bigfile "This is a long string that will exceed 128 bytes and require multiple blocks to store properly in our file system implementation."
stat bigfile
cat bigfile
```

## 🧪 Testing

The project includes comprehensive test scripts:

- **`test_script.txt`** - Basic functionality test
- **`test_large_files.txt`** - Multi-block file testing
- **`test_edge_cases.txt`** - Edge case testing
- **`test_persistence1.txt`** - Persistence testing
- **`test_newline.txt`** - Special character handling

### **Run Tests**
```bash
# Run basic test
./filesys -s test_script.txt

# Run large file test
./filesys -s test_large_files.txt

# Run edge case test
./filesys -s test_edge_cases.txt
```
