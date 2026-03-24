#include <stdlib.h>

int atoi(const char *str) {
    int res = 0;
    int sign = 1;
    int i = 0;

    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n') {
        i++;
    }

    if (str[i] == '-') {
        sign = -1;
        i++;
    } else if (str[i] == '+') {
        i++;
    }

    for (; str[i] != '\0'; ++i) {
        if (str[i] < '0' || str[i] > '9') {
            break;
        }
        res = res * 10 + (str[i] - '0');
    }

    return sign * res;
}

char *itoa(int value, char *str, int base) {
    if (base < 2 || base > 36) {
        str[0] = '\0';
        return str;
    }

    int i = 0;
    int is_negative = 0;
    unsigned int n;

    if (value < 0 && base == 10) {
        is_negative = 1;
        n = (unsigned int)(-(value + 1)) + 1u;
    } else {
        n = (unsigned int)value;
    }

    if (n == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    while (n != 0) {
        unsigned int rem = n % (unsigned int)base;
        if (rem > 9) {
            str[i++] = (char)(rem - 10 + 'a');
        } else {
            str[i++] = (char)(rem + '0');
        }
        n = n / (unsigned int)base;
    }

    if (is_negative) {
        str[i++] = '-';
    }

    str[i] = '\0';

    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }

    return str;
}
