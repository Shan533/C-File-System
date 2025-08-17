// Computing Systems: File System
// Implements the file system commands that are available to the shell.

#include <cstring>
#include <iostream>
using namespace std;

#include "FileSys.h"
#include "BasicFileSys.h"
#include "Blocks.h"

// mounts the file system
void FileSys::mount() {
  bfs.mount();
  curr_dir = 1;
  current_path = "/";  // initialize current path to root
}

// unmounts the file system
void FileSys::unmount() {
  bfs.unmount();
}

// make a directory
void FileSys::mkdir(const char *name)
{
    // Check if name is too long
    if (strlen(name) > MAX_FNAME_SIZE) {
        cout << "File name is too long" << endl;
        return;
    }
    
    // Check if file already exists
    short existing_block;
    if (find_file(name, existing_block) != -1) {
        cout << "File exists" << endl;
        return;
    }
    
    // Read current directory
    dirblock_t dir_block;
    bfs.read_block(curr_dir, &dir_block);
    
    // Check if directory is full
    if (dir_block.num_entries >= MAX_DIR_ENTRIES) {
        cout << "Directory is full" << endl;
        return;
    }
    
    // Get a free block for the new directory
    short new_block = bfs.get_free_block();
    if (new_block == 0) {
        cout << "Disk is full" << endl;
        return;
    }
    
    // Initialize new directory block
    dirblock_t new_dir;
    new_dir.magic = DIR_MAGIC_NUM;
    new_dir.num_entries = 0;
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        new_dir.dir_entries[i].block_num = 0;
        new_dir.dir_entries[i].name[0] = '\0';
    }
    
    // Write new directory block to disk
    bfs.write_block(new_block, &new_dir);
    
    // Add entry to current directory
    int free_entry = -1;
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (dir_block.dir_entries[i].block_num == 0) {
            free_entry = i;
            break;
        }
    }
    
    strcpy(dir_block.dir_entries[free_entry].name, name);
    dir_block.dir_entries[free_entry].block_num = new_block;
    dir_block.num_entries++;
    
    // Write updated current directory
    bfs.write_block(curr_dir, &dir_block);
}

// switch to a directory
void FileSys::cd(const char *name)
{
    short block_num;
    if (find_file(name, block_num) == -1) {
        cout << "File does not exist" << endl;
        return;
    }
    
    if (!is_directory(block_num)) {
        cout << "File is not a directory" << endl;
        return;
    }
    
    curr_dir = block_num;
    
    // Update current path
    if (current_path == "/") {
        current_path = "/" + string(name);
    } else {
        current_path = current_path + "/" + string(name);
    }
}

// switch to home directory
void FileSys::home() {
    curr_dir = 1; // root directory is block 1
    current_path = "/";  // reset path to root
}

// remove a directory
void FileSys::rmdir(const char *name)
{
    short block_num;
    int entry_index = find_file(name, block_num);
    if (entry_index == -1) {
        cout << "File does not exist" << endl;
        return;
    }
    
    if (!is_directory(block_num)) {
        cout << "File is not a directory" << endl;
        return;
    }
    
    // Check if directory is empty
    dirblock_t target_dir;
    bfs.read_block(block_num, &target_dir);
    
    if (target_dir.num_entries > 0) {
        cout << "Directory is not empty" << endl;
        return;
    }
    
    // Reclaim directory block
    bfs.reclaim_block(block_num);
    
    // Remove entry from parent directory
    dirblock_t parent_dir;
    bfs.read_block(curr_dir, &parent_dir);
    
    parent_dir.dir_entries[entry_index].block_num = 0;
    parent_dir.dir_entries[entry_index].name[0] = '\0';
    parent_dir.num_entries--;
    
    // Write updated parent directory
    bfs.write_block(curr_dir, &parent_dir);
}

// list the contents of current directory
void FileSys::ls()
{
    dirblock_t dir_block;
    bfs.read_block(curr_dir, &dir_block);
    
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (dir_block.dir_entries[i].block_num != 0) {
            cout << dir_block.dir_entries[i].name;
            if (is_directory(dir_block.dir_entries[i].block_num)) {
                cout << "/";
            }
            cout << endl;
        }
    }
}

// create an empty data file
void FileSys::create(const char *name)
{
    // Check if name is too long
    if (strlen(name) > MAX_FNAME_SIZE) {
        cout << "File name is too long" << endl;
        return;
    }
    
    // Check if file already exists
    short existing_block;
    if (find_file(name, existing_block) != -1) {
        cout << "File exists" << endl;
        return;
    }
    
    // Read current directory
    dirblock_t dir_block;
    bfs.read_block(curr_dir, &dir_block);
    
    // Check if directory is full
    if (dir_block.num_entries >= MAX_DIR_ENTRIES) {
        cout << "Directory is full" << endl;
        return;
    }
    
    // Get a free block for the inode
    short inode_block = bfs.get_free_block();
    if (inode_block == 0) {
        cout << "Disk is full" << endl;
        return;
    }
    
    // Initialize inode
    inode_t inode;
    inode.magic = INODE_MAGIC_NUM;
    inode.size = 0;
    for (int i = 0; i < MAX_DATA_BLOCKS; i++) {
        inode.blocks[i] = 0;
    }
    
    // Write inode to disk
    bfs.write_block(inode_block, &inode);
    
    // Add entry to current directory
    int free_entry = -1;
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (dir_block.dir_entries[i].block_num == 0) {
            free_entry = i;
            break;
        }
    }
    
    strcpy(dir_block.dir_entries[free_entry].name, name);
    dir_block.dir_entries[free_entry].block_num = inode_block;
    dir_block.num_entries++;
    
    // Write updated current directory
    bfs.write_block(curr_dir, &dir_block);
}

// append data to a data file
void FileSys::append(const char *name, const char *data)
{
    short block_num;
    if (find_file(name, block_num) == -1) {
        cout << "File does not exist" << endl;
        return;
    }
    
    if (!is_file(block_num)) {
        cout << "File is a directory" << endl;
        return;
    }
    
    inode_t inode;
    bfs.read_block(block_num, &inode);
    
    unsigned int data_len = strlen(data);
    
    // Check if append would exceed maximum file size
    if (inode.size + data_len > MAX_FILE_SIZE) {
        cout << "Append exceeds maximum file size" << endl;
        return;
    }
    
    unsigned int data_pos = 0;
    
    // Find last block with data
    int last_block_index = -1;
    for (int i = 0; i < MAX_DATA_BLOCKS; i++) {
        if (inode.blocks[i] != 0) {
            last_block_index = i;
        }
    }
    
    // If file has data, try to fill the last block first
    if (last_block_index >= 0) {
        datablock_t last_block;
        bfs.read_block(inode.blocks[last_block_index], &last_block);
        
        unsigned int bytes_in_last_block = inode.size % BLOCK_SIZE;
        if (bytes_in_last_block == 0) {
            bytes_in_last_block = BLOCK_SIZE;
        }
        
        unsigned int space_left = BLOCK_SIZE - bytes_in_last_block;
        if (space_left > 0) {
            unsigned int bytes_to_copy = (data_len < space_left) ? data_len : space_left;
            memcpy(&last_block.data[bytes_in_last_block], data, bytes_to_copy);
            bfs.write_block(inode.blocks[last_block_index], &last_block);
            
            data_pos += bytes_to_copy;
            inode.size += bytes_to_copy;
        }
    }
    
    // Append remaining data in new blocks
    while (data_pos < data_len) {
        // Find next free slot in inode
        int free_slot = -1;
        for (int i = 0; i < MAX_DATA_BLOCKS; i++) {
            if (inode.blocks[i] == 0) {
                free_slot = i;
                break;
            }
        }
        
        if (free_slot == -1) {
            cout << "Append exceeds maximum file size" << endl;
            return;
        }
        
        // Get new data block
        short new_block = bfs.get_free_block();
        if (new_block == 0) {
            cout << "Disk is full" << endl;
            return;
        }
        
        // Fill the new block
        datablock_t new_data_block;
        unsigned int bytes_left = data_len - data_pos;
        unsigned int bytes_to_copy = (bytes_left < BLOCK_SIZE) ? bytes_left : BLOCK_SIZE;
        
        memcpy(new_data_block.data, &data[data_pos], bytes_to_copy);
        
        // Clear remaining bytes in block if not full
        for (unsigned int i = bytes_to_copy; i < BLOCK_SIZE; i++) {
            new_data_block.data[i] = 0;
        }
        
        bfs.write_block(new_block, &new_data_block);
        inode.blocks[free_slot] = new_block;
        
        data_pos += bytes_to_copy;
        inode.size += bytes_to_copy;
    }
    
    // Write updated inode
    bfs.write_block(block_num, &inode);
}

// display the contents of a data file
void FileSys::cat(const char *name)
{
    short block_num;
    if (find_file(name, block_num) == -1) {
        cout << "File does not exist" << endl;
        return;
    }
    
    if (!is_file(block_num)) {
        cout << "File is a directory" << endl;
        return;
    }
    
    inode_t inode;
    bfs.read_block(block_num, &inode);
    
    unsigned int bytes_left = inode.size;
    for (int i = 0; i < MAX_DATA_BLOCKS && bytes_left > 0; i++) {
        if (inode.blocks[i] != 0) {
            datablock_t data_block;
            bfs.read_block(inode.blocks[i], &data_block);
            
            unsigned int bytes_to_print = (bytes_left > BLOCK_SIZE) ? BLOCK_SIZE : bytes_left;
            for (unsigned int j = 0; j < bytes_to_print; j++) {
                cout << data_block.data[j];
            }
            bytes_left -= bytes_to_print;
        }
    }
    cout << endl;
}

// display the last N bytes of the file
void FileSys::tail(const char *name, unsigned int n)
{
    short block_num;
    if (find_file(name, block_num) == -1) {
        cout << "File does not exist" << endl;
        return;
    }
    
    if (!is_file(block_num)) {
        cout << "File is a directory" << endl;
        return;
    }
    
    inode_t inode;
    bfs.read_block(block_num, &inode);
    
    if (inode.size == 0) {
        return; // Empty file, nothing to print
    }
    
    unsigned int bytes_to_print = (n > inode.size) ? inode.size : n;
    unsigned int start_pos = inode.size - bytes_to_print;
    
    // Find which block contains the start position
    unsigned int current_pos = 0;
    for (int i = 0; i < MAX_DATA_BLOCKS; i++) {
        if (inode.blocks[i] != 0) {
            unsigned int block_end = current_pos + BLOCK_SIZE;
            if (start_pos < block_end) {
                // This block contains part of what we need to print
                datablock_t data_block;
                bfs.read_block(inode.blocks[i], &data_block);
                
                unsigned int offset_in_block = (start_pos > current_pos) ? start_pos - current_pos : 0;
                unsigned int bytes_in_this_block = BLOCK_SIZE - offset_in_block;
                
                // Don't print beyond file size or beyond what we need
                unsigned int file_bytes_left = inode.size - (current_pos + offset_in_block);
                if (bytes_in_this_block > file_bytes_left) {
                    bytes_in_this_block = file_bytes_left;
                }
                if (bytes_in_this_block > bytes_to_print) {
                    bytes_in_this_block = bytes_to_print;
                }
                
                for (unsigned int j = 0; j < bytes_in_this_block; j++) {
                    cout << data_block.data[offset_in_block + j];
                }
                
                bytes_to_print -= bytes_in_this_block;
                if (bytes_to_print == 0) {
                    break;
                }
            }
            current_pos = block_end;
        }
    }
    cout << endl;
}

// delete a data file
void FileSys::rm(const char *name)
{
    short block_num;
    int entry_index = find_file(name, block_num);
    if (entry_index == -1) {
        cout << "File does not exist" << endl;
        return;
    }
    
    if (!is_file(block_num)) {
        cout << "File is a directory" << endl;
        return;
    }
    
    // Read inode and reclaim all data blocks
    inode_t inode;
    bfs.read_block(block_num, &inode);
    
    for (int i = 0; i < MAX_DATA_BLOCKS; i++) {
        if (inode.blocks[i] != 0) {
            bfs.reclaim_block(inode.blocks[i]);
        }
    }
    
    // Reclaim inode block
    bfs.reclaim_block(block_num);
    
    // Remove entry from directory
    dirblock_t dir_block;
    bfs.read_block(curr_dir, &dir_block);
    
    dir_block.dir_entries[entry_index].block_num = 0;
    dir_block.dir_entries[entry_index].name[0] = '\0';
    dir_block.num_entries--;
    
    // Write updated directory
    bfs.write_block(curr_dir, &dir_block);
}

// display stats about file or directory
void FileSys::stat(const char *name)
{
    short block_num;
    if (find_file(name, block_num) == -1) {
        cout << "File does not exist" << endl;
        return;
    }
    
    if (is_directory(block_num)) {
        cout << "Directory name: " << name << "/" << endl;
        cout << "Directory block: " << block_num << endl;
    } else if (is_file(block_num)) {
        inode_t inode;
        bfs.read_block(block_num, &inode);
        
        cout << "Inode block: " << block_num << endl;
        cout << "Bytes in file: " << inode.size << endl;
        
        // Count number of data blocks
        int num_blocks = 0;
        short first_block = 0;
        for (int i = 0; i < MAX_DATA_BLOCKS; i++) {
            if (inode.blocks[i] != 0) {
                if (first_block == 0) {
                    first_block = inode.blocks[i];
                }
                num_blocks++;
            }
        }
        
        cout << "Number of blocks: " << num_blocks << endl;
        cout << "First block: " << first_block << endl;
    }
}

// HELPER FUNCTIONS

// Find a file/directory by name in current directory
// Returns entry index if found, -1 if not found
// Sets block_num to the block number of the file/directory
int FileSys::find_file(const char *name, short &block_num) {
    dirblock_t dir_block;
    bfs.read_block(curr_dir, &dir_block);
    
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (dir_block.dir_entries[i].block_num != 0 && 
            strcmp(dir_block.dir_entries[i].name, name) == 0) {
            block_num = dir_block.dir_entries[i].block_num;
            return i;
        }
    }
    block_num = 0;
    return -1;
}

// Check if a block is a directory
bool FileSys::is_directory(short block_num) {
    dirblock_t block;
    bfs.read_block(block_num, &block);
    return block.magic == DIR_MAGIC_NUM;
}

// Check if a block is a file (inode)
bool FileSys::is_file(short block_num) {
    inode_t block;
    bfs.read_block(block_num, &block);
    return block.magic == INODE_MAGIC_NUM;
}

// NEW COMMANDS IMPLEMENTATION

// print working directory
void FileSys::pwd() {
    cout << current_path << endl;
}

// show disk free space
void FileSys::df() {
    superblock_t superblock;
    bfs.read_block(0, &superblock);
    
    int free_blocks = 0;
    for (int i = 0; i < NUM_BLOCKS; i++) {
        if (!(superblock.bitmap[i/8] & (1 << (i%8)))) {
            free_blocks++;
        }
    }
    
    int used_blocks = NUM_BLOCKS - free_blocks;
    int use_percent = (used_blocks * 100) / NUM_BLOCKS;
    
    cout << "Filesystem     Total    Used    Free   Use%" << endl;
    cout << "/dev/disk      " << NUM_BLOCKS << "     " 
         << used_blocks << "     " << free_blocks 
         << "    " << use_percent << "%" << endl;
}

// show first N bytes of file
void FileSys::head(const char *name, unsigned int n) {
    short block_num;
    if (find_file(name, block_num) == -1) {
        cout << "File does not exist" << endl;
        return;
    }
    
    if (!is_file(block_num)) {
        cout << "File is a directory" << endl;
        return;
    }
    
    inode_t inode;
    bfs.read_block(block_num, &inode);
    
    if (inode.size == 0 || n == 0) {
        return; // Empty file or nothing to print
    }
    
    unsigned int bytes_to_print = (n > inode.size) ? inode.size : n;
    unsigned int bytes_printed = 0;
    
    // Read data blocks sequentially until N bytes printed
    for (int i = 0; i < MAX_DATA_BLOCKS && bytes_printed < bytes_to_print; i++) {
        if (inode.blocks[i] != 0) {
            datablock_t data_block;
            bfs.read_block(inode.blocks[i], &data_block);
            
            unsigned int bytes_in_this_block = (bytes_to_print - bytes_printed > BLOCK_SIZE) ? 
                                               BLOCK_SIZE : bytes_to_print - bytes_printed;
            
            for (unsigned int j = 0; j < bytes_in_this_block; j++) {
                cout << data_block.data[j];
                bytes_printed++;
            }
        }
    }
    cout << endl;
}

// show word count (lines, words, bytes)
void FileSys::wc(const char *name) {
    short block_num;
    if (find_file(name, block_num) == -1) {
        cout << "File does not exist" << endl;
        return;
    }
    
    if (!is_file(block_num)) {
        cout << "File is a directory" << endl;
        return;
    }
    
    inode_t inode;
    bfs.read_block(block_num, &inode);
    
    unsigned int lines = 0;
    unsigned int words = 0;
    unsigned int bytes = inode.size;
    bool in_word = false;
    
    // Read file content and count
    unsigned int bytes_read = 0;
    for (int i = 0; i < MAX_DATA_BLOCKS && bytes_read < inode.size; i++) {
        if (inode.blocks[i] != 0) {
            datablock_t data_block;
            bfs.read_block(inode.blocks[i], &data_block);
            
            unsigned int bytes_in_this_block = (inode.size - bytes_read > BLOCK_SIZE) ? 
                                               BLOCK_SIZE : inode.size - bytes_read;
            
            for (unsigned int j = 0; j < bytes_in_this_block; j++) {
                char c = data_block.data[j];
                
                if (c == '\n') {
                    lines++;
                }
                
                if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                    if (in_word) {
                        words++;
                        in_word = false;
                    }
                } else {
                    in_word = true;
                }
                
                bytes_read++;
            }
        }
    }
    
    // Count last word if file doesn't end with whitespace
    if (in_word) {
        words++;
    }
    
    cout << lines << " " << words << " " << bytes << " " << name << endl;
}

// copy file
void FileSys::cp(const char *src, const char *dest) {
    // Check if source exists
    short src_block;
    if (find_file(src, src_block) == -1) {
        cout << "File does not exist" << endl;
        return;
    }
    
    if (!is_file(src_block)) {
        cout << "File is a directory" << endl;
        return;
    }
    
    // Check if destination already exists
    short dest_block;
    if (find_file(dest, dest_block) != -1) {
        cout << "File exists" << endl;
        return;
    }
    
    // Check if destination name is too long
    if (strlen(dest) > MAX_FNAME_SIZE) {
        cout << "File name is too long" << endl;
        return;
    }
    
    // Read source inode
    inode_t src_inode;
    bfs.read_block(src_block, &src_inode);
    
    // Create new inode for destination
    short dest_inode_block = bfs.get_free_block();
    if (dest_inode_block == 0) {
        cout << "Disk is full" << endl;
        return;
    }
    
    inode_t dest_inode;
    dest_inode.magic = INODE_MAGIC_NUM;
    dest_inode.size = src_inode.size;
    
    // Copy data blocks
    for (int i = 0; i < MAX_DATA_BLOCKS; i++) {
        if (src_inode.blocks[i] != 0) {
            // Get new block for destination
            short new_data_block = bfs.get_free_block();
            if (new_data_block == 0) {
                cout << "Disk is full" << endl;
                // Clean up already allocated blocks
                for (int j = 0; j < i; j++) {
                    if (dest_inode.blocks[j] != 0) {
                        bfs.reclaim_block(dest_inode.blocks[j]);
                    }
                }
                bfs.reclaim_block(dest_inode_block);
                return;
            }
            
            // Copy data
            datablock_t data_block;
            bfs.read_block(src_inode.blocks[i], &data_block);
            bfs.write_block(new_data_block, &data_block);
            
            dest_inode.blocks[i] = new_data_block;
        } else {
            dest_inode.blocks[i] = 0;
        }
    }
    
    // Write destination inode
    bfs.write_block(dest_inode_block, &dest_inode);
    
    // Add directory entry
    dirblock_t dir_block;
    bfs.read_block(curr_dir, &dir_block);
    
    if (dir_block.num_entries >= MAX_DIR_ENTRIES) {
        cout << "Directory is full" << endl;
        // Clean up
        for (int i = 0; i < MAX_DATA_BLOCKS; i++) {
            if (dest_inode.blocks[i] != 0) {
                bfs.reclaim_block(dest_inode.blocks[i]);
            }
        }
        bfs.reclaim_block(dest_inode_block);
        return;
    }
    
    // Find free entry
    int free_entry = -1;
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (dir_block.dir_entries[i].block_num == 0) {
            free_entry = i;
            break;
        }
    }
    
    strcpy(dir_block.dir_entries[free_entry].name, dest);
    dir_block.dir_entries[free_entry].block_num = dest_inode_block;
    dir_block.num_entries++;
    
    bfs.write_block(curr_dir, &dir_block);
}

// move/rename file
void FileSys::mv(const char *src, const char *dest) {
    // Check if source exists
    short src_block;
    int src_entry = find_file(src, src_block);
    if (src_entry == -1) {
        cout << "File does not exist" << endl;
        return;
    }
    
    // Check if destination already exists
    short dest_block;
    if (find_file(dest, dest_block) != -1) {
        cout << "File exists" << endl;
        return;
    }
    
    // Check if destination name is too long
    if (strlen(dest) > MAX_FNAME_SIZE) {
        cout << "File name is too long" << endl;
        return;
    }
    
    // Update directory entry name
    dirblock_t dir_block;
    bfs.read_block(curr_dir, &dir_block);
    
    strcpy(dir_block.dir_entries[src_entry].name, dest);
    
    bfs.write_block(curr_dir, &dir_block);
}

// find files/directories by name
void FileSys::find(const char *name) {
    find_recursive(name, curr_dir, current_path);
}

// Helper function for recursive find
void FileSys::find_recursive(const char *name, short dir_block, const string& path) {
    dirblock_t dir;
    bfs.read_block(dir_block, &dir);
    
    // Search current directory
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (dir.dir_entries[i].block_num != 0) {
            if (strcmp(dir.dir_entries[i].name, name) == 0) {
                string full_path = (path == "/") ? "/" + string(name) : path + "/" + string(name);
                cout << full_path << endl;
            }
            
            // If it's a directory, recurse into it
            if (is_directory(dir.dir_entries[i].block_num)) {
                string subdir_path = (path == "/") ? "/" + string(dir.dir_entries[i].name) : 
                                     path + "/" + string(dir.dir_entries[i].name);
                find_recursive(name, dir.dir_entries[i].block_num, subdir_path);
            }
        }
    }
}

// display directory tree
void FileSys::tree() {
    cout << current_path << endl;
    tree_recursive(curr_dir, "", true);
}

// Helper function for recursive tree display
void FileSys::tree_recursive(short dir_block, const string& prefix, bool is_last) {
    dirblock_t dir;
    bfs.read_block(dir_block, &dir);
    
    // Count non-empty entries
    int count = 0;
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (dir.dir_entries[i].block_num != 0) {
            count++;
        }
    }
    
    int current = 0;
    for (int i = 0; i < MAX_DIR_ENTRIES; i++) {
        if (dir.dir_entries[i].block_num != 0) {
            current++;
            bool is_last_entry = (current == count);
            
            cout << prefix;
            cout << (is_last_entry ? "└── " : "├── ");
            cout << dir.dir_entries[i].name;
            
            if (is_directory(dir.dir_entries[i].block_num)) {
                cout << "/";
            }
            cout << endl;
            
            // If it's a directory, recurse
            if (is_directory(dir.dir_entries[i].block_num)) {
                string new_prefix = prefix + (is_last_entry ? "    " : "│   ");
                tree_recursive(dir.dir_entries[i].block_num, new_prefix, is_last_entry);
            }
        }
    }
}

// show help information
void FileSys::help() {
    cout << "Available commands:" << endl;
    cout << "  mkdir <dir>     - Create directory" << endl;
    cout << "  cd <dir>        - Change to directory" << endl;
    cout << "  home            - Change to root directory" << endl;
    cout << "  rmdir <dir>     - Remove empty directory" << endl;
    cout << "  ls              - List directory contents" << endl;
    cout << "  create <file>   - Create empty file" << endl;
    cout << "  append <file> <data> - Append data to file" << endl;
    cout << "  cat <file>      - Display file contents" << endl;
    cout << "  tail <file> <n> - Display last N bytes of file" << endl;
    cout << "  rm <file>       - Delete file" << endl;
    cout << "  stat <name>     - Display file/directory statistics" << endl;
    cout << "  pwd             - Print working directory" << endl;
    cout << "  df              - Display disk usage" << endl;
    cout << "  head <file> <n> - Display first N bytes of file" << endl;
    cout << "  wc <file>       - Display word count (lines, words, bytes)" << endl;
    cout << "  cp <src> <dest> - Copy file" << endl;
    cout << "  mv <src> <dest> - Move/rename file" << endl;
    cout << "  find <name>     - Find files/directories by name" << endl;
    cout << "  tree            - Display directory tree" << endl;
    cout << "  help [command]  - Show help (general or for specific command)" << endl;
    cout << "  quit            - Exit the shell" << endl;
}

void FileSys::help(const char *command) {
    string cmd(command);
    
    if (cmd == "mkdir") {
        cout << "mkdir <dir> - Create a new directory" << endl;
        cout << "  Creates a new directory with the specified name in the current directory." << endl;
    } else if (cmd == "cd") {
        cout << "cd <dir> - Change to directory" << endl;
        cout << "  Changes the current working directory to the specified directory." << endl;
    } else if (cmd == "home") {
        cout << "home - Change to root directory" << endl;
        cout << "  Changes the current working directory to the root directory (/)." << endl;
    } else if (cmd == "rmdir") {
        cout << "rmdir <dir> - Remove empty directory" << endl;
        cout << "  Removes the specified directory. The directory must be empty." << endl;
    } else if (cmd == "ls") {
        cout << "ls - List directory contents" << endl;
        cout << "  Lists all files and directories in the current directory." << endl;
        cout << "  Directories are shown with a trailing '/' character." << endl;
    } else if (cmd == "create") {
        cout << "create <file> - Create empty file" << endl;
        cout << "  Creates a new empty file with the specified name." << endl;
    } else if (cmd == "append") {
        cout << "append <file> <data> - Append data to file" << endl;
        cout << "  Appends the specified data to the end of the file." << endl;
        cout << "  Use quotes around data containing spaces: append file \"hello world\"" << endl;
    } else if (cmd == "cat") {
        cout << "cat <file> - Display file contents" << endl;
        cout << "  Displays the entire contents of the specified file." << endl;
    } else if (cmd == "tail") {
        cout << "tail <file> <n> - Display last N bytes of file" << endl;
        cout << "  Displays the last N bytes of the specified file." << endl;
    } else if (cmd == "rm") {
        cout << "rm <file> - Delete file" << endl;
        cout << "  Permanently deletes the specified file." << endl;
    } else if (cmd == "stat") {
        cout << "stat <name> - Display file/directory statistics" << endl;
        cout << "  Shows detailed information about a file or directory." << endl;
    } else if (cmd == "pwd") {
        cout << "pwd - Print working directory" << endl;
        cout << "  Displays the current working directory path." << endl;
    } else if (cmd == "df") {
        cout << "df - Display disk usage" << endl;
        cout << "  Shows filesystem usage statistics including total, used, and free blocks." << endl;
    } else if (cmd == "head") {
        cout << "head <file> <n> - Display first N bytes of file" << endl;
        cout << "  Displays the first N bytes of the specified file." << endl;
    } else if (cmd == "wc") {
        cout << "wc <file> - Display word count" << endl;
        cout << "  Shows the number of lines, words, and bytes in the file." << endl;
    } else if (cmd == "cp") {
        cout << "cp <src> <dest> - Copy file" << endl;
        cout << "  Creates a copy of the source file with the destination name." << endl;
    } else if (cmd == "mv") {
        cout << "mv <src> <dest> - Move/rename file" << endl;
        cout << "  Renames the source file to the destination name." << endl;
    } else if (cmd == "find") {
        cout << "find <name> - Find files/directories by name" << endl;
        cout << "  Searches for files and directories with the specified name" << endl;
        cout << "  starting from the current directory and all subdirectories." << endl;
    } else if (cmd == "tree") {
        cout << "tree - Display directory tree" << endl;
        cout << "  Shows the directory structure as a tree starting from current directory." << endl;
    } else if (cmd == "help") {
        cout << "help [command] - Show help" << endl;
        cout << "  Shows general help or detailed help for a specific command." << endl;
    } else if (cmd == "quit") {
        cout << "quit - Exit the shell" << endl;
        cout << "  Exits the file system shell." << endl;
    } else {
        cout << "Unknown command: " << command << endl;
        cout << "Type 'help' for a list of available commands." << endl;
    }
}

