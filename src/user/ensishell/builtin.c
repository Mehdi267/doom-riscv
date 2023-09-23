#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int change_directory(const char *newDir) {
  // Find the first non-null character
  const char *start = newDir;
  while (*start != '\0' && *start == ' ') {
    start++;
  }

  // Find the last character before the null terminator
  const char *end = newDir + strlen(newDir) - 1;
  while (end > start && *end == ' ') {
    end--;
  }

  // Calculate the length of the trimmed string
  size_t trimmed_length = end - start + 1;

  // Check if the trimmed string is empty
  if (trimmed_length == 0) {
    printf("Invalid directory path.\n");
    return -1; // Failure
  }

  // Allocate memory for the trimmed string
  char *trimmedDir = (char *)malloc(trimmed_length + 1);
  if (trimmedDir == NULL) {
    perror("malloc");
    return -1; // Failure
  }

  // Copy the trimmed string
  strncpy(trimmedDir, start, trimmed_length);
  trimmedDir[trimmed_length] = '\0';

  // Change the current directory using the trimmed string
  if (chdir(trimmedDir) == 0) {
    printf("Current directory is now: %s\n", trimmedDir);
    free(trimmedDir);
    return 0; // Success
  } else {
    printf("chdir failed\n");
    free(trimmedDir);
    return -1; // Failure
  }
}

