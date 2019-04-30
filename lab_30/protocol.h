#ifndef PROTOCOL_H
#define PROTOCOL_H

#define REGULAR_MESSAGE_SIZE (256)
#define FINAL_MESSAGE_SIZE (0)
#define ACK_MESSAGE_SIZE (0)

#define ANY_MESSAGE_TYPE (0L)
#define REGULAR_MESSAGE_TYPE (1L)
#define FINAL_MESSAGE_TYPE (2L)
#define ACK_MESSAGE_TYPE (3L)

typedef struct message {
    long type;
    char buf[REGULAR_MESSAGE_SIZE];
} Message;

#endif // PROTOCOL_H
