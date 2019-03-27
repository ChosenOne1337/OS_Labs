#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <unistd.h>

#define NOT_EOF (0)
#define NO_FERROR (0)

typedef struct FileInfo {
    int fd;
    size_t mappedFileSize;
    char *data;
    int linesNum;
    size_t *lineSizes;
    off_t *lineOffsets;
} FileInfo;

int open_file(FileInfo *fileInfo, char *filename);
int close_file(FileInfo *fileInfo);

int fill_file_info(FileInfo *fileInfo, int linesLimit);
int print_line_from_file(FileInfo *fileinfo, long lineNo);
int print_file(FileInfo *fileinfo);

#endif // FILEUTILS_H
