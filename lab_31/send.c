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
#include "range.h"

#define GETLINE_ERROR (-1)

#define PERM_MODE (0660)

typedef void (*sighandler_t)(int);

void acknowledge_timeout_handler(int sig) {
    fprintf(stderr, "Acknowledge message timeout\n");
}

int create_message_queue() {
    key_t key = ftok(FTOK_PATHNAME, FTOK_ID);
    if (key == FTOK_ERROR) {
        perror("ftok error");
        return FAILURE_CODE;
    }

    int msgflg = IPC_CREAT | PERM_MODE;
    int msqid = msgget(key, msgflg);
    if (msqid == MSGGET_ERROR) {
        perror("msgget error");
        return FAILURE_CODE;
    }

    return msqid;
}

int delete_message_queue(int msqID) {
    int returnCode = msgctl(msqID, IPC_RMID, NULL);
    if (returnCode == MSGCTL_ERROR) {
        perror("msgctl error");
        return FAILURE_CODE;
    }
    return SUCCESS_CODE;
}


int receive_register_request(int msqid) {
    Message *requestMessage = message_create(REGISTER_REQUEST_SIZE);
    if (requestMessage == MESSAGE_CREATE_ERROR) {
        return FAILURE_CODE;
    }

    int returnCode = receive_message(msqid, REGISTER_REQUEST_TYPE, MSGFLG_NO_WAIT, requestMessage);
    if (returnCode == NO_MESSAGES_CODE) {
        message_destroy(requestMessage);
        return NO_MESSAGES_CODE;
    }

    if (returnCode != SUCCESS_CODE) {
        message_destroy(requestMessage);
        return FAILURE_CODE;
    }

    message_destroy(requestMessage);

    return SUCCESS_CODE;
}

int accept_listeners(int msqid, Range *listeners) {
    int returnCode;
    while ((returnCode = receive_register_request(msqid)) == SUCCESS_CODE) {
        long listenersNumber = range_get_extent(listeners);
        long lowestListenerID = range_get_lower_bound(listeners);
        long newListenerID = lowestListenerID + listenersNumber++;
        range_resize(listeners, listenersNumber);

        returnCode = send_message_unformed(msqid, REGISTER_RESPONSE_TYPE, &newListenerID, sizeof (newListenerID));
        if (returnCode == FAILURE_CODE) {
            return FAILURE_CODE;
        }
    }

    if (returnCode == NO_MESSAGES_CODE) {
        return SUCCESS_CODE;
    }

    return FAILURE_CODE;
}


int broadcast_message(int msqid, const void *data, size_t length, Range *listeners) {
    long listenersNumber = range_get_extent(listeners);
    if (listenersNumber == 0) {
        return SUCCESS_CODE;
    }

    Message *message = message_create_empty();
    if (message == MESSAGE_CREATE_ERROR) {
        return FAILURE_CODE;
    }

    int returnCode = message_set_data(message, data, length);
    if (returnCode == FAILURE_CODE) {
        message_destroy(message);
        return FAILURE_CODE;
    }

    long lowestListenerID = range_get_lower_bound(listeners);
    long largestListenerID = range_get_upper_bound(listeners);
    for (long listenerID = lowestListenerID; listenerID < largestListenerID; ++listenerID) {
        message_set_type(message, listenerID);
        int returnCode = send_message_formed(msqid, message);
        if (returnCode == FAILURE_CODE) {
            message_destroy(message);
            return FAILURE_CODE;
        }
    }

    message_destroy(message);

    return SUCCESS_CODE;
}


size_t truncate_newline_char(char *line, size_t lineLength) {
    const static char NEW_LINE_CHAR = '\n';
    const static char END_STRING_CHAR = '\0';

    if (lineLength == 0) {
        return lineLength;
    }

    if (line[lineLength - 1] != NEW_LINE_CHAR) {
        return lineLength;
    }

    line[lineLength - 1] = END_STRING_CHAR;

    return lineLength - 1;
}

// RENAME!!!!!
int send_messages(int msqid, Range *listeners) {
    char *line = NULL;
    size_t lineBufferSize = 0;
    ssize_t charsRead = 0;
    while ((charsRead = getline(&line, &lineBufferSize, stdin)) != GETLINE_ERROR) {
        int returnCode = accept_listeners(msqid, listeners);
        if (returnCode == FAILURE_CODE) {
            free(line);
            return FAILURE_CODE;
        }

        size_t lineLength = truncate_newline_char(line, (size_t) charsRead);
        returnCode = broadcast_message(msqid, line, lineLength + 1, listeners);
        if (returnCode == FAILURE_CODE) {
            free(line);
            return FAILURE_CODE;
        }
    }

    free(line);

    int inputErrorOccured = ferror(stdin);
    if (inputErrorOccured) {
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}


int receive_acknowledge_messages(int msqid, Range *listeners) {
    sighandler_t prevHandler = signal(SIGALRM, acknowledge_timeout_handler);
    if (prevHandler == SIG_ERR) {
        return FAILURE_CODE;
    }

    Message *ackMessage = message_create(ACK_MESSAGE_SIZE);
    if (ackMessage == MESSAGE_CREATE_ERROR) {
        return FAILURE_CODE;
    }

    long listenersNumber = range_get_extent(listeners);
    for (long messagesReceived = 0; messagesReceived < listenersNumber; ++messagesReceived) {
        alarm(ACKNOWLEDGE_TIMEOUT);
        int returnCode = receive_message(msqid, ACK_MESSAGE_TYPE, MSGFLG_EMPTY, ackMessage);
        if (returnCode != SUCCESS_CODE) {
            message_destroy(ackMessage);
            return FAILURE_CODE;
        }
    }

    message_destroy(ackMessage);

    return SUCCESS_CODE;
}


int exchange_messages(int msqid) {
    long lowestListenerID = 1L;
    long listenersNumber = 0L;
    Range listeners = range_make(lowestListenerID, listenersNumber);

    int returnCode = send_messages(msqid, &listeners);
    if (returnCode == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    returnCode = broadcast_message(msqid, FINAL_MESSAGE, FINAL_MESSAGE_SIZE, &listeners);
    if (returnCode == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    returnCode = receive_acknowledge_messages(msqid, &listeners);
    if (returnCode == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}


int main(int argc, char *argv[]) {
    int msqid = create_message_queue();
    if (msqid == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    int exitCode = EXIT_SUCCESS;
    int returnCode = exchange_messages(msqid);
    if (returnCode == FAILURE_CODE) {
        exitCode = EXIT_FAILURE;
    }

    delete_message_queue(msqid);

    return exitCode;
}
