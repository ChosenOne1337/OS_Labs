#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>

#define REQUIRED_ARGS_NUM (2)
#define FORK_ERROR_CODE (-1)
#define CHILD_RETURN_CODE (0)
#define WAIT_ERROR_CODE (-1)

#define SUCCESS_CODE (0)
#define FAILURE_CODE (-1)

int wait_for_child_process(void);
int fork_and_exec_command(char *cmdName, char *argv[]);

void print_usage(char *progPath) {
    fprintf(stderr, "Usage: %s <filename>\n", basename(progPath));
}

int main(int argc, char *argv[]) {
    char *progPath = argv[0];
    if (argc != REQUIRED_ARGS_NUM) {
        print_usage(progPath);
        return EXIT_FAILURE;
    }

    char cmd[] = "cat";
    char *filename = argv[1];
    char *cmdArgv[] = {cmd, filename, NULL};
    int returnCode = fork_and_exec_command(cmd, cmdArgv);
    if (returnCode == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    returnCode = wait_for_child_process();
    if (returnCode == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int fork_and_exec_command(char *cmdName, char *argv[]) {
    pid_t forkReturnCode = fork();
    if (forkReturnCode == FORK_ERROR_CODE) {
        perror("fork error");
        return FAILURE_CODE;
    }
    if (forkReturnCode == CHILD_RETURN_CODE) {
        execvp(cmdName, argv);
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
