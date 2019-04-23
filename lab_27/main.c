#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>

#define FCLOSE_ERROR (EOF)
#define PCLOSE_ERROR (-1)
#define FPUTS_ERROR (EOF)

#define SUCCESS_CODE (0)
#define FAILURE_CODE (-1)

#define TRUE (1)
#define FALSE (0)

typedef enum ArgvIndex {
    ProgPathIndex,
    FilePathIndex,
    RequiredArgc
} ArgvIndex;

void print_usage(char *progPath) {
    fprintf(stderr, "Usage: %s <file>\n", basename(progPath));
}

int count_empty_lines(FILE *inputFile, FILE *outputPipe) {
    static const char endLineChar = '\n';
    static const int emptyLineLen = 1;
    static char buf[BUFSIZ];

    int lineHasEnded = TRUE;
    while (fgets(buf, BUFSIZ, inputFile) != NULL) {
        size_t lineLen = strlen(buf);
        if (buf[lineLen - 1] != endLineChar) {
            lineHasEnded = FALSE;
            continue;
        }
        if (lineLen == emptyLineLen && lineHasEnded == TRUE) {
            int charsWritten = fputs(buf, outputPipe);
            if (charsWritten == FPUTS_ERROR) {
                perror("fputs error");
                return FAILURE_CODE;
            }
        }
        lineHasEnded = TRUE;
    }

    int inputErrorIndicator = ferror(inputFile);
    if (inputErrorIndicator) {
        perror("fgets error");
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

int close_files(FILE *inputFile, FILE *outputPipe) {
    int returnCode = SUCCESS_CODE;

    if (inputFile != NULL) {
        int fcloseCode = fclose(inputFile);
        if (fcloseCode == FCLOSE_ERROR) {
            perror("fclose");
            returnCode = FAILURE_CODE;
        }
    }

    if (outputPipe != NULL) {
        int pcloseCode = pclose(outputPipe);
        if (pcloseCode == PCLOSE_ERROR) {
            perror("pclose");
            returnCode = FAILURE_CODE;
        }
    }

    return returnCode;
}

int main(int argc, char *argv[]) {
    if (argc != RequiredArgc) {
        char *progPath = argv[ProgPathIndex];
        print_usage(progPath);
        return EXIT_FAILURE;
    }

    char *filePath = argv[FilePathIndex];
    FILE *inputFile = fopen(filePath, "r");
    if (inputFile == NULL) {
        perror("fopen");
        return EXIT_FAILURE;
    }

    FILE *outputPipe = popen("wc -l", "w");
    if (outputPipe == NULL) {
        close_files(inputFile, outputPipe);
        return EXIT_FAILURE;
    }

    int exitCode = EXIT_SUCCESS;
    int returnCode = count_empty_lines(inputFile, outputPipe);
    if (returnCode == FAILURE_CODE) {
        exitCode = EXIT_FAILURE;
    }

    returnCode = close_files(inputFile, outputPipe);
    if (returnCode == FAILURE_CODE) {
        exitCode = EXIT_FAILURE;
    }

    return exitCode;
}
