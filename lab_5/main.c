#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libgen.h>
#include <limits.h>
#include <errno.h>
#include "fileutils.h"
#include "constants.h"

#define REQUIRED_ARG_NUM (2)
#define LINES_MAX_NUM (100)

void handle_requests(int fd, size_t *lineSizes, off_t *lineOffsets, int tableSize);

void print_usage(char *progPath) {
    fprintf(stderr, "Usage: %s <input file>\n", basename(progPath));
}

int main(int argc, char *argv[]) {
    if (argc != REQUIRED_ARG_NUM) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    char *filename = argv[1];
    int fd = open_file(filename);
    if (fd == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    static size_t lineSizes[LINES_MAX_NUM];
    static off_t lineOffsets[LINES_MAX_NUM];
    int linesNum = fill_size_table(fd, lineSizes, LINES_MAX_NUM);
    if (linesNum == FAILURE_CODE) {
        close_file(fd);
        exit(EXIT_FAILURE);
    }
    fill_offset_table(lineOffsets, lineSizes, linesNum);

    handle_requests(fd, lineSizes, lineOffsets, linesNum);
    close_file(fd);

    return EXIT_SUCCESS;
}

#define BUFFER_SIZE (256)

int parse_long(long *val);

void handle_requests(int fd, size_t *lineSizes, off_t *lineOffsets, int tableSize) {
    printf("Enter line number from 1 to %d, or 0 to exit\n", tableSize);
    long lineNo = 0;
    int returnCode = 0, eofIndicator = 0, ferrorIndicator = 0;
    for (;;) {
        printf("\n>> ");
        returnCode = parse_long(&lineNo);
        eofIndicator = feof(stdin);
        ferrorIndicator = ferror(stdin);
        if (eofIndicator != NOT_EOF) {
            fprintf(stderr, "End of file has been reached\n");
            break;
        }
        if (ferrorIndicator != NO_FERROR) {
            fprintf(stderr, "An error has occured while reading from the standard input\n");
            break;
        }
        if (returnCode == FAILURE_CODE) {
            fprintf(stderr, "Error while reading a line number: incorrect input\n");
            continue;
        }
        if (lineNo < 0 || lineNo > tableSize) {
            fprintf(stderr, "Line number must be from 1 to %d, you've entered %ld\n",
                    tableSize, lineNo);
            continue;
        }
        if (lineNo == 0) {
            break;
        }
        print_line_from_file(fd, lineOffsets[lineNo - 1], lineSizes[lineNo - 1]);
    }
}

int is_whitespace_string(char *str);

int parse_long(long *val) {
    static char line[BUFFER_SIZE];
    char *res = fgets(line, BUFFER_SIZE, stdin);
    if (res == NULL) {
        return FAILURE_CODE;
    }

    int base = 10;
    char *lineTail = line;
    int errnoSaved = errno;
    errno = NO_ERROR;
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
    errno = errnoSaved;

    return SUCCESS_CODE;
}

int is_whitespace_string(char *str) {
    static const char spaceCharSet[] = " \t\n\v\f\r";
    unsigned long stringLength = strlen(str);
    size_t spaceSequenceLen = strspn(str, spaceCharSet);
    if ((size_t)stringLength != spaceSequenceLen) {
        return FALSE;
    }
    return TRUE;
}
