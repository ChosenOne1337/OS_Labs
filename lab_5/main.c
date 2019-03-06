#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "fileutils.h"

#define REQUIRED_ARG_NUM (2)
#define LINES_MAX_NUM (100)
#define BUFFER_SIZE (256)
#define TRUE (1)

int read_integer_value(int *val) {
    static char buf[BUFFER_SIZE];
    char *res = fgets(buf, BUFFER_SIZE, stdin);
    if (res == NULL) {

    }
    if (feof(stdin)) {
        return FAILURE_CODE;
    }
    if (ferror(stdin)) {
        return FAILURE_CODE;
    }
    printf("%s\n", buf);
    return SUCCESS_CODE;
}

void print_usage(char *progPath) {
    fprintf(stderr, "Usage: %s <input file>\n", basename(progPath));
}

void handle_requests(int fd, size_t *lineSizes, off_t *lineOffsets, int tableSize) {
    printf("Enter line number from 1 to %d, or 0 to exit\n", tableSize);
    int lineNo = 0, returnCode = 0;
    while (TRUE) {
        printf(">> ");
        returnCode = read_integer_value(&lineNo);
        if (returnCode == FAILURE_CODE) {
            continue;
        }
        if (lineNo < 0 || lineNo > tableSize) {
            fprintf(stderr, "Line number must be from 1 to %d, you've entered %d\n",
                    tableSize, lineNo);
            continue;
        }
        if (lineNo == 0) {
            break;
        }
        print_line_from_file(fd, lineOffsets[lineNo - 1], lineSizes[lineNo - 1]);
    }
}

int main(int argc, char *argv[]) {
//    if (argc != REQUIRED_ARG_NUM) {
//        print_usage(argv[0]);
//        return EXIT_FAILURE;
//    }

//    char *filename = argv[1];
//    int fd = open_file(filename);
//    if (fd == FAILURE_CODE) {
//        return EXIT_FAILURE;
//    }

//    static size_t lineSizes[LINES_MAX_NUM];
//    static off_t lineOffsets[LINES_MAX_NUM];

//    int linesNum = fill_size_table(fd, lineSizes, LINES_MAX_NUM);
//    if (linesNum == FAILURE_CODE) {
//        close_file(fd, filename);
//        exit(EXIT_FAILURE);
//    }
//    fill_offset_table(lineOffsets, lineSizes, linesNum);

//    handle_requests(fd, lineSizes, lineOffsets, linesNum);
//    close_file(fd, filename);
    while (1) {
        printf(">> ");
        int r = read_integer_value(NULL);
        if (r == FAILURE_CODE) {
            continue;
        }
    }

    return EXIT_SUCCESS;
}
