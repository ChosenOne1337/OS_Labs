#include <stdio.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "protocol.h"
#include "errorcodes.h"
#include "communications.h"
#include "message.h"

#define PROG_PATH_INDEX (0)

#define STRCMP(a, R, b) (strcmp(a, b) R 0)

long receive_listener_id(int msqid) {
    int returnCode = send_message_unformed(msqid, REGISTER_REQUEST_TYPE,
                                           REGISTER_REQUEST_MESSAGE, REGISTER_REQUEST_SIZE);
    if (returnCode == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    Message *responseMessage = message_create(REGISTER_RESPONSE_SIZE);
    if (responseMessage == MESSAGE_CREATE_ERROR) {
        return FAILURE_CODE;
    }

    returnCode = receive_message(msqid, REGISTER_RESPONSE_TYPE, MSGFLG_EMPTY, responseMessage);
    if (returnCode == FAILURE_CODE) {
        message_destroy(responseMessage);
        return FAILURE_CODE;
    }

    long *data = (long*) message_get_data(responseMessage);
    long listenerID = *data;

    message_destroy(responseMessage);

    return listenerID;
}

int handle_messages(int msqid, long listenerID, char *progPath) {
    Message *message = message_create_empty();
    if (message == MESSAGE_CREATE_ERROR) {
        return FAILURE_CODE;
    }

    long messageType = listenerID;
    int finalMessageReceived = FALSE;
    for (;;) {
        int returnCode = receive_indefinite_size_message(msqid, messageType, MSGFLG_EMPTY, message);
        if (returnCode == FAILURE_CODE) {
            break;
        }

        returnCode = send_message_unformed(msqid, ACK_MESSAGE_TYPE, &listenerID, ACK_MESSAGE_SIZE);
        if (returnCode == FAILURE_CODE) {
            break;
        }

        char *line = message_get_data(message);
        finalMessageReceived = STRCMP(FINAL_MESSAGE, ==, line);
        if (finalMessageReceived) {
            break;
        }

        printf("[%s]:\n%s\n\n", progPath, line);
    }

    message_destroy(message);

    return finalMessageReceived ? SUCCESS_CODE : FAILURE_CODE;
}

int main(int argc, char *argv[]) {
    int msqid = open_message_queue();
    if (msqid == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    long listenerID = receive_listener_id(msqid);
    if (listenerID == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    char *progPath = argv[PROG_PATH_INDEX];
    int returnCode = handle_messages(msqid, listenerID, progPath);
    if (returnCode == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
