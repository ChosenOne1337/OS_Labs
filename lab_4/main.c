#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

void fill_list(List *list, FILE *inFile);
void print_list(List *list, FILE *outFile);

int main(void) {
    List *strList = list_create();
    if (strList == NULL) {
        return EXIT_FAILURE;
    }

    fill_list(strList, stdin);
    print_list(strList, stdout);

    list_destroy(strList);

    return EXIT_SUCCESS;
}

#define STRING_BUF_LEN 64

void trunc_new_line(char *str);

void fill_list(List *list, FILE *inFile) {
    static char line[STRING_BUF_LEN] = {0};
    static const char END_CHAR_SEQUENCE[] = ".";

    while (fgets(line, sizeof(line), inFile) != NULL) {
        trunc_new_line(line);
        if (strcmp(END_CHAR_SEQUENCE, line) == 0) {
            break;
        }

        int errCode = list_append(list, line);
        if (errCode != 0) {
            break;
        }
    }

    if (ferror(inFile)) {
        fprintf(stderr, "Error while reading from the file\n");
    }
}

void print_list(List *list, FILE *outFile) {
    if (list == NULL) {
        return;
    }

    for (ListNode *node = list->head; node != NULL; node = node->next) {
        fprintf(outFile, "%s\n", node->str);
    }
}

void trunc_new_line(char *str) {
    if (str == NULL) {
        return;
    }

    unsigned long strLength = strlen(str);
    if (str[strLength - 1] == '\n') {
        str[strLength - 1] = 0;
    }
}
