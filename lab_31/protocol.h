#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <limits.h>

#define ANY_MESSAGE_TYPE            (0L)
#define REGISTER_REQUEST_TYPE       (LONG_MAX - 2L)
#define REGISTER_RESPONSE_TYPE      (LONG_MAX - 1L)
#define ACK_MESSAGE_TYPE            (LONG_MAX)

#define ACKNOWLEDGE_TIMEOUT         (5UL)

#define REGISTER_REQUEST_MESSAGE    ("")
#define FINAL_MESSAGE               ("\n$")
#define ACK_MESSAGE                 ("")

#define REGISTER_REQUEST_SIZE       (sizeof (REGISTER_REQUEST_MESSAGE))
#define REGISTER_RESPONSE_SIZE      (sizeof (long))
#define FINAL_MESSAGE_SIZE          (sizeof (FINAL_MESSAGE))
#define ACK_MESSAGE_SIZE            (sizeof (ACK_MESSAGE))

#define FTOK_PATHNAME               (".")
#define FTOK_ID                     ('$')

#endif // PROTOCOL_H
