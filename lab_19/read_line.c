#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "read_line.h"

#define REALLOC_ERROR (NULL)
#define FGETS_ERROR (NULL)

int expand_buffer(void **buffer, size_t *bufferSize) {
    static const size_t INITIAL_SIZE = 16;
    static const size_t SIZE_MULIIPLIER = 2;

    size_t newBufferSize = (*bufferSize == 0) ? INITIAL_SIZE : SIZE_MULIIPLIER * (*bufferSize);
    void *newBuffer = realloc(*buffer, newBufferSize);
    if (newBuffer == REALLOC_ERROR) {
        perror("expand_buffer::realloc(..) error");
        return FAILURE_CODE;
    }

    *buffer = newBuffer;
    *bufferSize = newBufferSize;

    return SUCCESS_CODE;
}

ssize_t read_line(char **linePtr, size_t *bufferSize, FILE *file) {
    static const char NEWLINE_CHAR = '\n';

    char *line = NULL;
    size_t bufferPos = 0;
    do {
        if (*linePtr == NULL || *bufferSize == 0 || bufferPos == *bufferSize - 1) {
            int returnCode = expand_buffer((void**) linePtr, bufferSize);
            if (returnCode == FAILURE_CODE) {
                return FAILURE_CODE;
            }
        }

        line = *linePtr;
        char *returnCode = fgets(&line[bufferPos], (int) (*bufferSize - bufferPos), file);
        if (returnCode == FGETS_ERROR) {
            break;
        }

        bufferPos += strlen(&line[bufferPos]);
    } while (line[bufferPos - 1] != NEWLINE_CHAR);

    int errorOccured = ferror(file);
    if (errorOccured) {
        return FAILURE_CODE;
    }

    int eofReached = feof(file);
    if (eofReached) {
        return (bufferPos > 0) ? (ssize_t) bufferPos : FAILURE_CODE;
    }

    return (ssize_t) bufferPos;
}

size_t truncate_newline_char(char *line, size_t lineLength) {
    const static char NEWLINE_CHAR = '\n';
    const static char END_STRING_CHAR = '\0';

    if (lineLength == 0) {
        return lineLength;
    }

    if (line[lineLength - 1] != NEWLINE_CHAR) {
        return lineLength;
    }

    line[lineLength - 1] = END_STRING_CHAR;

    return lineLength - 1;
}
