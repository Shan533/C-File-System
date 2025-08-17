// Computing Systems: File System
// Implements the file system commands that are available to the shell.

#ifndef FILESYS_H
#define FILESYS_H

#include <string>
#include "BasicFileSys.h"

using namespace std;

class FileSys {
  
  public:
    // mounts the file system
    void mount();

    // unmounts the file system
    void unmount();

    // make a directory
    void mkdir(const char *name);

    // switch to a directory
    void cd(const char *name);
    
    // switch to home directory
    void home();
    
    // remove a directory
    void rmdir(const char *name);

    // list the contents of current directory
    void ls();

    // create an empty data file
    void create(const char *name);

    // append data to a data file
    void append(const char *name, const char *data);

    // display the contents of a data file
    void cat(const char *name);

    // display the last N bytes of the file
    void tail(const char *name, unsigned int n);

    // delete a data file
    void rm(const char *name);

    // display stats about file or directory
    void stat(const char *name);

    // print working directory
    void pwd();

    // show disk free space
    void df();

    // show first N bytes of file
    void head(const char *name, unsigned int n);

    // show word count (lines, words, bytes)
    void wc(const char *name);

    // copy file
    void cp(const char *src, const char *dest);

    // move/rename file
    void mv(const char *src, const char *dest);

    // find files/directories by name
    void find(const char *name);

    // display directory tree
    void tree();

    // show help information
    void help();
    void help(const char *command);

  private:
    BasicFileSys bfs;	// basic file system
    short curr_dir;	// current directory
    string current_path;  // track current directory path

    // Helper functions
    int find_file(const char *name, short &block_num);
    bool is_directory(short block_num);
    bool is_file(short block_num);
    void find_recursive(const char *name, short dir_block, const string& path);
    void tree_recursive(short dir_block, const string& prefix, bool is_last);
};

#endif 
