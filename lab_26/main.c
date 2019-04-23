#include <stdio.h>
#include <libgen.h>
#include <stdlib.h>

#define SUCCESS_CODE (0)
#define FAILURE_CODE (-1)

#define POPEN_ERROR (NULL)
#define PCLOSE_ERROR (-1)
#define FCLOSE_ERROR (EOF)
#define FPUTS_ERROR (EOF)
#define NO_FERROR (-1)

typedef enum ArgvIndex {
    ProgPathIndex,
    SendProgPathIndex,
    RecvProgPathIndex,
    RequiredArgc
} ArgvIndex;

void print_usage(char *progPath) {
    fprintf(stderr, "Usage: %s <send program path> <recv program path>\n", basename(progPath));
}

int open_pipe(char *sendProgPath, char *recvProgPath, FILE **pipeInput, FILE **pipeOutput) {
    FILE *input = popen(sendProgPath, "r");
    if (input == POPEN_ERROR) {
        perror("popen error");
        return FAILURE_CODE;
    }

    FILE *output = popen(recvProgPath, "w");
    if (output == POPEN_ERROR) {
        perror("popen error");
        int returnCode = fclose(input);
        if (returnCode == FCLOSE_ERROR) {
            perror("fclose error");
        }
        return FAILURE_CODE;
    }

    *pipeInput = input;
    *pipeOutput = output;

    return SUCCESS_CODE;
}

int dispatch_message(FILE *input, FILE *output) {
    static char buf[BUFSIZ] = {0};
    while (fgets(buf, BUFSIZ, input) != NULL) {
        int charsWritten = fputs(buf, output);
        if (charsWritten == FPUTS_ERROR) {
            perror("fputs error");
            return FAILURE_CODE;
        }
    }

    int inputErrorIndicator = ferror(input);
    if (inputErrorIndicator) {
        perror("fgets error");
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

int close_pipe(FILE *pipeInput, FILE *pipeOutput) {
    int returnCode = SUCCESS_CODE;

    int pcloseCode = pclose(pipeInput);
    if (pcloseCode == PCLOSE_ERROR) {
        perror("pclose error");
        returnCode = FAILURE_CODE;
    }

    pcloseCode = pclose(pipeOutput);
    if (pcloseCode == PCLOSE_ERROR) {
        perror("pclose error");
        returnCode = FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

int main(int argc, char *argv[]) {
    if (argc != RequiredArgc) {
        char *progPath = argv[ProgPathIndex];
        print_usage(progPath);
        return EXIT_FAILURE;
    }

    char *sendProgPath = argv[SendProgPathIndex];
    char *recvProgPath = argv[RecvProgPathIndex];
    FILE *input = NULL, *output = NULL;

    int returnCode = open_pipe(sendProgPath, recvProgPath, &input, &output);
    if (returnCode == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    int exitCode = EXIT_SUCCESS;
    returnCode = dispatch_message(input, output);
    if (returnCode == FAILURE_CODE) {
        exitCode = EXIT_FAILURE;
    }

    returnCode = close_pipe(input, output);
    if (returnCode == FAILURE_CODE) {
        exitCode = EXIT_FAILURE;
    }

    return exitCode;
}
