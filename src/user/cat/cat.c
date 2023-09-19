#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
  printf("Argv is equal to %s \n", argv[0]);
  if (argc < 2) {
    fprintf(stderr, "Usage: %s file1 [file2 ...]\n", argv[0]);
    exit(EXIT_FAILURE);
  }


  for (int i = 1; i < argc; i++) {
    int file_descriptor = open(argv[i], O_RDONLY);

    if (file_descriptor == -1) {
      perror("cat");
      continue;
    }

    char buffer[4096];
    ssize_t bytes_read;

    while ((bytes_read = read(file_descriptor, buffer, sizeof(buffer))) > 0) {
      ssize_t bytes_written = write(STDOUT_FILENO, buffer, bytes_read);

      if (bytes_written != bytes_read) {
        perror("cat");
        close(file_descriptor);
        exit(EXIT_FAILURE);
      }
    }

    close(file_descriptor);
  }

  return 0;
}
