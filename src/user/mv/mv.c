#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

void move_file(const char *source, const char *destination) {
  struct stat dest_stat;
  if (stat(destination, &dest_stat) == -1) {
    // Destination does not exist or not accessible; assume it's a file
    if (rename(source, destination) == -1) {
      perror("rename");
      exit(EXIT_FAILURE);
    }
    printf("File moved from %s to %s\n", source, destination);
  } else {
    // Destination exists and is accessible; check if it's a directory
    if (S_ISDIR(dest_stat.st_mode)) {
      // Append the source file name to the destination directory path
      char *source_file_name = basename((char *)source);
      char destination_path[1024];
      snprintf(destination_path, sizeof(destination_path), "%s/%s", destination, source_file_name);

      if (rename(source, destination_path) == -1) {
        perror("rename");
        exit(EXIT_FAILURE);
      }
      printf("File moved from %s to %s\n", source, destination_path);
    } else {
      // Destination is not a directory; replace it
      if (rename(source, destination) == -1) {
        perror("rename");
        exit(EXIT_FAILURE);
      }
      printf("File moved from %s to %s\n", source, destination);
    }
  }
}


void move_directory(const char *source, const char *destination) {
  // Create the destination directory if it doesn't exist
  struct stat st;
  if (stat(destination, &st) == -1) {
    if (mkdir(destination, 0755) == -1) {
      perror("mkdir");
      exit(EXIT_FAILURE);
    }
  }

  DIR *dir = opendir(source);
  if (!dir) {
    perror("opendir");
    exit(EXIT_FAILURE);
  }

  struct dirent *entry;
  while ((entry = readdir(dir))) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    char source_path[1024];
    char destination_path[1024];
    snprintf(source_path, sizeof(source_path), "%s/%s", source, entry->d_name);
    snprintf(destination_path, sizeof(destination_path), "%s/%s", destination, entry->d_name);

    if (entry->d_type == DT_DIR) {
      move_directory(source_path, destination_path);
    } else {
      move_file(source_path, destination_path);
    }
  }

  closedir(dir);

  // Remove the source directory after moving its contents
  if (rmdir(source) == -1) {
    perror("rmdir");
    exit(EXIT_FAILURE);
  }
  printf("Directory moved from %s to %s\n", source, destination);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *source = argv[1];
  const char *destination = argv[2];

  struct stat source_stat;
  if (stat(source, &source_stat) == -1) {
    perror("stat");
    exit(EXIT_FAILURE);
  }

  if (S_ISREG(source_stat.st_mode)) {
    move_file(source, destination);
  } else if (S_ISDIR(source_stat.st_mode)) {
    move_directory(source, destination);
  } else {
    fprintf(stderr, "Unsupported file type\n");
    exit(EXIT_FAILURE);
  }

  return 0;
}
