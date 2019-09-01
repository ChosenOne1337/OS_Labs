#include <stdio.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#include "protocol.h"
#include "communications.h"
#include "message.h"
#include "errorcodes.h"
#include "readline.h"
#include "list.h"

Message *form_message(pid_t pid, char *line, size_t lineLength) {
    Message *message = message_create(sizeof (pid) + lineLength + 1);
    if (message == MESSAGE_CREATE_ERROR) {
        return MESSAGE_CREATE_ERROR;
    }

    message_set_type(message, REGULAR_MESSAGE_TYPE);
    void *data = message_get_data(message);

    pid_t *pidPtr = (pid_t*) data;
    *pidPtr = pid;

    char *charPtr = (char*) data + sizeof (pid_t);
    memcpy(charPtr, line, lineLength + 1);

    return message;
}

int send_lines(int msqid) {
    char *line = NULL;
    pid_t pid = getpid();
    size_t lineBufferSize = 0;
    ssize_t charsRead = 0;

    while ((charsRead = read_line(&line, &lineBufferSize, stdin)) != FAILURE_CODE) {
        size_t lineLength = truncate_newline_char(line, (size_t) charsRead);
        Message *message = form_message(pid, line, lineLength);
        if (message == MESSAGE_CREATE_ERROR) {
            free(line);
            return FAILURE_CODE;
        }

        int returnCode = send_message_formed(msqid, message);
        message_destroy(message);
        if (returnCode == FAILURE_CODE) {
            free(line);
            return FAILURE_CODE;
        }
    }

    free(line);

    int eofReached = feof(stdin);
    return eofReached ? SUCCESS_CODE : FAILURE_CODE;
}

int send_final_message(int msqid) {
    pid_t pid = getpid();

    Message *message = form_message(pid, FINAL_MESSAGE, FINAL_MESSAGE_SIZE);
    if (message == MESSAGE_CREATE_ERROR) {
        return FAILURE_CODE;
    }

    int returnCode = send_message_formed(msqid, message);
    message_destroy(message);
    if (returnCode == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}


int main(int argc, char *argv[]) {
    int msqid = open_message_queue();
    if (msqid == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    int returnCode = send_lines(msqid);
    if (returnCode == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    returnCode = send_final_message(msqid);
    if (returnCode == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
