#include <sysapi.h>
#include <stat.h>

void print_stat(const char *path) {
  struct stat st;
  if (stat(path, &st) == 0) {
    printf("Attributes of '%s':\n", path);
    printf("  - Size: %ld bytes\n", st.st_size);
    printf("  - Type: %s \n", S_ISDIR(st.st_mode) ? "Directory" : "File");
    printf("\n");
  } else {
    printf  ("stat");
    exit(EXIT_FAILURE);
  }
}

void create_file(const char* path){
  // Create a test file
  int fd = open(path, O_CREAT | O_WRONLY, 0666);
  if (fd == -1) {
    printf  ("open");
    exit(EXIT_FAILURE);
  }
  close(fd);
}

int rename_test() {
  if (mkdir("test34", 0777)<0){
    printf("mkdir test34 failed \n");
    return 1;
  }
  if (chdir("test34")<0){
    printf("chdir test34 failed \n");
    return 1;
  }

  const char *oldPathFile = "oldfile.txt";
  const char *newPathFile = "newfile.txt";

  create_file(oldPathFile);

  // Print attributes before rename
  printf("Before Rename:\n");
  print_stat(oldPathFile);
  printf("Calling rename syscall %p ; %p \n", oldPathFile, newPathFile);
  // Test 1: Rename a file
  if (rename(oldPathFile, newPathFile) == 0) {
    printf("Test 1: File renamed successfully.\n");
    print_stat(newPathFile);  // Print attributes after rename
  } else {
    printf  ("Test 1: rename");
    exit(EXIT_FAILURE);
  }

  // Test 2: Rename a directory
  const char *oldDir = "mydir";
  const char *newDir = "newdir";
  if (mkdir(oldDir, 0777) == -1) {
    if (rmdir(oldDir)<0){
      printf  ("mkdir");
      exit(EXIT_FAILURE);
    }
  }
  // Print attributes before rename
  printf("\nBefore Rename:\n");
  print_stat(oldDir);

  if (rename(oldDir, newDir) == 0) {
    printf("Test 2: Directory renamed successfully.\n");
    print_stat(newDir);  // Print attributes after rename
  } else {
    printf  ("Test 2: rename");
    exit(EXIT_FAILURE);
  }

  // Test 3: Attempt to rename to an existing file (should replace the file)
  create_file(oldPathFile);
  // Print attributes before rename
  printf("\nBefore Rename:\n");
  print_stat(newPathFile);

  if (rename(oldPathFile, newPathFile) == 0) {
    printf("Test 3: File replaced successfully.\n");
    print_stat(newPathFile);  // Print attributes after rename
  } else {
    printf  ("Test 3: rename");
    exit(EXIT_FAILURE);
  }

  create_file(oldPathFile);
  // Test 4: Attempt to rename to an existing non-empty directory (should fail)
  // Print attributes before rename
  printf("\nBefore Rename:\n");
  print_stat(oldPathFile);

  if (rename(oldPathFile, newDir) == -1) {
    printf("Test 4: Rename to non-empty directory failed as expected.\n");
    print_stat(oldPathFile);  // Print attributes after failed rename
  } else {
    printf("Test 4: Unexpected success (should have failed).\n");
  }

  // Clean up
  unlink(newPathFile);  // Remove the renamed file
  unlink(oldPathFile);  // Remove the renamed file
  rmdir(newDir);      // Remove the renamed directory
  
  if (chdir("..")<0){
    printf("chdir test34 failed \n");
    return 1;
  }
  if (rmdir("test34")<0){
    printf("mkdir test34 failed \n");
    return 1;
  }
  return 0;
}


void main(){
  assert(rename_test() == 0);
}