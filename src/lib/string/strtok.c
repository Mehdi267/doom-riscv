#include <string.h>
#include <stddef.h>

char* strtok(char* str, const char* delimiters) {
    static char* saved_ptr = NULL;

    if (str != NULL) {
        saved_ptr = str;
    } else {
        // Check if there is a previous token
        if (saved_ptr == NULL) {
            return NULL;
        }
    }

    // Skip leading delimiters
    while (*saved_ptr != '\0' && strchr(delimiters, *saved_ptr) != NULL) {
        saved_ptr++;
    }

    // Return NULL if no more tokens are found
    if (*saved_ptr == '\0') {
        return NULL;
    }

    // Find the end of the token
    char* token = saved_ptr;
    while (*saved_ptr != '\0' && strchr(delimiters, *saved_ptr) == NULL) {
        saved_ptr++;
    }

    // Mark the end of the token with a null terminator
    if (*saved_ptr != '\0') {
        *saved_ptr = '\0';
        saved_ptr++;
    }

    return token;
}
