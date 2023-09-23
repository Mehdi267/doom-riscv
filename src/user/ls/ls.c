#include <stdio.h>
#include <dirent.h>

int main() {
  // Directory path
  const char *dir_path = "."; // Current directory

  // Open the directory
  DIR *dir = opendir(dir_path);
  if (dir == NULL) {
      perror("opendir");
      return 1;
  }

  // Read and print directory entries
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
      printf("%s\n", entry->d_name);
  }

  // Close the directory
  closedir(dir);

  return 0;
}