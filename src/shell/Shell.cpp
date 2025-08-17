// Computing Systems: Shell
// Implements a basic shell (command line interface) for the file system

#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#include "Shell.h"

static const string PROMPT_STRING = "FS> ";	// shell prompt

// Executes the shell until the user quits.
void Shell::run()
{
  // mount the file system
  filesys.mount();
  
  // continue until the user quits
  bool user_quit = false;
  while (!user_quit) {

    // print prompt and get command line
    string command_str;
    cout << PROMPT_STRING;
    getline(cin, command_str);

    // execute the command
    user_quit = execute_command(command_str);
  }

  // unmount the file system
  filesys.unmount();
}

// Execute a script.
void Shell::run_script(char *file_name)
{
  // open script file
  ifstream infile;
  infile.open(file_name);
  if (infile.fail()) {
    cerr << "Could not open script file" << endl;
    return;
  }

  // mount the file system
  filesys.mount();

  // execute each line in the script
  bool user_quit = false;
  string command_str;
  getline(infile, command_str, '\n');
  while (!infile.eof() && !user_quit) {
    cout << PROMPT_STRING << command_str << endl;
    user_quit = execute_command(command_str);
    getline(infile, command_str);
  }

  // clean up
  filesys.unmount();
  infile.close();
}

// Executes the command. Returns true for quit and false otherwise.
bool Shell::execute_command(string command_str)
{
  // parse the command line
  struct Command command = parse_command(command_str);

  // look for the matching command
  if (command.name == "") {
    return false;
  }
  else if (command.name == "mkdir") {
    filesys.mkdir(command.file_name.c_str());
  }
  else if (command.name == "cd") {
    filesys.cd(command.file_name.c_str());
  }
  else if (command.name == "home") {
    filesys.home();
  }
  else if (command.name == "rmdir") {
    filesys.rmdir(command.file_name.c_str());
  }
  else if (command.name == "ls") {
    filesys.ls();
  }
  else if (command.name == "create") {
    filesys.create(command.file_name.c_str());
  }
  else if (command.name == "append") {
    filesys.append(command.file_name.c_str(), command.append_data.c_str());
  }
  else if (command.name == "cat") {
    filesys.cat(command.file_name.c_str());
  }
  else if (command.name == "tail") {
    errno = 0;
    unsigned long n = strtoul(command.append_data.c_str(), NULL, 0);
    if (0 == errno) {
      filesys.tail(command.file_name.c_str(), n);
    } else {
      cerr << "Invalid command line: " << command.append_data;
      cerr << " is not a valid number of bytes" << endl;
      return false;
    }
  }
  else if (command.name == "rm") {
    filesys.rm(command.file_name.c_str());
  }
  else if (command.name == "stat") {
    filesys.stat(command.file_name.c_str());
  }
  else if (command.name == "pwd") {
    filesys.pwd();
  }
  else if (command.name == "df") {
    filesys.df();
  }
  else if (command.name == "head") {
    errno = 0;
    unsigned long n = strtoul(command.append_data.c_str(), NULL, 0);
    if (0 == errno) {
      filesys.head(command.file_name.c_str(), n);
    } else {
      cerr << "Invalid command line: " << command.append_data;
      cerr << " is not a valid number of bytes" << endl;
      return false;
    }
  }
  else if (command.name == "wc") {
    filesys.wc(command.file_name.c_str());
  }
  else if (command.name == "cp") {
    filesys.cp(command.file_name.c_str(), command.append_data.c_str());
  }
  else if (command.name == "mv") {
    filesys.mv(command.file_name.c_str(), command.append_data.c_str());
  }
  else if (command.name == "find") {
    filesys.find(command.file_name.c_str());
  }
  else if (command.name == "tree") {
    filesys.tree();
  }
  else if (command.name == "help") {
    if (command.file_name.empty()) {
      filesys.help();
    } else {
      filesys.help(command.file_name.c_str());
    }
  }
  else if (command.name == "quit") {
    return true;
  }

  return false;
}

// Parses a command line into a command struct. Returned name is blank
// for invalid command lines.
Shell::Command Shell::parse_command(string command_str)
{
  // empty command struct returned for errors
  struct Command empty = {"", "", ""};

  // grab each of the tokens (if they exist)
  struct Command command;
  istringstream ss(command_str);
  int num_tokens = 0;
  
  if (ss >> command.name) {
    num_tokens++;
    if (ss >> command.file_name) {
      num_tokens++;
      
      // For append command, handle quoted strings
      if (command.name == "append") {
        // Get the rest of the line for append data
        string rest_of_line;
        getline(ss, rest_of_line);
        
        // Trim leading whitespace
        size_t start = rest_of_line.find_first_not_of(" \t");
        if (start != string::npos) {
          rest_of_line = rest_of_line.substr(start);
          
          // Check if it's quoted
          if (rest_of_line.length() >= 2 && rest_of_line[0] == '"' && rest_of_line.back() == '"') {
            // Remove quotes
            command.append_data = rest_of_line.substr(1, rest_of_line.length() - 2);
            num_tokens++;
          } else if (!rest_of_line.empty()) {
            // Not quoted, use as is
            command.append_data = rest_of_line;
            num_tokens++;
          }
        }
      } else {
        // For other commands, use normal parsing
        if (ss >> command.append_data) {
          num_tokens++;
          string junk;
          if (ss >> junk) {
            num_tokens++;
          }
        }
      }
    }
  }

  // Check for empty command line
  if (num_tokens == 0) {
    return empty;
  }
    
  // Check for invalid command lines
  if (command.name == "ls" ||
      command.name == "home" ||
      command.name == "pwd" ||
      command.name == "df" ||
      command.name == "tree" ||
      command.name == "help" ||
      command.name == "quit")
  {
    if (num_tokens != 1 && !(command.name == "help" && num_tokens == 2)) {
      cerr << "Invalid command line: " << command.name;
      cerr << " has improper number of arguments" << endl;
      return empty;
    }
  }
  else if (command.name == "mkdir" ||
      command.name == "cd"    ||
      command.name == "rmdir" ||
      command.name == "create"||
      command.name == "cat"   ||
      command.name == "rm"    ||
      command.name == "stat"  ||
      command.name == "wc"    ||
      command.name == "find")
  {
    if (num_tokens != 2) {
      cerr << "Invalid command line: " << command.name;
      cerr << " has improper number of arguments" << endl;
      return empty;
    }
  }
  else if (command.name == "append" || 
           command.name == "tail" ||
           command.name == "head" ||
           command.name == "cp" ||
           command.name == "mv")
  {
    if (num_tokens != 3) {
      cerr << "Invalid command line: " << command.name;
      cerr << " has improper number of arguments" << endl;
      return empty;
    }
  }
  else {
    cerr << "Invalid command line: " << command.name;
    cerr << " is not a command" << endl; 
    return empty;
  } 

  return command;
}

