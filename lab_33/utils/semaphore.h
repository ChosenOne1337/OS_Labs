#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stddef.h>
#include <sys/sem.h>
#include "errorcodes.h"

#define SEM_REMOVED_CODE (1)
#define SEM_AGAIN_CODE (2)
#define SEM_INTERRUPTED_CODE (3)

int get_semaphores_set(int ftokID);
int create_semaphores_set(int amount, int ftokID);
int remove_semaphores_set(int semid);

int perform_operation(int semid, short semop, short semflg, unsigned short semIndex);
int perform_operations(int semid, struct sembuf *sops, size_t nsops);

int set_semaphore_value(int semid, unsigned short semIndex, int semval);

void set_operation(struct sembuf *op, short semop, short semflg, unsigned short semIndex);

#endif // SEMAPHORE_H
