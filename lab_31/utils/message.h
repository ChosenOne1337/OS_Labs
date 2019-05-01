#ifndef MESSAGE_H
#define MESSAGE_H

#include <stddef.h>

#define MESSAGE_CREATE_ERROR (NULL)
#define MESSAGE_SET_DATA_ERROR (NULL)

typedef struct Message {
    void *buffer;
    size_t size;
    size_t bufferSize;
} Message;

Message* message_create_empty(void);
Message* message_create(size_t size);
void message_destroy(Message *message);

long message_get_type(Message *message);
void* message_get_data(Message *message);

int message_resize(Message *message, size_t newSize);
void message_set_type(Message *message, long type);
int message_set_data(Message *message, const void *newData, size_t dataLength);


#endif // MESSAGE_H
