#include "message.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errorcodes.h"

#define MALLOC_ERROR (NULL)
#define CALLOC_ERROR (NULL)
#define REALLOC_ERROR (NULL)

static int message_resize_buffer(Message *message, size_t newBufferSize) {
    void *newBuffer = realloc(message->buffer, newBufferSize);
    if (newBuffer == REALLOC_ERROR) {
        perror("realloc error");
        return FAILURE_CODE;
    }

    message->buffer = newBuffer;
    message->bufferSize = newBufferSize;

    return SUCCESS_CODE;
}

Message* message_create(size_t size) {
    Message *message = (Message*) malloc(sizeof (Message));
    if (message == MALLOC_ERROR) {
        perror("malloc error");
        return MESSAGE_CREATE_ERROR;
    }

    size_t bufferSize = sizeof (long) + size;
    message->buffer = malloc(bufferSize);
    if (message->buffer == MALLOC_ERROR) {
        perror("malloc error");
        free(message);
        return MESSAGE_CREATE_ERROR;
    }

    message->size = size;
    message->bufferSize = bufferSize;

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
    void *data = (char*) message->buffer + sizeof (long);
    return data;
}

int message_set_data(Message *message, const void *newData, size_t dataLength) {
    size_t requiredBufferSize = dataLength + sizeof (long);
    if (requiredBufferSize > message->bufferSize) {
        int returnCode = message_resize_buffer(message, requiredBufferSize);
        if (returnCode == FAILURE_CODE) {
            return FAILURE_CODE;
        }
    }

    void *data = message_get_data(message);
    memcpy(data, newData, dataLength);
    message->size = dataLength;

    return SUCCESS_CODE;
}
