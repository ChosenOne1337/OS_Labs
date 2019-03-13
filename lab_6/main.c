#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libgen.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>
#include "fileutils.h"
#include "constants.h"

#define REQUIRED_ARG_NUM (2)
#define LINES_LIMIT (100)
#define ALARM_TIMEOUT (5)
#define ALARM_RESET (0)

#define INTERRUPTED_CODE (2)
#define STDIN_ERROR_CODE (3)
#define EXIT_CODE (4)

int handle_request(FileInfo*);

void print_usage(char *progPath) {
    fprintf(stderr, "Usage: %s <input file>\n", basename(progPath));
}

void handle_alarm_signal(int sig) {
    signal(SIGALRM, handle_alarm_signal);
}

int main(int argc, char *argv[]) {
    if (argc != REQUIRED_ARG_NUM) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    FileInfo fileInfo;
    char *filename = argv[1];
    int returnCode = open_file(&fileInfo, filename);
    if (returnCode == FAILURE_CODE) {
        return EXIT_FAILURE;
    }
    returnCode = fill_file_info(&fileInfo, LINES_LIMIT);
    if (returnCode == FAILURE_CODE) {
        close_file(&fileInfo);
        return EXIT_FAILURE;
    }

    printf("Enter line number from 1 to %d, or 0 to exit\n", fileInfo.linesNum);
    signal(SIGALRM, handle_alarm_signal);
    alarm(ALARM_TIMEOUT);
    returnCode = handle_request(&fileInfo);
    alarm(ALARM_RESET);
    if (returnCode == INTERRUPTED_CODE) {
        printf("\n");
        print_file(fileInfo.fd);
    }
    else while (returnCode != EXIT_CODE && returnCode != STDIN_ERROR_CODE) {
        returnCode = handle_request(&fileInfo);
    }
    close_file(&fileInfo);

    return EXIT_SUCCESS;
}


#define BUFFER_SIZE BUFSIZ

int read_line(char *line, int bufLength);
int parse_long(long *val, char *line);
int is_whitespace_string(char *str);
int get_line_number(long *lineNumber, long maxNumber);

int handle_request(FileInfo *fileInfo) {
    long lineNo = 0;
    int returnCode = get_line_number(&lineNo, fileInfo->linesNum);
    if (returnCode != SUCCESS_CODE) {
        return returnCode;
    }

    returnCode = print_line_from_file(fileInfo->fd, fileInfo->lineOffsets[lineNo - 1], fileInfo->lineSizes[lineNo - 1]);
    if (returnCode == FAILURE_CODE) {
        fprintf(stderr, "Failed to print a line from the file\n");
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

int get_line_number(long *lineNumber, long maxNumber) {
    long lineNo = 0;
    int returnCode = 0, eofIndicator = 0, ferrorIndicator = 0;
    static char buf[BUFFER_SIZE];

    returnCode = read_line(buf, BUFFER_SIZE);
    eofIndicator = feof(stdin);
    ferrorIndicator = ferror(stdin);
    if (returnCode == INTERRUPTED_CODE) {
        return INTERRUPTED_CODE;
    }
    if (eofIndicator != NOT_EOF) {
        fprintf(stderr, "End of file has been reached\n");
        return STDIN_ERROR_CODE;
    }
    if (ferrorIndicator != NO_FERROR) {
        fprintf(stderr, "An error has occured while reading from the standard input\n");
        return STDIN_ERROR_CODE;
    }

    returnCode = parse_long(&lineNo, buf);
    if (returnCode == FAILURE_CODE) {
        fprintf(stderr, "Error while reading a line number: incorrect input\n");
        return FAILURE_CODE;
    }
    if (lineNo < 0 || lineNo > maxNumber) {
        fprintf(stderr, "Line number must be from 1 to %ld, you've entered %ld\n",
                maxNumber, lineNo);
        return FAILURE_CODE;
    }
    if (lineNo == 0) {
        return EXIT_CODE;
    }
    *lineNumber = lineNo;

    return SUCCESS_CODE;
}

int read_line(char *line, int bufLength) {
    int errnoSaved = errno;
    errno = NO_ERROR;
    char *res = fgets(line, bufLength, stdin);
    if (errno == EINTR) {
        return INTERRUPTED_CODE;
    }
    if (res == NULL) {
        return FAILURE_CODE;
    }
    errno = errnoSaved;

    return SUCCESS_CODE;
}

int parse_long(long *val, char *line) {
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
