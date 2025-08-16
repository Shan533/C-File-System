// Computing Systems: main
// Executes the file system program by starting the shell.

#include <iostream>
#include <cstring>
using namespace std;

#include "Shell.h"
#include "Blocks.h"

int main(int argc, char **argv)
{
  // Uncomment this section to make sure the size of the blocks are
  // equal to the block size.
#if 0
  cout << "superblock size: " << sizeof(struct superblock_t) << endl;
  cout << "dirblock size: " << sizeof(struct dirblock_t) << endl;
  cout << "inode size: " <<  sizeof(struct inode_t) << endl;
  cout << "datablock size: " << sizeof(struct datablock_t) << endl;
#endif

  Shell shell;

  if (argc == 1) {
    shell.run();
  }
  else if (argc == 3 && strcmp(argv[1], "-s") == 0) {
    shell.run_script(argv[2]);
  }
  else {
    cerr << "Invalid command line" << endl;
    cerr << "Usage (one of the following): " << endl;
    cerr << "./filesys" << endl;
    cerr << "./filesys -s <script-name> " << endl;
  }

  return 0;
}
