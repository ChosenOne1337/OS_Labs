#ifndef COMMUNICATIONS_H
#define COMMUNICATIONS_H

#include "message.h"
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>

#define MSGFLG_EMPTY (0)
#define MSGFLG_NO_WAIT (IPC_NOWAIT)

#define MESSAGE_TOO_BIG_CODE (1)
#define NO_MESSAGES_CODE (2)

int send_message_formed(int msqid, Message *message);
int send_message_unformed(int msqid, long type, const void *data, size_t length);

int receive_message(int msqid, long type, int msgflg, Message *message);
int receive_indefinite_size_message(int msqid, long type, int msgflg, Message *message);

#endif // COMMUNICATIONS_H
