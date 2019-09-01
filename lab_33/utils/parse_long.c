#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "parse_long.h"

#define NO_ERROR (0)

static int is_whitespace_string(char *str) {
    static const char spaceCharSet[] = " \t\n\v\f\r";
    unsigned long stringLength = strlen(str);
    size_t spaceSequenceLen = strspn(str, spaceCharSet);
    if ((size_t)stringLength != spaceSequenceLen) {
        return FALSE;
    }
    return TRUE;
}

int parse_long(long *val, char *line) {
    errno = NO_ERROR;

    int base = 10;
    char *lineTail = line;
    long number = strtol(line, &lineTail, base);

    if (errno == ERANGE && (number == LONG_MAX || number == LONG_MIN)) {
        perror("parse_long(..) error");
        return FAILURE_CODE;
    }

    if (number == 0 && errno != NO_ERROR) {
        perror("parse_long(..) error");
        return FAILURE_CODE;
    }

    if (lineTail == line) {
        return FAILURE_CODE;
    }

    int isWhitespaceStr = is_whitespace_string(lineTail);
    if (isWhitespaceStr == FALSE) {
        return FAILURE_CODE;
    }

    *val = number;

    return SUCCESS_CODE;
}
