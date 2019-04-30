#include <stdio.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "protocol.h"
#include "errorcodes.h"

int create_message_queue() {
    key_t key = ftok(".", '$');
    if (key == FTOK_ERROR) {
        perror("ftok error");
        return FAILURE_CODE;
    }

    int msgflg = 0;
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
        ssize_t bytesReceived = msgrcv(msqID, &msg, MESSAGE_SIZE, ANY_MESSAGE_TYPE, msgflg);
        if (bytesReceived == MSGRCV_ERROR) {
            return FAILURE_CODE;
        }
        if (msg.type == MESSAGE_TYPE) {
            printf("message type: %ld\n"
                   "message size: %zd bytes\n"
                   "message: %s\n\n",
                   MESSAGE_TYPE, bytesReceived, msg.buf);
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
