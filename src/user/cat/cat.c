#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s file1 [file2 ...]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  for (int i = 1; i < argc; i++) {
    fprintf(stdout, "Treating arg[%d] = %s\n", i, argv[i]);
    int file_descriptor = open(argv[i], O_RDONLY, 0);
    if (file_descriptor == -1) {
      fprintf(stderr, "Can't find the file %s\n",
           argv[i]);
      continue;
    }

    char buffer[4096];
    ssize_t bytes_read;

    while ((bytes_read = read(file_descriptor, buffer, sizeof(buffer))) > 0) {
      ssize_t bytes_written = write(stdout, buffer, bytes_read);
      printf("Buffer = %s \n", buffer);
      printf("bytes_written = %d // bytes_read = %d \n", 
          bytes_written, bytes_read);
      if (bytes_written != bytes_read) {
        perror("Cat failed\n");
        close(file_descriptor);
        exit(EXIT_FAILURE);
      }
    }

    close(file_descriptor);
  }

  return 0;
}
