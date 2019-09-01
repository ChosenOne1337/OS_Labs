#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <limits.h>

#define ANY_MESSAGE_TYPE            (0L)
#define REGULAR_MESSAGE_TYPE        (1L)

#define RECEIVE_TIMEOUT             (30)

#define FINAL_MESSAGE               ("\n$")
#define FINAL_MESSAGE_SIZE          (sizeof (FINAL_MESSAGE))

#endif // PROTOCOL_H
