#include <stdlib.h>
#include <limits.h>

int atoi(const char *str) {
    if (!str) return 0;

    int res = 0;
    int sign = 1;

    while (*str == ' ' || *str == '\t' || *str == '\n' || 
           *str == '\r' || *str == '\v' || *str == '\f') {
        str++;
    }

    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    while (*str >= '0' && *str <= '9') {
        int digit = *str - '0';

        if (res > (INT_MAX - digit) / 10) {
            return sign == 1 ? INT_MAX : INT_MIN;
        }

        res = res * 10 + digit;
        str++;
    }

    return sign * res;
}

char *itoa(int value, char *str, int base) {
    if (!str || base < 2 || base > 36) {
        if (str) *str = '\0';
        return str;
    }

    char *ptr = str;
    unsigned int n = (unsigned int)value;

    if (value < 0 && base == 10) {
        n = -n;
    }

    const char digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";

    do {
        *ptr++ = digits[n % base];
        n /= base;
    } while (n);

    if (value < 0 && base == 10) {
        *ptr++ = '-';
    }

    *ptr = '\0';

    char *start = str;
    char *end = ptr - 1;
    while (start < end) {
        char temp = *start;
        *start++ = *end;
        *end-- = temp;
    }

    return str;
}
