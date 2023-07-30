#include <stddef.h>
#include <string.h>

char *strrchr(const char *str, int character) {
    const char *last_occurrence = NULL;

    while (*str != '\0') {
        if (*str == character) {
            last_occurrence = str;
        }
        str++;
    }

    // Include the null terminator character in the search
    if (*str == character) {
        last_occurrence = str;
    }

    return (char *)last_occurrence;
}
