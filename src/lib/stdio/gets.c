#include "stdio.h"
#include "unistd.h"

char* fgets(char* buffer, int size, int fd) {
    if (buffer == NULL || size <= 0 ) {
        return NULL;
    }
    int c;
    int i = 0;

    while (i < size) {
        c = fgetc(fd);
        if (c == EOF){
          break;
        }
        if (c == BS){
          if (i != 0){i--;}
          buffer[i] = 0 ;
          continue;
        }
        buffer[i++] = c;
        if (c == '\n') {
            break;
        }
    }

    buffer[i] = '\0';

    if (i == 0 && c == EOF) {
        return NULL; // No characters read (EOF reached)
    }

    return buffer;
}