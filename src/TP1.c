#include "../lib/TP1.h"

// ---- Functions Implementation ----
// 1. weather mesurements

// 1.1 helping functions

// returns ERR_NONE (0) if date is valid, otherwise it returns the corresponding error number
int isValidDate(const char *date) {
    int d, m, y;
    int pos = 0;
    int errors = ERR_NONE;

    // Check format first
    if (sscanf(date, "%2d/%2d/%4d%n", &d, &m, &y, &pos) != 3 || date[pos] != '\0') {
        errors |= ERR_FORMAT;
        return errors;  // can't trust values if format is bad
    }

    // Check month range
    if (m < 1 || m > 12)
        errors |= ERR_MONTH_RANGE;

    // Check day range
    if (d < 1 || d > 31)
        errors |= ERR_DAY_RANGE;

    // Check year range
    if (y < 1900 || y > 2100)
        errors |= ERR_YEAR_RANGE;

    // Check leap year rule if February 29
    int leap = (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0));
    if (m == 2 && d > (leap ? 29 : 28))
        errors |= ERR_LEAP_DAY;

    return errors;  // can be 0 (no error) or combination
}

// returns a random integer in interval [low, up]
int random(int low, int up) {
    return ((rand() % up-low+1) + up);
}

// reminder: str => int and int => str conversions
// int atoi(char s[])
// void itoa(int n, char s[])







