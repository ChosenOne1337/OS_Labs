#include <stdio.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "protocol.h"
#include "message.h"
#include "errorcodes.h"
#include "range.h"

#define NO_ERROR (0)
#define NO_REQUESTS_CODE (1)
#define GETLINE_ERROR (-1)

#define PERM_MODE (0660)

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

    errno = NO_ERROR;
    msgrcv(msqid, requestMessage->buffer, REGISTER_REQUEST_SIZE, REGISTER_REQUEST_TYPE, IPC_NOWAIT);
    message_destroy(requestMessage);

    if (errno == ENOMSG) {
        return NO_REQUESTS_CODE;
    }

    if (errno != NO_ERROR) {
        perror("msgrcv error");
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

int send_register_response(int msqid, long newListenerID) {
    Message *responseMessage = message_create(REGISTER_RESPONSE_SIZE);
    if (responseMessage == MESSAGE_CREATE_ERROR) {
        return FAILURE_CODE;
    }

    message_set_type(responseMessage, REGISTER_RESPONSE_TYPE);
    int returnCode = message_set_data(responseMessage, &newListenerID, sizeof (newListenerID));
    if (returnCode == FAILURE_CODE) {
        message_destroy(responseMessage);
        return FAILURE_CODE;
    }

    const static int msgflg = 0;
    returnCode = msgsnd(msqid, responseMessage->buffer, responseMessage->size, msgflg);
    message_destroy(responseMessage);
    if (returnCode == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

int accept_listeners(int msqid, Range *listeners) {
    int returnCode;
    while ((returnCode = receive_register_request(msqid)) == SUCCESS_CODE) {
        long listenersNumber = range_get_extent(listeners);
        long lowestListenerID = range_get_lower_bound(listeners);
        long newListenerID = lowestListenerID + listenersNumber++;
        range_resize(listeners, listenersNumber);

        returnCode = send_register_response(msqid, newListenerID);
        if (returnCode == FAILURE_CODE) {
            return FAILURE_CODE;
        }
    }

    if (returnCode == NO_REQUESTS_CODE) {
        return SUCCESS_CODE;
    }

    return FAILURE_CODE;
}

int broadcast_message(int msqid, Message *message, Range *listeners) {
    long largestListenerID = range_get_upper_bound(listeners);
    for (long listenerID = 0; listenerID < largestListenerID; ++listenerID) {
        long type = listenerID;
        message_set_type(message, type);

        const static int msgflg = 0;
        int returnCode = msgsnd(msqid, message->buffer, message->size, msgflg);
        if (returnCode == MSGSND_ERROR) {
            perror("msgsnd error");
            return FAILURE_CODE;
        }
    }

    return SUCCESS_CODE;
}

// RENAME!!!!!
int send_messages(int msqid, Range *listeners) {
    Message *message = message_create_empty();
    if (message == MESSAGE_CREATE_ERROR) {
        return FAILURE_CODE;
    }

    char *line = NULL;
    size_t lineLength = 0;
    ssize_t charsRead = 0;
    int returnCode = SUCCESS_CODE;
    while ((charsRead = getline(&line, &lineLength, stdin)) != GETLINE_ERROR) {
        returnCode = accept_listeners(msqid, listeners);
        if (returnCode == FAILURE_CODE) {
            break;
        }
        returnCode = message_set_data(message, line, lineLength + 1);
        if (returnCode == FAILURE_CODE) {
            break;
        }
        returnCode = broadcast_message(msqid, message, listeners);
        if (returnCode == FAILURE_CODE) {
            break;
        }
    }

    int inputErrorOccured = ferror(stdin);
    if (inputErrorOccured) {
        returnCode = FAILURE_CODE;
    }

    free(line);
    message_destroy(message);

    return returnCode;
}

int send_final_message(int msqid, Range *listeners) {
    Message *finalMessage = message_create_empty();
    if (finalMessage == MESSAGE_CREATE_ERROR) {
        return FAILURE_CODE;
    }

    int returnCode = message_set_data(finalMessage, FINAL_MESSAGE, FINAL_MESSAGE_SIZE);
    if (returnCode == FAILURE_CODE) {
        message_destroy(finalMessage);
        return FAILURE_CODE;
    }

    returnCode = broadcast_message(msqid, finalMessage, listeners);
    if (returnCode == FAILURE_CODE) {
        message_destroy(finalMessage);
        return FAILURE_CODE;
    }

    message_destroy(finalMessage);

    return SUCCESS_CODE;
}

int receive_acknowledge_messages(int msqid, Range *listeners) {
    Message *ackMessage = message_create_empty();
    if (ackMessage == MESSAGE_CREATE_ERROR) {
        return FAILURE_CODE;
    }

    message_set_type(ackMessage, ACK_MESSAGE_TYPE);
    int returnCode = message_set_data(ackMessage, ACK_MESSAGE, ACK_MESSAGE_SIZE);
    if (returnCode == FAILURE_CODE) {
        message_destroy(ackMessage);
        return FAILURE_CODE;
    }

    static const int msgflg = 0;
    long listenersNumber = range_get_extent(listeners);
    for (long messagesReceived = 0; messagesReceived < listenersNumber; ++messagesReceived) {
        ssize_t bytesReceived = msgrcv(msqid, ackMessage->buffer, ackMessage->size, ACK_MESSAGE_TYPE, msgflg);
        if (bytesReceived == MSGRCV_ERROR) {
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

    returnCode = send_final_message(msqid, &listeners);
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
