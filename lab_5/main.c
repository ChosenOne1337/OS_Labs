#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libgen.h>
#include <errno.h>
#include "fileutils.h"

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
#define TRUE (1)

int parse_long(long *val);

void handle_requests(int fd, size_t *lineSizes, off_t *lineOffsets, int tableSize) {
    printf("Enter line number from 1 to %d, or 0 to exit\n", tableSize);
    long lineNo = 0;
    int returnCode = 0;
    while (TRUE) {
        printf("\n>> ");
        returnCode = parse_long(&lineNo);
        if (feof(stdin)) {
            fprintf(stderr, "End of file has been reached\n");
            break;
        } else if (ferror(stdin)) {
            fprintf(stderr, "An error has occured while reading from the standard input\n");
            break;
        } else if (returnCode == FAILURE_CODE) {
            fprintf(stderr, "Error while reading a line number: incorrect input\n");
            continue;
        } else if (lineNo < 0 || lineNo > tableSize) {
            fprintf(stderr, "Line number must be from 1 to %d, you've entered %ld\n",
                    tableSize, lineNo);
            continue;
        } else if (lineNo == 0) {
            break;
        }
        print_line_from_file(fd, lineOffsets[lineNo - 1], lineSizes[lineNo - 1]);
    }
}

int parse_long(long *val) {
    static char buf[BUFFER_SIZE];
    char *res = fgets(buf, BUFFER_SIZE, stdin);
    if (res == NULL) {
        return FAILURE_CODE;
    }

    int base = 10;
    char *endPtr = NULL;
    int errnoSaved = errno;
    errno = 0;
    long number = strtol(buf, &endPtr, base);
    if (endPtr == buf) {
        return FAILURE_CODE;
    } else if (errno == ERANGE) {
        perror("parse_long(..) error");
        return FAILURE_CODE;
    }

    static const char STR_END_CHAR = '\0';
    for (; *endPtr != STR_END_CHAR; ++endPtr) {
        if (!isspace(*endPtr)) {
            return FAILURE_CODE;
        }
    }

    *val = number;
    errno = errnoSaved;

    return SUCCESS_CODE;
}
