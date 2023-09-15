#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

long int ftell(int stream) {
  // Get the current file position.
  long int position = lseek(stream, 0, SEEK_CUR);
  if (position == -1L) {
    // Error occurred while determining the position.
    return -1L;
  }

  return position;
}
