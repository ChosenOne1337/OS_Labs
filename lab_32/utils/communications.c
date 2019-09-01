#include <stdio.h>
#include <errno.h>
#include "communications.h"
#include "errorcodes.h"

#define FTOK_ERROR (-1)
#define MSGGET_ERROR (-1)
#define MSGCTL_ERROR (-1)
#define MSGRCV_ERROR (-1)
#define MSGSND_ERROR (-1)

#define PERM_MODE       (0660)
#define FTOK_PATHNAME   (".")
#define FTOK_ID         ('$')

static int msgget_wrapper(int msgflg) {
    key_t key = ftok(FTOK_PATHNAME, FTOK_ID);
    if (key == FTOK_ERROR) {
        perror("msgget_wrapper::ftok(..) error");
        return FAILURE_CODE;
    }

    int msgid = msgget(key, msgflg);
    if (msgid == MSGGET_ERROR) {
        perror("msgget_wrapper::msgget(..) error");
        return FAILURE_CODE;
    }

    return msgid;
}

int open_message_queue(void) {
    return msgget_wrapper(MSGFLG_EMPTY);
}

int create_message_queue() {
    return msgget_wrapper(IPC_CREAT | IPC_EXCL | PERM_MODE);
}

int delete_message_queue(int msqid) {
    int returnCode = msgctl(msqid, IPC_RMID, NULL);
    if (returnCode == MSGCTL_ERROR) {
        perror("delete_message_queue::msgctl(..) error");
        return FAILURE_CODE;
    }
    return SUCCESS_CODE;
}


int send_message_formed(int msqid, Message *message) {
    const static int msgflg = 0;
    int returnCode = msgsnd(msqid, message->buffer, message->size, msgflg);
    if (returnCode == FAILURE_CODE) {
        perror("send_message_formed::msgsnd(..) error");
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
    ssize_t returnCode = msgrcv(msqid, message->buffer, message->size, type, msgflg);

    if (returnCode != MSGRCV_ERROR) {
        return SUCCESS_CODE;
    }

    if (errno == E2BIG) {
        return MESSAGE_TOO_BIG_CODE;
    }

    if (errno == ENOMSG) {
        return NO_MESSAGES_CODE;
    }

    if (errno == EINTR) {
        return INTERRUPTED_CODE;
    }

    perror("receive_message::msgrcv(..) error");
    return FAILURE_CODE;
}

int receive_indefinite_size_message(int msqid, long type, int msgflg, Message *message) {
    static const size_t INITIAL_MESSAGE_SIZE = 16;
    static const size_t MESSAGE_SIZE_MULTIPLIER = 2;

    int returnCode;
    for (;;) {
        returnCode = receive_message(msqid, type, msgflg, message);
        if (returnCode != MESSAGE_TOO_BIG_CODE) {
            break;
        }

        size_t newMessageSize = (message->size == 0) ?
                    INITIAL_MESSAGE_SIZE : message->size * MESSAGE_SIZE_MULTIPLIER;
        returnCode = message_resize(message, newMessageSize);
        if (returnCode == FAILURE_CODE) {
            break;
        }
    }

    return returnCode;
}
