#include <stdio.h>
#include <stdlib.h>
#include <glob.h>
#include <libgen.h>
#include <string.h>

#include "read_line.h"
#include "errorcodes.h"

#define ERRFUNC_SUCCESS_CODE (0)
#define GLOB_SUCCESS_CODE (0)

int read_pathname_pattern(FILE *file, char **pattern) {
    char *line = NULL;
    size_t lineBufferSize = 0;
    ssize_t lineLength = read_line(&line, &lineBufferSize, file);
    if (lineLength == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    truncate_newline_char(line, (size_t) lineLength);
    *pattern = line;

    return SUCCESS_CODE;
}

int error_function(const char *pathname, int eerrno) {
    fprintf(stderr, "%s: %s\n", pathname, strerror(eerrno));
    return ERRFUNC_SUCCESS_CODE;
}

int print_matching_pathnames(char *pattern) {
    glob_t matchedPathnames;
    int returnCode = glob(pattern, GLOB_NOCHECK, error_function, &matchedPathnames);
    if (returnCode == GLOB_NOSPACE) {
        fprintf(stderr, "glob(..) error: failed to allocate memory\n");
        return FAILURE_CODE;
    }

    char **pathnamesList = matchedPathnames.gl_pathv;
    size_t pathnamesNumber = matchedPathnames.gl_pathc;
    for (size_t pathnameIndex = 0; pathnameIndex < pathnamesNumber; ++pathnameIndex) {
        printf("%s\n", pathnamesList[pathnameIndex]);
    }

    globfree(&matchedPathnames);
    return SUCCESS_CODE;
}

int main(int argc, char *argv[]) {
    char *pathnamePattern = NULL;
    int returnCode = read_pathname_pattern(stdin, &pathnamePattern);
    if (returnCode == FAILURE_CODE) {
        int eofReached = feof(stdin);
        return eofReached ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    returnCode = print_matching_pathnames(pathnamePattern);
    if (returnCode == FAILURE_CODE) {
        free(pathnamePattern);
        return EXIT_FAILURE;
    }

    free(pathnamePattern);
    return EXIT_SUCCESS;
}
