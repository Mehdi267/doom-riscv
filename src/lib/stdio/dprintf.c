#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <unistd.h>

int dprintf(int fd, const char *format, ...) {
    va_list args;
    va_start(args, format);

    char buffer[1024];  // Adjust the buffer size as needed.
    int len = vsnprintf(buffer, sizeof(buffer), format, args);

    va_end(args);

    if (len < 0) {
        return -1;  // Error in vsnprintf.
    }

    size_t written = write(fd, buffer, len);

    if (written < 0) {
        return -1;  // Error in write.
    }

    return written;
}