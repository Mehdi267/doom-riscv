#include "ufunc.h"

char* my_fgets(char* buffer, int size, int fd) {
    if (buffer == NULL || size <= 0 ) {
        return NULL;
    }
    int c;
    int i = 0;

    while (i < size - 1 && (c = per_fgetc(fd)) != EOF) {
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