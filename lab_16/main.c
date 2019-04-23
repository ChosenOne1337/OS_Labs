#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>

#define TRUE (1)
#define FALSE (0)

#define YES_ANSWER ('y')
#define NO_ANSWER ('n')

#define SUCCESS_CODE (0)
#define FAILURE_CODE (-1)

#define READ_ERROR (-1)
#define WRITE_ERROR (-1)
#define FILENO_ERROR (-1)
#define TCSETATTR_ERROR (-1)
#define TCGETATTR_ERROR (-1)

#define NONCANON_INPUT_LEN (1)

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
        perror("isatty(stdin)");
        return FAILURE_CODE;
    }
    if (isatty(stdoutFileDesc) == FALSE) {
        perror("isatty(stdout)");
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
    tty.c_lflag &= ~(ISIG | ICANON | ECHO);
    tty.c_iflag |= IUCLC;
    tty.c_cc[VMIN] = NONCANON_INPUT_LEN;

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

int ask_question(int inputFileDesc, int outputFileDesc) {
    char question[] = "Do you like the weather today? [Y/n]: ";
    ssize_t charsWritten = write(outputFileDesc, question, sizeof(question) - 1);
    if (charsWritten == WRITE_ERROR) {
        perror("write() error");
        return FAILURE_CODE;
    }

    char answer = 0;
    char defaultAnswer = YES_ANSWER;
    ssize_t charsRead = read(inputFileDesc, &answer, NONCANON_INPUT_LEN);
    if (charsRead == READ_ERROR) {
        perror("read() error");
        return FAILURE_CODE;
    }

    if (answer != YES_ANSWER && answer != NO_ANSWER) {
        return defaultAnswer;
    }

    return answer;
}

int main(int argc, char *argv[]) {
    int stdinFileDesc = 0, stdoutFileDesc = 0;
    int returnCode = get_terminal_descriptors(&stdinFileDesc, &stdoutFileDesc);
    if (returnCode == FAILURE_CODE) {
        fprintf(stderr, "stdin/stdout are not associated with the terminal\n");
        return EXIT_FAILURE;
    }

    struct termios ttySaved;
    returnCode = configure_terminal(stdinFileDesc, &ttySaved);
    if (returnCode == FAILURE_CODE) {
        fprintf(stderr, "Failed to set terminal's attributes\n");
        return EXIT_FAILURE;
    }

    int answer = ask_question(stdinFileDesc, stdoutFileDesc);
    if (answer == FAILURE_CODE) {
        fprintf(stderr, "Failed to ask the question\n");
        restore_terminal(stdinFileDesc, &ttySaved);
        return EXIT_FAILURE;
    }
    printf("%s\n", answer == YES_ANSWER ? "yes" : "no");

    returnCode = restore_terminal(stdinFileDesc, &ttySaved);
    if (returnCode == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
