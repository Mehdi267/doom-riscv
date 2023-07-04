#include "stdio.h"


int isdigit(int c) {
    return c >= '0' && c <= '9';
}

int isspace(int c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

int atoi(const char *str) {
    int result = 0;
    int sign = 1;
    int i = 0;

    // Skip leading whitespaces
    while (isspace(str[i])) {
        i++;
    }

    // Check for sign
    if (str[i] == '-' || str[i] == '+') {
        sign = (str[i++] == '-') ? -1 : 1;
    }

    // Convert digits to integer
    while (isdigit(str[i])) {
        result = result * 10 + (str[i++] - '0');
    }

    return result * sign;
}