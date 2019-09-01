#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <libgen.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include "semaphores_set.h"
#include "semaphore.h"
#include "errorcodes.h"

#define SEMID_NOT_INITIALIZED (-1)
static int MODULE_SEMAPHORE_ID = SEMID_NOT_INITIALIZED;

typedef void (*sighandler_t)(int);

void sigint_handler(int sig) {
    if (MODULE_SEMAPHORE_ID == SEMID_NOT_INITIALIZED) {
        exit(EXIT_SUCCESS);
    }

    int returnCode = remove_semaphores_set(MODULE_SEMAPHORE_ID);
    if (returnCode == FAILURE_CODE) {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

int produce_modules(int moduleSemID, int detailAsemID, int detailBsemID) {
    sighandler_t prevHandler = sigset(SIGINT, sigint_handler);
    if (prevHandler == SIG_ERR) {
        perror("produce_modules::sigset(..) error");
        return FAILURE_CODE;
    }

    short semflg = 0;
    unsigned short semIndex = 0;
    short makeModuleOp = 1, takeDetailOp = -1;
    unsigned moduleNo = 1;
    for (;;) {
        int returnCode = perform_operation(detailAsemID, takeDetailOp, semflg, semIndex);
        if (returnCode != SUCCESS_CODE) {
            return FAILURE_CODE;
        }

        returnCode = perform_operation(detailBsemID, takeDetailOp, semflg, semIndex);
        if (returnCode != SUCCESS_CODE) {
            return FAILURE_CODE;
        }

        returnCode = perform_operation(moduleSemID, makeModuleOp, semflg, semIndex);
        if (returnCode != SUCCESS_CODE) {
            return FAILURE_CODE;
        }

        printf("Produced module No%u\n", moduleNo++);
    }

    return FAILURE_CODE;
}

int main(int argc, char *argv[]) {
    int detailAsemID = get_semaphores_set(DETAIL_A_FTOK_ID);
    if (detailAsemID == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    int detailBsemID = get_semaphores_set(DETAIL_B_FTOK_ID);
    if (detailBsemID == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    int semaphoresNumber = 1;
    MODULE_SEMAPHORE_ID = create_semaphores_set(semaphoresNumber, MODULE_FTOK_ID);
    if (MODULE_SEMAPHORE_ID == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    int returnCode = produce_modules(MODULE_SEMAPHORE_ID, detailAsemID, detailBsemID);
    if (returnCode == FAILURE_CODE) {
        remove_semaphores_set(MODULE_SEMAPHORE_ID);
        return EXIT_FAILURE;
    }

    returnCode = remove_semaphores_set(MODULE_SEMAPHORE_ID);
    if (returnCode == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
