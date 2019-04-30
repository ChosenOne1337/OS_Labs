#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <limits.h>

#define ANY_MESSAGE_TYPE        (0L)
#define REGISTER_REQUEST_TYPE   (LONG_MAX - 2L)
#define REGISTER_RESPONSE_TYPE  (LONG_MAX - 1L)
#define ACK_MESSAGE_TYPE        (LONG_MAX)

#define FINAL_MESSAGE   ("\n$")
#define ACK_MESSAGE     ("")

#define REGISTER_REQUEST_SIZE   (0L)
#define REGISTER_RESPONSE_SIZE  (sizeof (long))
#define FINAL_MESSAGE_SIZE      (sizeof (FINAL_MESSAGE))
#define ACK_MESSAGE_SIZE        (sizeof (ACK_MESSAGE))

#define FTOK_PATHNAME   (".")
#define FTOK_ID         ('$')

//#define LISTENERS_CREATE_ERROR (NULL)

//typedef struct ListenerList {
//    long *list;
//    size_t listenersNumber;
//    size_t bufferSize;
//} ListenerList;

//ListenerList* listeners_create(size_t bufferSize);
//void listeners_destroy(ListenerList *list);

//int listeners_add(ListenerList *list, long listener);

#endif // PROTOCOL_H
