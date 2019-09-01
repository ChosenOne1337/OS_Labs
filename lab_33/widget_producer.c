#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <libgen.h>
#include <stdarg.h>
#include <signal.h>
#include "parse_long.h"
#include "semaphores_set.h"
#include "semaphore.h"
#include "errorcodes.h"

typedef void (*sighandler_t)(int);

void sigint_handler(int sig) {
    exit(EXIT_SUCCESS);
}

int produce_widgets(int moduleSemID, int detailCSemID) {
    sighandler_t prevHandler = sigset(SIGINT, sigint_handler);
    if (prevHandler == SIG_ERR) {
        perror("produce_widgets::sigset(..) error");
        return FAILURE_CODE;
    }

    short semflg = 0;
    unsigned short semIndex = 0;
    unsigned long widgetNo = 1;

    for (;;) {
        short takeDetailOp = -1;
        int returnCode = perform_operation(detailCSemID, takeDetailOp, semflg, semIndex);
        if (returnCode != SUCCESS_CODE) {
            return FAILURE_CODE;
        }

        short takeModuleOp = -1;
        returnCode = perform_operation(moduleSemID, takeModuleOp, semflg, semIndex);
        if (returnCode != SUCCESS_CODE) {
            return FAILURE_CODE;
        }

        printf("Produced a widget No%ld\n", widgetNo++);
    }

    return FAILURE_CODE;
}

int main(int argc, char *argv[]) {
    int moduleSemID = get_semaphores_set(MODULE_FTOK_ID);
    if (moduleSemID == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    int detailCSemID = get_semaphores_set(DETAIL_C_FTOK_ID);
    if (detailCSemID == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    produce_widgets(moduleSemID, detailCSemID);

    return EXIT_SUCCESS;
}
