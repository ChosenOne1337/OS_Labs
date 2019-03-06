#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <unistd.h>

#define FAILURE_CODE (1)
#define SUCCESS_CODE (0)

int open_file(char *filename);
int rewind_file(int fd);
void close_file(int fd, char *filename);

int fill_size_table(int fd, size_t *lineSizes, int tableMaxSize);
void fill_offset_table(off_t *lineOffsets, size_t *lineSizes, int tableSize);
int print_line_from_file(int fd, off_t pos, size_t lineSize);

#endif // FILEUTILS_H
