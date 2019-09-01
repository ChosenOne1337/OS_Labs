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
#include "numset.h"

#define ALARM_RESET (0)

typedef void (*sighandler_t)(int);

void acknowledge_timeout_handler(int sig) {}

int receive_register_request(int msqid) {
    Message *requestMessage = message_create(REGISTER_REQUEST_SIZE);
    if (requestMessage == MESSAGE_CREATE_ERROR) {
        return FAILURE_CODE;
    }

    int returnCode = receive_message(msqid, REGISTER_REQUEST_TYPE, MSGFLG_NO_WAIT, requestMessage);
    message_destroy(requestMessage);

    return returnCode;
}

int accept_listeners(int msqid, NumSet *listeners, long *lastListenerID) {
    int returnCode;
    while ((returnCode = receive_register_request(msqid)) == SUCCESS_CODE) {
        long newListenerID = *lastListenerID + 1;
        *lastListenerID = newListenerID;
        returnCode = numset_insert(listeners, newListenerID);
        if (returnCode == FAILURE_CODE) {
            return FAILURE_CODE;
        }

        returnCode = send_message_unformed(msqid, REGISTER_RESPONSE_TYPE,
                                           &newListenerID, REGISTER_RESPONSE_SIZE);
        if (returnCode == FAILURE_CODE) {
            return FAILURE_CODE;
        }
    }

    if (returnCode == NO_MESSAGES_CODE) {
        return SUCCESS_CODE;
    }

    return FAILURE_CODE;
}

int receive_acknowledge_messages(int msqid, NumSet *listeners) {
    Message *ackMessage = message_create(ACK_MESSAGE_SIZE);
    if (ackMessage == MESSAGE_CREATE_ERROR) {
        return FAILURE_CODE;
    }

    int returnCode = SUCCESS_CODE;
    size_t listenersNumber = listeners->valuesNumber;
    numset_clear(listeners);
    for (size_t messagesReceived = 0; messagesReceived < listenersNumber; ++messagesReceived) {
        alarm(ACKNOWLEDGE_TIMEOUT);
        returnCode = receive_message(msqid, ACK_MESSAGE_TYPE, MSGFLG_EMPTY, ackMessage);
        alarm(ALARM_RESET);

        if (returnCode == INTERRUPTED_CODE) {
            returnCode = SUCCESS_CODE;
            break;
        }

        if (returnCode != SUCCESS_CODE) {
            break;
        }

        long *data = (long*) message_get_data(ackMessage);
        long listenerID = *data;
        returnCode = numset_insert(listeners, listenerID);
        if (returnCode == FAILURE_CODE) {
            break;
        }
    }

    message_destroy(ackMessage);
    return returnCode;
}

int broadcast_message(int msqid, const void *data, size_t length, NumSet *listeners) {
    Message *message = message_create_empty();
    if (message == MESSAGE_CREATE_ERROR) {
        return FAILURE_CODE;
    }

    int returnCode = message_set_data(message, data, length);
    if (returnCode == FAILURE_CODE) {
        message_destroy(message);
        return FAILURE_CODE;
    }

    long listenerID = 0;
    size_t listenersNumber = listeners->valuesNumber;
    for (size_t listenerNo = 0; listenerNo < listenersNumber; ++listenerNo) {
        int returnCode = numset_find_first_existing(listeners, listenerID + 1, &listenerID);
        if (returnCode == FAILURE_CODE) {
            break;
        }

        message_set_type(message, listenerID);
        returnCode = send_message_formed(msqid, message);
        if (returnCode == FAILURE_CODE) {
            break;
        }
    }

    message_destroy(message);
    return returnCode;
}


int send_lines(int msqid, NumSet *listeners) {
    char *line = NULL;
    size_t lineBufferSize = 0;
    ssize_t charsRead = 0;
    long lastListenerID = 0;
    while ((charsRead = read_line(&line, &lineBufferSize, stdin)) != FAILURE_CODE) {
        int returnCode = accept_listeners(msqid, listeners, &lastListenerID);
        if (returnCode == FAILURE_CODE) {
            break;
        }

        size_t listenersNumber = listeners->valuesNumber;
        if (listenersNumber == 0) {
            continue;
        }

        size_t lineLength = truncate_newline_char(line, (size_t) charsRead);
        returnCode = broadcast_message(msqid, line, lineLength + 1, listeners);
        if (returnCode == FAILURE_CODE) {
            break;
        }

        returnCode = receive_acknowledge_messages(msqid, listeners);
        if (returnCode != SUCCESS_CODE) {
            break;
        }
    }

    free(line);

    int eofReached = feof(stdin);
    return eofReached ? SUCCESS_CODE : FAILURE_CODE;
}

int send_final_messages(int msqid, NumSet *listeners) {
    size_t listenersNumber = listeners->valuesNumber;
    if (listenersNumber == 0) {
        return SUCCESS_CODE;
    }

    int returnCode = broadcast_message(msqid, FINAL_MESSAGE, FINAL_MESSAGE_SIZE, listeners);
    if (returnCode == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    returnCode = receive_acknowledge_messages(msqid, listeners);
    if (returnCode != SUCCESS_CODE) {
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

int exchange_messages(int msqid) {
    sighandler_t prevHandler = sigset(SIGALRM, acknowledge_timeout_handler);
    if (prevHandler == SIG_ERR) {
        return FAILURE_CODE;
    }

    NumSet *listeners = numset_create();
    if (listeners == NUMSET_CREATE_ERROR) {
        return FAILURE_CODE;
    }

    int returnCode = send_lines(msqid, listeners);
    if (returnCode == FAILURE_CODE) {
        numset_destroy(listeners);
        return FAILURE_CODE;
    }

    returnCode = send_final_messages(msqid, listeners);
    if (returnCode == FAILURE_CODE) {
        numset_destroy(listeners);
        return FAILURE_CODE;
    }

    numset_destroy(listeners);

    return SUCCESS_CODE;
}


int main(int argc, char *argv[]) {
    int msqid = create_message_queue();
    if (msqid == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    int returnCode = exchange_messages(msqid);
    if (returnCode == FAILURE_CODE) {
        delete_message_queue(msqid);
        return EXIT_FAILURE;
    }

    returnCode = delete_message_queue(msqid);
    if (returnCode == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
