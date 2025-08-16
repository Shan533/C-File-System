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
}

// switch to home directory
void FileSys::home() {
    curr_dir = 1; // root directory is block 1
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

