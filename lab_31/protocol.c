//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include "protocol.h"
//#include "errorcodes.h"

//#define MALLOC_ERROR (NULL)
//#define CALLOC_ERROR (NULL)
//#define REALLOC_ERROR (NULL)

//Message* message_create_empty(void) {
//    Message *message = (Message*) malloc(sizeof (Message));
//    if (message == MALLOC_ERROR) {
//        perror("malloc error");
//        return MESSAGE_CREATE_ERROR;
//    }

//    message->size = 0;
//    message->bufferSize = 0;

//    return message;
//}

//Message* message_create(size_t size) {
//    Message *message = message_create_empty();
//    if (message == MESSAGE_CREATE_ERROR) {
//        return MESSAGE_CREATE_ERROR;
//    }

//    size_t bufferSize = sizeof (long) + size;
//    message->buffer = malloc(bufferSize);
//    if (message->buffer == MALLOC_ERROR) {
//        perror("malloc error");
//        message_destroy(message);
//        return MESSAGE_CREATE_ERROR;
//    }

//    message->size = size;
//    message->bufferSize = bufferSize;

//    return message;
//}

//void message_destroy(Message *message) {
//    if (message == NULL) {
//        return;
//    }

//    free(message->buffer);
//    free(message);
//}

//long message_get_type(Message *message) {
//    long *typePtr = (long*) message->buffer;
//    return *typePtr;
//}

//void* message_get_data(Message *message) {
//    void *data = (char*) message->buffer + sizeof (long);
//    return data;
//}

//void message_set_type(Message *message, long type) {
//    long *typePtr = (long*) message->buffer;
//    *typePtr = type;
//}

//static int message_resize_buffer(Message *message, size_t newBufferSize) {
//    void *newBuffer = realloc(message->buffer, newBufferSize);
//    if (newBuffer == REALLOC_ERROR) {
//        perror("realloc error");
//        return FAILURE_CODE;
//    }

//    message->buffer = newBuffer;
//    message->bufferSize = newBufferSize;

//    return SUCCESS_CODE;
//}

//int message_set_data(Message *message, const void *newData, size_t dataLength) {
//    size_t requiredBufferSize = dataLength + sizeof (long);
//    if (requiredBufferSize > message->bufferSize) {
//        int returnCode = message_resize_buffer(message, requiredBufferSize);
//        if (returnCode == FAILURE_CODE) {
//            return FAILURE_CODE;
//        }
//    }

//    void *data = message_get_data(message);
//    memcpy(data, newData, dataLength);
//    message->size = dataLength;

//    return SUCCESS_CODE;
//}



//int expand_buffer(void **buffer, size_t *bufferSize, size_t elemSize) {
//    static const int sizeMultiplier = 2;
//    static const int initialSize = 16;

//    size_t newBufferSize = (*bufferSize == 0) ? initialSize : (*bufferSize) * sizeMultiplier;
//    void *newBuffer = (char*)realloc(*buffer, (size_t) newBufferSize * elemSize);
//    if (newBuffer == REALLOC_ERROR) {
//        perror("realloc error");
//        return FAILURE_CODE;
//    }

//    *buffer = newBuffer;
//    *bufferSize = newBufferSize;

//    return SUCCESS_CODE;
//}

//ListenerList* listeners_create(size_t bufferSize) {
//    if (bufferSize == 0) {
//        return LISTENERS_CREATE_ERROR;
//    }

//    ListenerList *list = malloc(sizeof (ListenerList));
//    if (list == MALLOC_ERROR) {
//        perror("malloc error");
//        return LISTENERS_CREATE_ERROR;
//    }

//    list->list = calloc(bufferSize, sizeof (long));
//    if (list->list == CALLOC_ERROR) {
//        perror("calloc error");
//        free(list);
//        return LISTENERS_CREATE_ERROR;
//    }

//    list->listenersNumber = 0;
//    list->bufferSize = bufferSize;

//    return list;
//}

//void listeners_destroy(ListenerList *list) {
//    if (list == NULL) {
//        return;
//    }

//    free(list->list);
//    free(list);
//}

//int listeners_add(ListenerList *list, long listener) {
//    if (list->listenersNumber == list->bufferSize) {
//        int returnCode = expand_buffer((void**) &list->list, &list->bufferSize, sizeof (long));
//        if (returnCode == FAILURE_CODE) {
//            return FAILURE_CODE;
//        }
//    }

//    size_t listenerIndex = list->listenersNumber;
//    list->list[listenerIndex] = listener;
//    list->listenersNumber++;

//    return SUCCESS_CODE;
//}
