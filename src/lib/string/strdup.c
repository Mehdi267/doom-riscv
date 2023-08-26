#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *strdup(const char *str) {
    if (str == NULL) {
        return NULL; // Return NULL for NULL input
    }

    size_t len = strlen(str) + 1; // Include space for null-terminator

    char *duplicate = (char *)malloc(len); // Allocate memory for duplicate
    if (duplicate == NULL) {
        return NULL; // Return NULL if memory allocation fails
    }

    strcpy(duplicate, str); // Copy the string
    return duplicate;
}

