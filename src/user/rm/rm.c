#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>

int is_character_in_string(const char *str, char target) {
  while (*str != '\0') {
    if (*str == target) {
      return 1; // Character found in the string
    }
    str++; // Move to the next character in the string
  }
  return 0; // Character not found in the string
}

void remove_file(const char *path) {
  if (unlink(path) != 0) {
    perror("remove");
  } else {
    printf("Removed file: %s\n", path);
  }
}

void remove_directory(const char *path) {
  DIR *dir = opendir(path);
  if (dir == NULL) {
    perror("opendir");
    return;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue; // Skip "." and ".." entries
    }

    char entry_path[PATH_MAX];
    snprintf(entry_path, sizeof(entry_path), "%s/%s", path, entry->d_name);

    struct stat st;
    if (lstat(entry_path, &st) == -1) {
      perror("lstat");
      continue;
    }

    if (S_ISDIR(st.st_mode)) {
      remove_directory(entry_path); // Recursive call for subdirectories
    } else {
      remove_file(entry_path);
    }
  }

  closedir(dir);

  if (rmdir(path) == 0) {
    printf("Removed directory: %s\n", path);
  } else {
    perror("rmdir");
  }
}

int main(int argc, char *argv[]) {
  int recursive = 0;
  int directory = 0;
  int argfound = 0;
  for (int i = 1; i < argc; i++) {
    const char *arg = argv[i];
    if (arg[0] == '-') {
      argfound = 0;
      if (is_character_in_string(arg, 'r') == 1) {
        recursive = 1;
        argfound = 1;
      } 
      if (is_character_in_string(arg, 'd') == 1) {
        directory = 1;
        argfound = 1;
      }
      if (argfound == 0){
        fprintf(stderr, "Unknown option: %s\n", arg);
        exit(EXIT_FAILURE);
      }
    } else {
      // Process non-option arguments (file/directory paths)
      struct stat st;
      if (lstat(arg, &st) == -1) {
        perror("lstat");
        continue;
      }

      printf("S_ISDIR(st.st_mode) = %d", S_ISDIR(st.st_mode));
      if (S_ISDIR(st.st_mode) && directory && recursive) {
        remove_directory(arg);
      } else if (!S_ISDIR(st.st_mode)) {
        remove_file(arg);
      } else {
        fprintf(stderr, "Cannot remove directory '%s' without -rd option.\n", arg);
      }
    }
  }

  return 0;
}
