#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

void copy_file(const char *src, const char *dest) {
  int src_fd, dest_fd;
  struct stat src_stat, dest_stat;

  src_fd = open(src, O_RDONLY, 0);
  if (src_fd == -1) {
    perror("open (source)");
    return;
  }

  if (fstat(src_fd, &src_stat) == -1) {
    perror("fstat");
    close(src_fd);
    return;
  }

  // Check if the destination is a directory
  if (lstat(dest, &dest_stat) == 0 && S_ISDIR(dest_stat.st_mode)) {
    // Append the source file's name to the destination directory
    char dest_filename[PATH_MAX];
    snprintf(dest_filename, sizeof(dest_filename), "%s/%s", dest, basename(src));
    
    dest_fd = open(dest_filename, O_WRONLY | O_CREAT | O_TRUNC, src_stat.st_mode);
    if (dest_fd == -1) {
      perror("open (destination)");
      close(src_fd);
      return;
    }
  } else {
    dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, src_stat.st_mode);
    if (dest_fd == -1) {
      perror("open (destination)");
      close(src_fd);
      return;
    }
  }

  char buffer[1024];
  ssize_t bytes_read, bytes_written;

  while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
    bytes_written = write(dest_fd, buffer, bytes_read);
    if (bytes_written == -1) {
      perror("write");
      close(src_fd);
      close(dest_fd);
      return;
    }
  }

  close(src_fd);
  close(dest_fd);
}

void copy_directory(const char *src, const char *dest) {
  struct dirent *entry;
  DIR *dir = opendir(src);

  if (!dir) {
    perror("opendir");
    return;
  }

  struct stat src_stat;
  if (lstat(src, &src_stat) == -1) {
    perror("lstat (source directory)");
    return;
  }
  char dest_final[1024];
  memset(dest_final, 0, 1024);
  // Check if the destination already exists
  struct stat dest_stat;
  if (lstat(dest, &dest_stat) == 0) {
    // Destination exists, and it's a directory
    if (S_ISDIR(dest_stat.st_mode)) {
      // Append the source directory name to the destination path
      snprintf(dest_final, sizeof(dest_final), "%s/%s", dest, src);
      dest = dest_final;
    } else {
      fprintf(stderr, "Destination is not a directory: %s\n", dest);
      return;
    }
  }

  // Create the destination directory
  if (mkdir(dest, src_stat.st_mode) == -1) {
    printf("mkdir failed %s\n", dest);
    return;
  }
  while ((entry = readdir(dir))) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    char src_path[1024];
    char dest_path[1024];
    memset(src_path, 0, 1024);
    memset(dest_path, 0, 1024);
    snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
    snprintf(dest_path, sizeof(dest_path), "%s/%s", dest, entry->d_name);

    if (entry->d_type == DT_DIR) {
      copy_directory(src_path, dest_path);
    } else {
      copy_file(src_path, dest_path);
    }
  }

  closedir(dir);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <source_path> <destination_path>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *src = argv[1];
  const char *dest = argv[2];

  struct stat src_stat;
  if (lstat(src, &src_stat) == -1) {
    perror("lstat (source)");
    exit(EXIT_FAILURE);
  }

  if (S_ISDIR(src_stat.st_mode)) {
    copy_directory(src, dest);
  } else {
    copy_file(src, dest);
  }

  return 0;
}
