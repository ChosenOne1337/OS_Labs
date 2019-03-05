#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define REQUIRED_ARG_NUM (2)
#define OPEN_ERROR (-1)
#define READ_ERROR (-1)
#define CLOSE_ERROR (-1)

#define FAILURE_CODE (1)
#define SUCCESS_CODE (0)

#define LINES_MAX_NUM (100)

int open_file(char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == OPEN_ERROR) {
        fprintf(stderr, "Failed to open %s: %s", filename, strerror(errno));
        return FAILURE_CODE;
    }
    return fd;
}

void close_file(int fd, char *filename) {
    int returnCode = close(fd);
    if (returnCode == CLOSE_ERROR) {
        fprintf(stderr, "Failed to close %s: %s", filename, strerror(errno));
    }
}

void print_usage(char *progPath) {
    fprintf(stderr, "Usage: %s <input file>\n", basename(progPath));
}

int fill_line_tables(int fd, int *lineSizes, off_t *lineOffsets, int tableMaxSize) {
    static const char LINE_END_CHAR = '\n';
    static char buf[BUFSIZ];
    ssize_t readNum = 0;
    off_t offset = 0;
    int lineSize = 0, lineNo = 0;

    while ((readNum = read(fd, buf, BUFSIZ)) != 0) {
        if (readNum == READ_ERROR) {
            perror("fill_line_tables(..) error while reading from a file");
            return FAILURE_CODE;
        }

        for (int bufPos = 0; bufPos < readNum; ++bufPos) {
            if (buf[bufPos] == LINE_END_CHAR) {
                lineSizes[lineNo] = lineSize + 1;
                lineOffsets[lineNo] = offset;
                offset += lineSize;
                lineSize = 0;
                ++lineNo;
                continue;
            }
            ++lineSize;
        }
    }
    if (lineSize > 0) {
        lineSizes[lineNo] = lineSize;
        lineOffsets[lineNo] = offset;
        ++lineNo;
    }
    // check on a error!
    lseek(fd, 0, SEEK_SET);

    return lineNo;
}

int handle_requests(int fd, int *lineSizes, off_t *lineOffsets, int tableSize) {
    const char prompt[] = "Enter line number from 1 to %d, or 0 to exit\n";


    return SUCCESS_CODE;
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

    static int lineSizes[LINES_MAX_NUM];
    static off_t lineOffsets[LINES_MAX_NUM];

    int linesNum = fill_line_tables(fd, lineSizes, lineOffsets, LINES_MAX_NUM);
    if (linesNum == FAILURE_CODE) {
        close_file(fd, filename);
        exit(EXIT_FAILURE);
    }

    handle_requests(fd, lineSizes, lineOffsets, linesNum);
    close_file(fd, filename);

    return EXIT_SUCCESS;
}
