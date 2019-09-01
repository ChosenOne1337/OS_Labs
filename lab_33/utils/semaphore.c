#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <errno.h>
#include "semaphore.h"

#define PLACEHOLDER (0)
#define FTOK_PATHNAME   (".")

#define FTOK_ERROR (-1)
#define SEMGET_ERROR (-1)
#define SEMCTL_ERROR (-1)
#define SEMOP_ERROR (-1)
#define PERM_MODE (0660)

#define NO_ERROR (0)

static int semget_wrapper(int amount, int semflg, int ftokID) {
    key_t semkey = ftok(FTOK_PATHNAME, ftokID);
    if (semkey == FTOK_ERROR) {
        perror("semget_wrapper::ftok(..) error");
        return FAILURE_CODE;
    }

    int semid = semget(semkey, amount, semflg);
    if (semid == SEMGET_ERROR) {
        perror("semget_wrapper::semget(..) error");
        return FAILURE_CODE;
    }

    return semid;
}

int create_semaphores_set(int amount, int ftokID) {
    int semflg = IPC_CREAT | IPC_EXCL | PERM_MODE;
    int returnCode = semget_wrapper(amount, semflg, ftokID);
    return returnCode;
}

int get_semaphores_set(int ftokID) {
    int amount = 0, semflg = 0;
    int returnCode = semget_wrapper(amount, semflg, ftokID);
    return returnCode;
}

int perform_operations(int semid, struct sembuf *sops, size_t nsops) {
    errno = NO_ERROR;
    int returnCode = semop(semid, sops, nsops);
    if (returnCode != SEMOP_ERROR) {
        return SUCCESS_CODE;
    }

    perror("perform_operations::semop(..) error");

    if (errno == EINTR) {
        return SEM_INTERRUPTED_CODE;
    }

    if (errno == EIDRM) {
        return SEM_REMOVED_CODE;
    }

    if (errno == EAGAIN) {
        return SEM_AGAIN_CODE;
    }

    return FAILURE_CODE;
}

int perform_operation(int semid, short semop, short semflg, unsigned short semIndex) {
    struct sembuf operation = {
        .sem_op = semop,
        .sem_flg = semflg,
        .sem_num = semIndex
    };

    size_t operationsNumber = 1;
    int returnCode = perform_operations(semid, &operation, operationsNumber);

    return returnCode;
}

int set_semaphore_value(int semid, unsigned short semIndex, int semval) {
    int returnCode = semctl(semid, semIndex, SETVAL, semval);
    if (returnCode == SEMCTL_ERROR) {
        perror("set_semaphore_value::semctl(..) error");
        return FAILURE_CODE;
    }
    return SUCCESS_CODE;
}

int remove_semaphores_set(int semid) {
    int returnCode = semctl(semid, PLACEHOLDER, IPC_RMID);
    if (returnCode == FAILURE_CODE) {
        perror("remove_semaphores_set::semctl(..) error");
        return FAILURE_CODE;
    }
    return SUCCESS_CODE;
}

void set_operation(struct sembuf *op, short semop, short semflg, unsigned short semIndex) {
    op->sem_op = semop;
    op->sem_flg = semflg;
    op->sem_num = semIndex;
}
