#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <libgen.h>
#include <stdarg.h>
#include <string.h>
#include "semaphores_set.h"
#include "semaphore.h"
#include "errorcodes.h"
#include "signal.h"

#define STRCMP(a, R, b) (strcmp(a, b) R 0)

typedef enum DetailType {
    DetailA,
    DetailB,
    DetailC,
    UnknownDetail
} DetailType;

enum Argv {
    ProgPathIndex,
    DetailTypeIndex,
    RequiredArgc
};

#define SEMID_NOT_INITIALIZED (-1)
static int DETAIL_SEMAPHORE_ID = SEMID_NOT_INITIALIZED;

typedef void (*sighandler_t)(int);

void sigint_handler(int sig) {
    if (DETAIL_SEMAPHORE_ID == SEMID_NOT_INITIALIZED) {
        exit(EXIT_SUCCESS);
    }

    int returnCode = remove_semaphores_set(DETAIL_SEMAPHORE_ID);
    if (returnCode == FAILURE_CODE) {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

int produce_details(int detailSemID, unsigned delay, char detailName) {
    sighandler_t prevHandler = sigset(SIGINT, sigint_handler);
    if (prevHandler == SIG_ERR) {
        perror("produce_details::sigset(..) error");
        return FAILURE_CODE;
    }

    short semflg = 0;
    unsigned short semIndex = 0;
    short makeDetailOp = 1;
    unsigned detailNo = 1;

    for (;;) {
        sleep(delay);

        int returnCode = perform_operation(detailSemID, makeDetailOp, semflg, semIndex);
        if (returnCode != SUCCESS_CODE) {
            return FAILURE_CODE;
        }

        printf("Produced detail \"%c\" No%u\n", detailName, detailNo++);
    }

    return FAILURE_CODE;
}

int get_detail_type(char *detailType) {
    if (STRCMP("a", ==, detailType)) {
        return DetailA;
    }
    if (STRCMP("b", ==, detailType)) {
        return DetailB;
    }
    if (STRCMP("c", ==, detailType)) {
        return DetailC;
    }
    fprintf(stderr, "Error: invalid detail type:\n"
           "expected \"a\", \"b\", or \"c\", got \"%s\"\n", detailType);
    return UnknownDetail;
}

int create_detail_semaphore(int detailType) {
    int ftokID;
    switch (detailType) {
    case DetailA:
        ftokID = DETAIL_A_FTOK_ID;
        break;
    case DetailB:
        ftokID = DETAIL_B_FTOK_ID;
        break;
    case DetailC:
        ftokID = DETAIL_C_FTOK_ID;
        break;
    default:
        return FAILURE_CODE;
    }

    int semaphoresNumber = 1;
    int detailSemID = create_semaphores_set(semaphoresNumber, ftokID);
    if (detailSemID == FAILURE_CODE) {
        return FAILURE_CODE;
    }

    return detailSemID;
}

void print_usage(char *progPath) {
    fprintf(stderr, "Usage: %s <detail type (a, b, or c)>\n", basename(progPath));
}

int main(int argc, char *argv[]) {
    if (argc != RequiredArgc) {
        print_usage(argv[ProgPathIndex]);
        return EXIT_FAILURE;
    }

    const static char detailNames[] = {'A', 'B', 'C'};
    const static unsigned delays[] = {2, 3, 4};
    int detailType = get_detail_type(argv[DetailTypeIndex]);
    if (detailType == UnknownDetail) {
        return EXIT_FAILURE;
    }

    DETAIL_SEMAPHORE_ID = create_detail_semaphore(detailType);
    if (DETAIL_SEMAPHORE_ID == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    char detailName = detailNames[detailType];
    unsigned delay = delays[detailType];
    int returnCode = produce_details(DETAIL_SEMAPHORE_ID, delay, detailName);
    if (returnCode == FAILURE_CODE) {
        remove_semaphores_set(DETAIL_SEMAPHORE_ID);
        return EXIT_FAILURE;
    }

    returnCode = remove_semaphores_set(DETAIL_SEMAPHORE_ID);
    if (returnCode == FAILURE_CODE) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
