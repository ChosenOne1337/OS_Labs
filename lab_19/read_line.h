#ifndef READ_LINE_H
#define READ_LINE_H

#include <stdio.h>
#include <stddef.h>

#include "errorcodes.h"

ssize_t read_line(char **linePtr, size_t *bufferSize, FILE *file);

int expand_buffer(void **buffer, size_t *bufferSize);

size_t truncate_newline_char(char *line, size_t lineLength);

#endif // READ_LINE_H
