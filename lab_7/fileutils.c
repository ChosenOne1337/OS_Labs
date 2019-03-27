#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include "fileutils.h"
#include "constants.h"

#define OPEN_ERROR (-1)
#define READ_ERROR (-1)
#define WRITE_ERROR (-1)
#define LSEEK_ERROR (-1)
#define CLOSE_ERROR (-1)
#define MUNMAP_ERROR (-1)

#define FILE_BEG_POS (0)
#define FILE_ZERO_OFFSET (0)

static int map_file_into_memory(FileInfo *fileInfo);
static int fill_size_table(FileInfo *fileInfo, int linesLimit);
static void fill_offset_table(FileInfo *fileInfo);
static int write_with_retry(int fd, const char *buf, size_t bytesToWrite);

int open_file(FileInfo *fileInfo, char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == OPEN_ERROR) {
        fprintf(stderr, "Failed to open %s: %s\n", filename, strerror(errno));
        return FAILURE_CODE;
    }
    fileInfo->fd = fd;
    fileInfo->linesNum = 0;
    fileInfo->mappedFileSize = 0;
    fileInfo->data = NULL;
    fileInfo->lineSizes = NULL;
    fileInfo->lineOffsets = NULL;

    int returnCode = map_file_into_memory(fileInfo);
    if (returnCode == FAILURE_CODE) {
        fprintf(stderr, "open_file(..) error: failed to map the file \"%s\" into memory\n", filename);
        close_file(fileInfo);
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

int close_file(FileInfo *fileInfo) {
    int returnValue = SUCCESS_CODE;
    int errorCode = close(fileInfo->fd);
    if (errorCode == CLOSE_ERROR) {
        perror("Failed to close the file");
        returnValue = FAILURE_CODE;
    }
    if (fileInfo->data != NULL) {
        errorCode = munmap(fileInfo->data, fileInfo->mappedFileSize);
        if (errorCode == MUNMAP_ERROR) {
            perror("Failed to unmap the file");
            returnValue = FAILURE_CODE;
        }
    }
    free(fileInfo->lineSizes);
    free(fileInfo->lineOffsets);
    fileInfo->lineSizes = NULL;
    fileInfo->lineOffsets = NULL;

    return returnValue;
}

static int map_file_into_memory(FileInfo *fileInfo) {
    off_t fileSize = lseek(fileInfo->fd, FILE_ZERO_OFFSET, SEEK_END);
    if (fileSize == LSEEK_ERROR) {
        perror("open_file(..) error while getting a file size");
        return FAILURE_CODE;
    }
    fileInfo->mappedFileSize = (size_t)fileSize;
    char *fileData = (char*)mmap(NULL, fileInfo->mappedFileSize, PROT_READ, MAP_SHARED, fileInfo->fd, FILE_BEG_POS);
    if (fileData == MAP_FAILED) {
        perror("map_file_into_memory(..) error");
        return FAILURE_CODE;
    }
    fileInfo->data = fileData;

    return SUCCESS_CODE;
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
    size_t lineSize = 0;
    int lineNo = 0;

    for (size_t filePos = 0; filePos < fileInfo->mappedFileSize; ++filePos) {
        ++lineSize;
        if (fileInfo->data[filePos] != LINE_END_CHAR) {
            continue;
        }
        if (lineNo >= linesLimit) {
            fprintf(stderr, "fill_line_table(..) error: number of lines exceeded lines limit\n");
            return FAILURE_CODE;
        }
        fileInfo->lineSizes[lineNo++] = lineSize;
        lineSize = 0;
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

int print_file(FileInfo *fileinfo) {
    int returnCode = write_with_retry(STDOUT_FILENO, fileinfo->data, fileinfo->mappedFileSize);
    if (returnCode == FAILURE_CODE) {
        fprintf(stderr, "print_file(..) error: failed to print the file\n");
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

int print_line_from_file(FileInfo *fileinfo, long lineNo) {
    size_t lineSize = fileinfo->lineSizes[lineNo];
    off_t lineBegPos = fileinfo->lineOffsets[lineNo];
    int returnCode = write_with_retry(STDOUT_FILENO, &fileinfo->data[lineBegPos], lineSize);
    if (returnCode == FAILURE_CODE) {
        fprintf(stderr, "print_line_from_file(..) error: failed to print a line from the file\n");
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

static int write_with_retry(int fd, const char *buf, size_t bytesToWrite) {
    ssize_t bytesWritten = 0;
    size_t bytesRemained = bytesToWrite;
    size_t bufPos = 0;

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

    return SUCCESS_CODE;
}
