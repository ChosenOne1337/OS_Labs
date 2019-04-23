#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <libgen.h>

#define FCLOSE_ERROR (-1)
#define PCLOSE_ERROR (-1)
#define P2OPEN_ERROR (-1)
#define TIME_ERROR (-1)
#define NO_FERROR (0)

#define SUCCESS_CODE (0)
#define FAILURE_CODE (-1)

#define MAX_VALUE (100)
#define LINE_LENGTH (10)
#define SEQUENCE_LENGTH (100)

typedef enum PipeIndex {
    OutputPipeIndex,
    InputPipeIndex,
    TotalPipesNumber
} PipeIndex;

int set_seed(void) {
    time_t currTime = time(NULL);
    if (currTime == TIME_ERROR) {
        perror("time error");
        return FAILURE_CODE;
    }
    srand((unsigned)currTime);
    return SUCCESS_CODE;
}

int write_random_sequence(FILE *output, size_t maxValue, size_t sequenceLength) {
    int bytesWritten = 0;
    size_t randomNumber = 0;
    for (size_t numIndex = 0; numIndex < sequenceLength; ++numIndex) {
        randomNumber = (size_t) rand() % maxValue;
        bytesWritten = fprintf(output, "%zu\n", randomNumber);
        if (bytesWritten < 0) {
            perror("fprintf error");
            return FAILURE_CODE;
        }
    }
    return SUCCESS_CODE;
}

int read_sorted_sequence(FILE *input) {
    size_t number = 0;
    int numbersOnLine = 0;
    printf("Sorted number sequence:\n");
    while (fscanf(input, "%zu", &number) != EOF) {
        printf("%2zu ", number);
        ++numbersOnLine;
        if (numbersOnLine == LINE_LENGTH) {
            numbersOnLine = 0;
            printf("\n");
        }
    }
    if (numbersOnLine > 0) {
        printf("\n");
    }

    int errorIndicator = ferror(input);
    if (errorIndicator != NO_FERROR) {
        perror("fscanf error");
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

int main(int argc, char *argv[]) {
    FILE *pipes[TotalPipesNumber] = {NULL, NULL};
    int returnCode = p2open("sort -n", pipes);
    if (returnCode == P2OPEN_ERROR) {
        perror("p2open error");
        return EXIT_FAILURE;
    }

    set_seed();
    int exitCode = EXIT_SUCCESS;
    returnCode = write_random_sequence(pipes[OutputPipeIndex], MAX_VALUE, SEQUENCE_LENGTH);
    if (returnCode == FAILURE_CODE) {
        exitCode = EXIT_FAILURE;
    }

    returnCode = fclose(pipes[OutputPipeIndex]);
    if (returnCode == FCLOSE_ERROR) {
        perror("fclose error");
        exitCode = EXIT_FAILURE;
    }

    if (exitCode == EXIT_SUCCESS) {
        returnCode = read_sorted_sequence(pipes[InputPipeIndex]);
        if (returnCode == FAILURE_CODE) {
            exitCode = EXIT_FAILURE;
        }
    }

    returnCode = pclose(pipes[InputPipeIndex]);
    if (returnCode == FAILURE_CODE) {
        perror("pclose error");
        exitCode = EXIT_FAILURE;
    }

    return exitCode;
}
