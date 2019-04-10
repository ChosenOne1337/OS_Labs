#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>

#define FORK_ERROR_CODE (-1)
#define CHILD_RETURN_CODE (0)
#define WAIT_ERROR_CODE (-1)

#define SUCCESS_CODE (0)
#define FAILURE_CODE (-1)

int wait_for_child_process(void);
int execute_program(char *progName, char *argv[]);

void print_usage(char *progPath) {
    fprintf(stderr, "Usage: %s <filename>\n", basename(progPath));
}

typedef enum ArgvIndex {
    ProgPathIndex,
    FileNameIndex,
    RequiredArgsNum
} ArgvIndex;

int main(int argc, char *argv[]) {
    char *progPath = argv[ProgPathIndex];
    if (argc != RequiredArgsNum) {
        print_usage(progPath);
        return EXIT_FAILURE;
    }

    char cmd[] = "cat";
    char *filename = argv[FileNameIndex];
    char *cmdArgv[] = {cmd, filename, NULL};
    int returnCode = execute_program(cmd, cmdArgv);
    if (returnCode == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    returnCode = wait_for_child_process();
    if (returnCode == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int execute_program(char *progName, char *argv[]) {
    pid_t forkReturnCode = fork();
    if (forkReturnCode == FORK_ERROR_CODE) {
        perror("fork error");
        return FAILURE_CODE;
    }
    if (forkReturnCode == CHILD_RETURN_CODE) {
        execvp(progName, argv);
        perror("execvp error");
        exit(EXIT_FAILURE);
    }
    return SUCCESS_CODE;
}

int wait_for_child_process(void) {
    int waitStatus = 0;
    pid_t waitReturnCode = wait(&waitStatus);
    if (waitReturnCode == WAIT_ERROR_CODE) {
        perror("wait error");
        return FAILURE_CODE;
    }
    if (WIFEXITED(waitStatus)) {
        int exitStatus = WEXITSTATUS(waitStatus);
        printf("Child process exited with code %d\n", exitStatus);
    } else if (WIFSIGNALED(waitStatus)) {
        int termSignal = WTERMSIG(waitStatus);
        printf("Child process was terminated by signal %d\n", termSignal);
    }
    return SUCCESS_CODE;
}
