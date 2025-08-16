// Computing Systems: Shell
// Implements a basic shell (command line interface) for the file system

#ifndef SHELL_H
#define SHELL_H

#include <string>
#include "FileSys.h"

// Shell
class Shell {

  public:
    // Executes the shell until the user quits.
    void run();

    // Execute a script.
    void run_script(char *file_name);

  private:
    FileSys filesys;  // file system

    // data structure for command line
    struct Command
    {
      string name;		// name of command
      string file_name;		// name of file
      string append_data;	// append data (append only)
    };

    // Executes the command. Returns true for quit and false otherwise.
    bool execute_command(string command_str);

    // Parses a command line into a command struct. Returned name is blank
    // for invalid command lines.
    struct Command parse_command(string command_str);
};

#endif
  
