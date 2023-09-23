#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <directory_name>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *dirName = argv[1];

  if (mkdir(dirName, 0777) == 0) {
    printf("Directory '%s' created successfully.\n", dirName);
  } else {
    perror("mkdir");
    exit(EXIT_FAILURE);
  }

  return 0;
}
