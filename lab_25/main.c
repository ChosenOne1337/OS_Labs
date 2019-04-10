#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/types.h>
#include <wait.h>
#include <errno.h>

#define FORK_ERROR_CODE (-1)
#define CHILD_RETURN_CODE (0)
#define WAIT_ERROR_CODE (-1)
#define NO_ERROR (0)

#define PIPE_ERROR (-1)
#define READ_ERROR (-1)
#define WRITE_ERROR (-1)
#define CLOSE_ERROR (-1)

#define SUCCESS_CODE (0)
#define FAILURE_CODE (-1)

#define MSG_SIZE (256)

typedef enum PipeEnd {
    FirstEnd,
    SecondEnd,
    EndsNumber
} PipeEnd;

ssize_t read_message(int fd, char message[MSG_SIZE]) {
    size_t msgPos = 0;
    ssize_t bytesRead = 0;
    while ((bytesRead = read(fd, &message[msgPos], MSG_SIZE - msgPos - 1)) != 0) {
        if (bytesRead == READ_ERROR) {
            perror("read error");
            return FAILURE_CODE;
        }
        msgPos += (size_t) bytesRead;
    }
    return (ssize_t) msgPos;
}

void convert_to_uppercase(char *str, size_t strLength) {
    for (size_t charIndex = 0; charIndex < strLength; ++charIndex) {
        str[charIndex] = (char) toupper(str[charIndex]);
    }
}

int first_process_work(int pipeFildes[EndsNumber]) {
    int exitCode = EXIT_SUCCESS;

    int returnCode = close(pipeFildes[FirstEnd]);
    if (returnCode == CLOSE_ERROR) {
        perror("close error");
        exitCode = EXIT_FAILURE;
    } else {
        char message[MSG_SIZE] = "qWeRtYuIoP aSdFgHjKl ZxCvBnM";
        size_t messageSize = strlen(message);
        printf("Original message:\n%s\n", message);

        ssize_t bytesWritten = write(pipeFildes[SecondEnd], message, messageSize);
        if (bytesWritten == WRITE_ERROR) {
            exitCode = EXIT_FAILURE;
        }
    }

    returnCode = close(pipeFildes[SecondEnd]);
    if (returnCode == CLOSE_ERROR) {
        perror("close error");
        exitCode = EXIT_FAILURE;
    }

    return exitCode;
}

int second_process_work(int pipeFildes[EndsNumber]) {
    int exitCode = EXIT_SUCCESS;

    int returnCode = close(pipeFildes[SecondEnd]);
    if (returnCode == CLOSE_ERROR) {
        perror("close error");
        exitCode = EXIT_FAILURE;
    } else {
        char message[MSG_SIZE] = {0};
        ssize_t bytesRead = read_message(pipeFildes[FirstEnd], message);
        if (bytesRead == FAILURE_CODE) {
            exitCode = EXIT_FAILURE;
        } else {
            convert_to_uppercase(message, (size_t) bytesRead);
            printf("Processed message:\n%s\n", message);
        }
    }

    returnCode = close(pipeFildes[FirstEnd]);
    if (returnCode == CLOSE_ERROR) {
        perror("close error");
        exitCode = EXIT_FAILURE;
    }

    return exitCode;
}

int close_pipe_ends(int pipeFildes[EndsNumber]) {
    int exitCode = EXIT_SUCCESS;

    int returnCode = close(pipeFildes[FirstEnd]);
    if (returnCode == CLOSE_ERROR) {
        perror("close error");
        exitCode = EXIT_FAILURE;
    }

    returnCode = close(pipeFildes[SecondEnd]);
    if (returnCode == CLOSE_ERROR) {
        perror("close error");
        exitCode = EXIT_FAILURE;
    }

    return exitCode;
}

void wait_child_processes(void) {
    errno = NO_ERROR;
    while (errno != ECHILD) {
        wait(NULL);
    }
}

int main(int argc, char *argv[]) {
    int pipeFildes[EndsNumber] = {0};
    int returnCode = pipe(pipeFildes);
    if (returnCode == PIPE_ERROR) {
        perror("pipe error");
        return EXIT_FAILURE;
    }

    pid_t forkReturnCode = fork();
    if (forkReturnCode == FORK_ERROR_CODE) {
        perror("fork error");
        return EXIT_FAILURE;
    }
    if (forkReturnCode == CHILD_RETURN_CODE) {
        int exitCode = first_process_work(pipeFildes);
        return exitCode;
    }

    forkReturnCode = fork();
    if (forkReturnCode == FORK_ERROR_CODE) {
        perror("fork error");
        return EXIT_FAILURE;
    }
    if (forkReturnCode == CHILD_RETURN_CODE) {
        int exitCode = second_process_work(pipeFildes);
        return exitCode;
    }

    int exitCode = close_pipe_ends(pipeFildes);
    wait_child_processes();

    return exitCode;
}
