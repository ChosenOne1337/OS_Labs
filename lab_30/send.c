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
#define MSGCTL_ERROR (-1)
#define MSGRCV_ERROR (-1)
#define MSGSND_ERROR (-1)

#define PERM_MODE (0660)

int create_message_queue() {
    key_t key = (key_t) getuid();
    int msgflg = IPC_CREAT | PERM_MODE;
    int msgID = msgget(key, msgflg);
    if (msgID == MSGGET_ERROR) {
        perror("msgget error");
        return FAILURE_CODE;
    }
    return msgID;
}

int send_messages(int msqID) {
    static const char *MESSAGES[] = {
        "Hello!",
        "How are you???",
        "Well, I have to go...",
        "Bye!"
    };
    static const size_t MESSAGES_NUMBER = sizeof (MESSAGES) / sizeof (char*);

    static Message msg = {
        .type = REGULAR_MESSAGE_TYPE,
        .buf = ""
    };

    int msgflg = 0;
    for (size_t msgIndex = 0; msgIndex < MESSAGES_NUMBER; ++msgIndex) {
        strncpy(msg.buf, MESSAGES[msgIndex], REGULAR_MESSAGE_SIZE - 1);
        size_t messageLength = strlen(msg.buf) + 1;
        int returnCode = msgsnd(msqID, &msg, messageLength, msgflg);
        if (returnCode == MSGSND_ERROR) {
            perror("msgsnd error");
            return FAILURE_CODE;
        }
    }

    return SUCCESS_CODE;
}

int send_final_message(int msqID) {
    static Message msg = {
        .type = FINAL_MESSAGE_TYPE,
        .buf = ""
    };

    int msgflg = 0;
    int returnCode = msgsnd(msqID, &msg, FINAL_MESSAGE_SIZE, msgflg);
    if (returnCode == MSGSND_ERROR) {
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

int receive_acknowledge_message(int msqID) {
    static Message msg;

    int msgflg = 0;
    ssize_t bytesReceived = msgrcv(msqID, &msg, ACK_MESSAGE_SIZE, ACK_MESSAGE_TYPE, msgflg);
    if (bytesReceived == MSGRCV_ERROR) {
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

int delete_message_queue(int msqID) {
    int returnCode = msgctl(msqID, IPC_RMID, NULL);
    if (returnCode == MSGCTL_ERROR) {
        perror("msgctl error");
        return FAILURE_CODE;
    }
    return SUCCESS_CODE;
}

int main(int argc, char *argv[]) {
    int msqID = create_message_queue();
    if (msqID == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    int returnCode = send_messages(msqID);
    if (returnCode == FAILURE_CODE) {
        delete_message_queue(msqID);
        return EXIT_FAILURE;
    }

    returnCode = send_final_message(msqID);
    if (returnCode == FAILURE_CODE) {
        delete_message_queue(msqID);
        return EXIT_FAILURE;
    }

    returnCode = receive_acknowledge_message(msqID);
    if (returnCode == FAILURE_CODE) {
        delete_message_queue(msqID);
        return EXIT_FAILURE;
    }

    returnCode = delete_message_queue(msqID);
    if (returnCode == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
