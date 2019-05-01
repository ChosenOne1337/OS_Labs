#include <stdio.h>
#include <errno.h>
#include "communications.h"
#include "errorcodes.h"

#define ERRNO_NOERROR (0)

int send_message_formed(int msqid, Message *message) {
    const static int msgflg = 0;
    int returnCode = msgsnd(msqid, message->buffer, message->size, msgflg);
    if (returnCode == FAILURE_CODE) {
        perror("msgsnd error");
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

int send_message_unformed(int msqid, long type, const void *data, size_t length) {
    Message *message = message_create_empty();
    if (message == MESSAGE_CREATE_ERROR) {
        return FAILURE_CODE;
    }

    message_set_type(message, type);
    int returnCode = message_set_data(message, data, length);
    if (returnCode == FAILURE_CODE) {
        message_destroy(message);
        return FAILURE_CODE;
    }

    returnCode = send_message_formed(msqid, message);
    if (returnCode == FAILURE_CODE) {
        message_destroy(message);
        return FAILURE_CODE;
    }

    message_destroy(message);

    return SUCCESS_CODE;
}

int receive_message(int msqid, long type, int msgflg, Message *message) {
    errno = ERRNO_NOERROR;
    ssize_t returnCode = msgrcv(msqid, message->buffer, message->size, type, msgflg);

    if (errno == E2BIG) {
        return MESSAGE_TOO_BIG_CODE;
    }

    if (errno == ENOMSG) {
        return NO_MESSAGES_CODE;
    }

    if (returnCode == MSGRCV_ERROR) {
        perror("msgrcv error");
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

int receive_indefinite_size_message(int msqid, long type, int msgflg, Message *message) {
    const static size_t INITIAL_MESSAGE_SIZE = 16;
    const static size_t MESSAGE_SIZE_MULTIPLIER = 2;

    for (;;) {
        int returnCode = receive_message(msqid, type, msgflg, message);
        if (returnCode == FAILURE_CODE) {
            break;
        }

        if (returnCode == NO_MESSAGES_CODE) {
            return NO_MESSAGES_CODE;
        }

        if (returnCode != MESSAGE_TOO_BIG_CODE) {
            return SUCCESS_CODE;
        }

        size_t newMessageSize = (message->size == 0) ? INITIAL_MESSAGE_SIZE :
                                                     message->size * MESSAGE_SIZE_MULTIPLIER;
        returnCode = message_resize(message, newMessageSize);
        if (returnCode == FAILURE_CODE) {
            break;
        }
    }

    return FAILURE_CODE;
}
