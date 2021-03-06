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

int wait_for_child_process(char *progName);
int execute_program(char *progName, char *argv[]);

void print_usage(char *progPath) {
    fprintf(stderr, "Usage: %s <cmd name> <arg1> <arg2> ...\n", basename(progPath));
}

typedef enum ArgvIndex {
    ProgPathIndex,
    CmdNameIndex,
    CmdArgvStartIndex = CmdNameIndex,
    MinArgsNum
} ArgvIndex;

int main(int argc, char *argv[]) {
    char *progPath = argv[ProgPathIndex];
    if (argc < MinArgsNum) {
        print_usage(progPath);
        return EXIT_FAILURE;
    }

    char *cmdName = argv[CmdNameIndex];
    char **cmdArgv = &argv[CmdArgvStartIndex];
    int returnCode = execute_program(cmdName, cmdArgv);
    if (returnCode == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    returnCode = wait_for_child_process(cmdName);
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

int wait_for_child_process(char *progName) {
    int waitStatus = 0;
    pid_t waitReturnCode = wait(&waitStatus);
    if (waitReturnCode == WAIT_ERROR_CODE) {
        perror("wait error");
        return FAILURE_CODE;
    }
    if (WIFEXITED(waitStatus)) {
        int exitStatus = WEXITSTATUS(waitStatus);
        printf("%s exited with code %d\n", progName, exitStatus);
    } else if (WIFSIGNALED(waitStatus)) {
        int termSignal = WTERMSIG(waitStatus);
        printf("%s was terminated by signal %d\n", progName, termSignal);
    }
    return SUCCESS_CODE;
}
