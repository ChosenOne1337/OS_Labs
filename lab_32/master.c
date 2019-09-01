#include <stdio.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include "protocol.h"
#include "errorcodes.h"
#include "communications.h"
#include "message.h"
#include "list.h"

#define STRCMP(a, R, b) (strcmp(a, b) R 0)
#define ALARM_RESET (0)


void decode_message(Message *message, pid_t *pid, char **text) {
    void *data = message_get_data(message);

    pid_t *pidPtr = (pid_t*) data;
    *pid = *pidPtr;

    char *charPtr = (char*) data + sizeof (pid_t);
    *text = charPtr;
}

int add_sender_if_new(List *sendersList, long id) {
    ListNode *listElement = list_find_element(sendersList, id);
    if (listElement != LIST_NO_ELEMENT) {
        return SUCCESS_CODE;
    }

    int returnCode = list_prepend(sendersList, id);
    if (returnCode == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    return SUCCESS_CODE;
}

int handle_messages(int msqid, List *sendersList) {
    Message *message = message_create_empty();
    if (message == MESSAGE_CREATE_ERROR) {
        return FAILURE_CODE;
    }

    pid_t pid = 0;
    char *line = NULL;
    do {
        alarm(RECEIVE_TIMEOUT);
        int returnCode = receive_indefinite_size_message(msqid, REGULAR_MESSAGE_TYPE, MSGFLG_EMPTY, message);
        alarm(ALARM_RESET);
        if (returnCode == FAILURE_CODE) {
            message_destroy(message);
            return FAILURE_CODE;
        }

        decode_message(message, &pid, &line);
        int finalMessageReceived = STRCMP(FINAL_MESSAGE, ==, line);
        if (finalMessageReceived) {
            list_remove_element(sendersList, (long) pid);
            continue;
        }

        returnCode = add_sender_if_new(sendersList, (long) pid);
        if (returnCode == FAILURE_CODE) {
            message_destroy(message);
            return FAILURE_CODE;
        }

        printf("[%d]:\n%s\n\n", pid, line);
    } while (sendersList->length != 0);

    message_destroy(message);
    return SUCCESS_CODE;
}


typedef void (*sighandler_t)(int);

void timeout_handler(int sig) {
    fprintf(stderr, "Timeout: messages haven't been received for a long time\n");
}

int listen_senders(int msqid) {
    sighandler_t prevHandler = signal(SIGALRM, timeout_handler);
    if (prevHandler == SIG_ERR) {
        return FAILURE_CODE;
    }

    List *sendersList = list_create();
    if (sendersList == LIST_CREATE_ERROR) {
        return FAILURE_CODE;
    }

    int returnCode = handle_messages(msqid, sendersList);
    if (returnCode == FAILURE_CODE) {
        list_destroy(sendersList);
        return FAILURE_CODE;
    }

    list_destroy(sendersList);
    return SUCCESS_CODE;
}

int main(int argc, char *argv[]) {
    int msqid = create_message_queue();
    if (msqid == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    int returnCode = listen_senders(msqid);
    if (returnCode == FAILURE_CODE) {
        delete_message_queue(msqid);
        return EXIT_FAILURE;
    }

    delete_message_queue(msqid);
    return EXIT_SUCCESS;
}
