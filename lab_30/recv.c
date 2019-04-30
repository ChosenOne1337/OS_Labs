#include <stdio.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "protocol.h"

#define SUCCESS_CODE (0)
#define FAILURE_CODE (-1)

#define MSGGET_ERROR (-1)
#define MSGRCV_ERROR (-1)
#define MSGSND_ERROR (-1)

int open_message_queue() {
    int msgflg = 0;
    key_t key = (key_t) getuid();
    int msgID = msgget(key, msgflg);
    if (msgID == MSGGET_ERROR) {
        perror("msgget error");
        return FAILURE_CODE;
    }
    return msgID;
}

int receive_messages(int msqID) {
    static Message msg;

    int msgflg = 0;
    do {
        ssize_t bytesReceived = msgrcv(msqID, &msg, REGULAR_MESSAGE_SIZE, ANY_MESSAGE_TYPE, msgflg);
        if (bytesReceived == MSGRCV_ERROR) {
            return FAILURE_CODE;
        }
        if (msg.type == REGULAR_MESSAGE_TYPE) {
            printf("message type: %ld\n"
                   "message size: %zd bytes\n"
                   "message: %s\n\n",
                   REGULAR_MESSAGE_TYPE, bytesReceived, msg.buf);
        }
    } while (msg.type != FINAL_MESSAGE_TYPE);
    
    return SUCCESS_CODE;
}

int send_acknowledge_message(int msqID) {
    static Message msg = {
        .type = ACK_MESSAGE_TYPE,
        .buf = ""
    };

    int msgflg = 0;
    int returnCode = msgsnd(msqID, &msg, ACK_MESSAGE_SIZE, msgflg);
    if (returnCode == MSGSND_ERROR) {
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

int main(int argc, char *argv[]) {
    int msqID = open_message_queue();
    if (msqID == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    int returnCode = receive_messages(msqID);
    if (returnCode == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    returnCode = send_acknowledge_message(msqID);
    if (returnCode == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
