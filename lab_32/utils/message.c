#include "message.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errorcodes.h"

#define MALLOC_ERROR (NULL)
#define CALLOC_ERROR (NULL)
#define REALLOC_ERROR (NULL)

#define HEADER_LENGTH (sizeof (long))

Message* message_create(size_t size) {
    Message *message = (Message*) malloc(sizeof (Message));
    if (message == MALLOC_ERROR) {
        perror("message_create::malloc(..) error");
        return MESSAGE_CREATE_ERROR;
    }

    message->buffer = malloc(HEADER_LENGTH + size);
    if (message->buffer == MALLOC_ERROR) {
        perror("message_create::malloc(..) error");
        free(message);
        return MESSAGE_CREATE_ERROR;
    }

    message->size = size;

    return message;
}

Message* message_create_empty(void) {
    return message_create(0);
}

void message_destroy(Message *message) {
    if (message == NULL) {
        return;
    }

    free(message->buffer);
    free(message);
}



long message_get_type(Message *message) {
    long *typePtr = (long*) message->buffer;
    return *typePtr;
}

void message_set_type(Message *message, long type) {
    long *typePtr = (long*) message->buffer;
    *typePtr = type;
}



void* message_get_data(Message *message) {
    void *data = (char*) message->buffer + HEADER_LENGTH;
    return data;
}

int message_set_data(Message *message, const void *newData, size_t dataLength) {
    if (dataLength > message->size) {
        int returnCode = message_resize(message, dataLength);
        if (returnCode == FAILURE_CODE) {
            return FAILURE_CODE;
        }
    }

    void *data = message_get_data(message);
    memcpy(data, newData, dataLength);
    message->size = dataLength;

    return SUCCESS_CODE;
}

int message_resize(Message *message, size_t newSize) {
    void *newBuffer = realloc(message->buffer, newSize + HEADER_LENGTH);
    if (newBuffer == REALLOC_ERROR) {
        perror("message_resize::realloc(..) error");
        return FAILURE_CODE;
    }

    message->buffer = newBuffer;
    message->size = newSize;

    return SUCCESS_CODE;
}
