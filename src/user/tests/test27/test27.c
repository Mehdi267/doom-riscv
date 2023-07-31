/*
 * Test 27 - Nested Folder Creation and Deletion
 *
 * This program demonstrates the creation and deletion of a specified number of nested
 * folders in a systematic manner. It starts by creating a folder named "test27" and
 * enters it. Inside "test27", it creates a specified number of subfolders named
 * "folder0", "folder1", ..., "foldern". After creating the nested folders, it proceeds
 * to delete them one by one, starting from "foldern" and moving upwards. Finally, it
 * moves back to the initial directory and deletes the "test27" folder.
 *
 * Test Functions:
 * 1. createAndDeleteFolders() - Creates and deletes a specified number of nested folders.
 * 2. main() - Entry point of the program, calls createAndDeleteFolders() with the desired
 *    number of nested folders.
 */

#include "sysapi.h"
#include "test27.h"

int createAndDeleteFolders(int numFolders) {
  int i;
  char folderName[20];

  // Create test27 folder
  if (mkdir("test27", 0)<0){
    return -1;
  }

  // Move to test27 folder
  if (chdir("test27")<0){
    return -1;
  }

  // Create and change directories multiple times
  for (i = 0; i < numFolders; i++) {
    sprintf(folderName, "folder%d", i);
    if (mkdir(folderName, 0)<0){
      return -1;
    }
    if (chdir(folderName)<0){
      return -1;
    }
    if (chdir("../././")<0){
      return -1;
    }
    if (chdir(folderName)<0){
      return -1;
    }
  }

  // Delete folders in reverse order
  for (i = numFolders - 1; i >= 0; i--) {
    // Move back to the parent directory
    if (chdir("..")<0){
      return -1;
    }
    sprintf(folderName, "folder%d", i);
    if (rmdir(folderName)<0){
      return -1;
    }
  }

  // Move back to the initial directory and delete test27 folder
  if (chdir("..")<0){
    return -1;
  }
  if (rmdir("test27")<0){
    return -1;
  }
  return 0;
}

int simple_chdir_test(){
  if (chdir("/")<0){
    return -1;
  }
  if (mkdir("chtest27", 0)<0){
    return -1;
  }
  if (chdir("chtest27")<0){
    return -1;
  }
  if (mkdir("dir1", 0)<0){
    return -1;
  }
  if (chdir("dir1")<0){
    return -1;
  }
  if (mkdir("dir2", 0)<0){
    return -1;
  }
  if (chdir("dir2")<0){
    return -1;
  }
  if (chdir(".././.././././././././.././")<0){
    return -1;
  }
  if (rmdir("chtest27/dir1/dir2/")<0){
    return -1;
  }
  if (rmdir("/chtest27/dir1/")<0){
    return -1;
  }
  if (rmdir("./././././chtest27/")<0){
    return -1;
  }
  return 0;
}

int main() {
  int numFolders = 30;
  assert(createAndDeleteFolders(numFolders) == 0);
  assert(simple_chdir_test() == 0);
  return 0;
}
