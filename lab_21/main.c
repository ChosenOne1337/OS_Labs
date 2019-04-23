#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>

#define TRUE (1)
#define FALSE (0)

#define SUCCESS_CODE (0)
#define FAILURE_CODE (-1)

#define FILENO_ERROR (-1)
#define TCSETATTR_ERROR (-1)
#define TCGETATTR_ERROR (-1)

#define DEL_CODE (27)
#define BELL_CODE ('\a')

int signalCount = 0;
int stdinFileDesc = 0;
int stdoutFileDesc = 0;
struct termios ttySaved;
typedef void (*sighandler)(int);

int get_terminal_descriptors(int *inputDesc, int *outputDesc) {
    int stdinFileDesc = fileno(stdin);
    if (stdinFileDesc == FILENO_ERROR) {
        perror("fileno(stdin)");
        return FAILURE_CODE;
    }

    int stdoutFileDesc = fileno(stdout);
    if (stdoutFileDesc == FILENO_ERROR) {
        perror("fileno(stdout)");
        return FAILURE_CODE;
    }

    if (isatty(stdinFileDesc) == FALSE) {
        perror("stdin");
        return FAILURE_CODE;
    }
    if (isatty(stdoutFileDesc) == FALSE) {
        perror("stdout");
        return FAILURE_CODE;
    }

    *inputDesc = stdinFileDesc;
    *outputDesc = stdoutFileDesc;

    return SUCCESS_CODE;
}

int configure_terminal(int stdinFileDesc, struct termios *ttySaved) {
    struct termios tty;
    int returnCode = tcgetattr(stdinFileDesc, &tty);
    if (returnCode == TCGETATTR_ERROR) {
        perror("tcgetattr() error");
        return FAILURE_CODE;
    }

    *ttySaved = tty;
    tty.c_lflag &= ~ECHO;
    tty.c_cc[VINTR] = DEL_CODE;

    returnCode = tcsetattr(stdinFileDesc, TCSAFLUSH, &tty);
    if (returnCode == TCSETATTR_ERROR) {
        perror("tcsetattr() error");
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

int restore_terminal(int stdinFileDesc, struct termios *ttySaved) {
    int returnCode = tcsetattr(stdinFileDesc, TCSAFLUSH, ttySaved);
    if (returnCode == TCSETATTR_ERROR) {
        perror("Failed to restore terminal's attributes");
        return EXIT_FAILURE;
    }
    return SUCCESS_CODE;
}

void sigint_handler(int sig) {
    ++signalCount;
    putchar(BELL_CODE);
    fflush(stdout);
}

void sigquit_handler(int sig) {
    printf("Interrupt signals caught: %d\n", signalCount);
    int returnCode = restore_terminal(stdinFileDesc, &ttySaved);
    if (returnCode == FAILURE_CODE) {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    sighandler prevHandler = sigset(SIGINT, sigint_handler);
    if (prevHandler == SIG_ERR) {
        perror("sigset() error");
        return EXIT_FAILURE;
    }

    prevHandler = sigset(SIGQUIT, sigquit_handler);
    if (prevHandler == SIG_ERR) {
        perror("sigset() error");
        return EXIT_FAILURE;
    }

    int returnCode = get_terminal_descriptors(&stdinFileDesc, &stdoutFileDesc);
    if (returnCode == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    returnCode = configure_terminal(stdinFileDesc, &ttySaved);
    if (returnCode == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    for (;;) {
        pause();
    }

    return EXIT_SUCCESS;
}
