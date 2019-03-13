#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "fileutils.h"
#include "constants.h"

#define OPEN_ERROR (-1)
#define READ_ERROR (-1)
#define WRITE_ERROR (-1)
#define LSEEK_ERROR (-1)
#define CLOSE_ERROR (-1)

#define FILE_BEG_POS (0)
#define BUFFER_SIZE BUFSIZ

static char buf[BUFFER_SIZE];

static int fill_size_table(FileInfo *fileInfo, int linesLimit);
static void fill_offset_table(FileInfo *fileInfo);
static int read_with_retry(int fd, char *buf, size_t bytesToRead);
static int write_with_retry(int fd, const char *buf, size_t bytesToWrite);

int open_file(FileInfo *fileInfo, char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == OPEN_ERROR) {
        fprintf(stderr, "Failed to open %s: %s\n", filename, strerror(errno));
        return FAILURE_CODE;
    }
    fileInfo->fd = fd;
    fileInfo->linesNum = 0;
    fileInfo->lineSizes = NULL;
    fileInfo->lineOffsets = NULL;

    return SUCCESS_CODE;
}

int close_file(FileInfo *fileInfo) {
    int returnCode = close(fileInfo->fd);
    if (returnCode == CLOSE_ERROR) {
        perror("Failed to close a file");
        returnCode = FAILURE_CODE;
    } else {
        returnCode = SUCCESS_CODE;
    }
    free(fileInfo->lineSizes);
    free(fileInfo->lineOffsets);
    fileInfo->lineSizes = NULL;
    fileInfo->lineOffsets = NULL;

    return returnCode;
}

int fill_file_info(FileInfo *fileInfo, int linesLimit) {
    fileInfo->lineSizes = (size_t*)calloc((size_t)linesLimit, sizeof(size_t));
    fileInfo->lineOffsets = (off_t*)calloc((size_t)linesLimit, sizeof(off_t));
    if (fileInfo->lineSizes == NULL || fileInfo->lineOffsets == NULL) {
        free(fileInfo->lineSizes);
        free(fileInfo->lineOffsets);
        fileInfo->lineSizes = NULL;
        fileInfo->lineOffsets = NULL;
        perror("fill_file_info(..) error");
        return FAILURE_CODE;
    }

    int returnCode = fill_size_table(fileInfo, linesLimit);
    if (returnCode == FAILURE_CODE) {
        free(fileInfo->lineSizes);
        free(fileInfo->lineOffsets);
        fileInfo->lineSizes = NULL;
        fileInfo->lineOffsets = NULL;
        fprintf(stderr, "Failed to fill the file info\n");
        return FAILURE_CODE;
    }
    fill_offset_table(fileInfo);

    void *ptr = realloc(fileInfo->lineSizes, (size_t)fileInfo->linesNum * sizeof(size_t));
    if (ptr != NULL) {
        fileInfo->lineSizes = (size_t*)ptr;
    }
    ptr = realloc(fileInfo->lineOffsets, (size_t)fileInfo->linesNum * sizeof(off_t));
    if (ptr != NULL) {
        fileInfo->lineOffsets = (off_t*)ptr;
    }

    return SUCCESS_CODE;
}

static int fill_size_table(FileInfo *fileInfo, int linesLimit) {
    static const char LINE_END_CHAR = '\n';
    ssize_t charsRead = 0;
    size_t lineSize = 0;
    int lineNo = 0;

    while ((charsRead = read(fileInfo->fd, buf, BUFFER_SIZE)) != 0) {
        if (charsRead == READ_ERROR) {
            perror("fill_line_table(..) error while reading from a file");
            return FAILURE_CODE;
        }
        for (ssize_t bufPos = 0; bufPos < charsRead; ++bufPos) {
            ++lineSize;
            if (buf[bufPos] != LINE_END_CHAR) {
                continue;
            }
            if (lineNo >= linesLimit) {
                fprintf(stderr, "fill_line_table(..) error: "
                                "number of lines exceeded lines limit\n");
                return FAILURE_CODE;
            }
            fileInfo->lineSizes[lineNo++] = lineSize;
            lineSize = 0;
        }
    }
    if (lineSize > 0) {
        fileInfo->lineSizes[lineNo++] = lineSize;
    }
    fileInfo->linesNum = lineNo;

    return SUCCESS_CODE;
}

static void fill_offset_table(FileInfo *fileInfo) {
    off_t offset = 0;
    for (int lineNo = 0; lineNo < fileInfo->linesNum; ++lineNo) {
        fileInfo->lineOffsets[lineNo] = offset;
        offset += fileInfo->lineSizes[lineNo];
    }
}

int print_line_from_file(int fd, off_t pos, size_t lineSize) {
    off_t returnVal = lseek(fd, pos, SEEK_SET);
    if (returnVal == LSEEK_ERROR) {
        perror("print_line_from_file(..) error while setting starting position in a file");
        return FAILURE_CODE;
    }

    int returnCode = 0;
    size_t charsRemained = lineSize, charsToPrint = 0;
    while (charsRemained > 0) {
        charsToPrint = charsRemained < BUFFER_SIZE ? charsRemained : BUFFER_SIZE;
        returnCode = read_with_retry(fd, buf, charsToPrint);
        if (returnCode == FAILURE_CODE) {
            fprintf(stderr, "print_line_from_file(..) error: failed to read a line\n");
            return FAILURE_CODE;
        }
        returnCode = write_with_retry(STDOUT_FILENO, buf, charsToPrint);
        if (returnCode == FAILURE_CODE) {
            fprintf(stderr, "print_line_from_file(..) error: failed to print a line\n");
            return FAILURE_CODE;
        }
        charsRemained -= charsToPrint;
    }
    return SUCCESS_CODE;
}

static int read_with_retry(int fd, char *buf, size_t bytesToRead) {
    ssize_t bytesRead = 0;
    size_t bytesRemained = bytesToRead;
    size_t bufPos = 0;

    int errnoSaved = errno;
    do {
        errno = NO_ERROR;
        bufPos = bytesToRead - bytesRemained;
        bytesRead = read(fd, &buf[bufPos], bytesRemained);
        if (errno == EINTR || errno == EAGAIN) {
            continue;
        }
        if (bytesRead == READ_ERROR) {
            perror("read_with_retry(..) error");
            return FAILURE_CODE;
        }
        if (bytesRead == 0) {
            fprintf(stderr, "read_with_retry(..) error: end of file has been reached\n");
            return FAILURE_CODE;
        }
        bytesRemained -= (size_t)bytesRead;
    } while (bytesRemained > 0);
    errno = errnoSaved;

    return SUCCESS_CODE;
}

static int write_with_retry(int fd, const char *buf, size_t bytesToWrite) {
    ssize_t bytesWritten = 0;
    size_t bytesRemained = bytesToWrite;
    size_t bufPos = 0;

    int errnoSaved = errno;
    do {
        errno = NO_ERROR;
        bufPos = bytesToWrite - bytesRemained;
        bytesWritten = write(fd, &buf[bufPos], bytesRemained);
        if (errno == EINTR || errno == EAGAIN) {
            continue;
        }
        if (bytesWritten == WRITE_ERROR) {
            perror("write_with_retry(..) error");
            return FAILURE_CODE;
        }
        bytesRemained -= (size_t)bytesWritten;
    } while (bytesRemained > 0);
    errno = errnoSaved;

    return SUCCESS_CODE;
}
