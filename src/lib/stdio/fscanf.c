#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

int sscanf(const char *str, const char *format, ...) {
  va_list args;
  va_start(args, format);

  const char *fmt = format;
  const char *buf = str;
  int numMatches = 0;

  while (*fmt && *buf) {
    if (*fmt == '%') {
      fmt++;  // Move past the '%'.

      if (*fmt == '\0') {
        break;  // Unexpected end of format string.
      }

      // Handle format specifiers (e.g., %d, %s, %c, %lf).
      switch (*fmt) {
        case 'd': {
          int *intPtr = va_arg(args, int *);
          int sign = 1;

          // Handle negative numbers.
          if (*buf == '-') {
            sign = -1;
            buf++;
          }

          // Read and convert digits.
          int result = 0;
          while (isdigit(*buf)) {
            result = result * 10 + (*buf - '0');
            buf++;
          }

          *intPtr = result * sign;
          numMatches++;
          break;
        }
        case 's': {
          char *strPtr = va_arg(args, char *);
          while (!isspace(*buf) && *buf != '\0') {
            *strPtr = *buf;
            strPtr++;
            buf++;
          }
          *strPtr = '\0';  // Null-terminate the string.
          numMatches++;
          break;
        }
        case 'c': {
          char *charPtr = va_arg(args, char *);
          *charPtr = *buf;
          buf++;
          numMatches++;
          break;
        }
        // Add more format specifiers as needed.
        default:
          return numMatches;  // Unsupported format specifier.
      }
    } else if (*fmt != *buf) {
      return numMatches;  // Mismatched character.
    } else {
      fmt++;
      buf++;
    }
  }

  va_end(args);
  return numMatches;
}

