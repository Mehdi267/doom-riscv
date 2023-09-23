#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <directory>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *newDir = argv[1];
  if (chdir(newDir) == 0) {
    printf("Current directory is now: %s\n", newDir);
  } else {
    perror("chdir");
    exit(EXIT_FAILURE);
  }

  return 0;
}
