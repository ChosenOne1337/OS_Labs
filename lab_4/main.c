#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "constants.h"

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


#define LINE_BUF_SIZE 256

int expand_buffer(char **buf, int *bufSize);
void read_file(FILE *inFile, List *lineList, char **lineBuf, int bufSize);

void fill_list(List *list, FILE *inFile) {
    // create a buffer
    char *lineBuf = (char*)calloc(LINE_BUF_SIZE, sizeof(char));
    if (lineBuf == NULL) {
        perror("fill_list(..) error while allocating memory for a buffer");
        return;
    }
    // fill the list
    read_file(inFile, list, &lineBuf, LINE_BUF_SIZE);
    if (ferror(inFile)) {
        fprintf(stderr, "fill_list(..) error while reading from the file\n");
    }
    // free resources
    free(lineBuf);
}

int expand_buffer(char **buf, int *bufSize) {
    static const int sizeMultiplier = 2;
    int newBufSize = (*bufSize) * sizeMultiplier;
    char *newBuf = (char*)realloc(*buf, (size_t)newBufSize);
    if (newBuf == NULL) {
        perror("expand_buffer(..) error");
        return FAILURE_CODE;
    }
    *buf = newBuf;
    *bufSize = newBufSize;

    return SUCCESS_CODE;
}

void read_file(FILE *inFile, List *lineList, char **lineBuf, int bufSize) {
    static const char END_COMMAND[] = ".";
    static const char NEW_LINE_CHAR = '\n';
    static const char STRING_END_CHAR = '\0';

    int bufPos = 0;
    int errCode = 0;
    char *buf = *lineBuf;
    while (fgets(&buf[bufPos], bufSize - bufPos, inFile) != NULL) {
        // check buffer overflow
        bufPos += strlen(&buf[bufPos]);
        if (buf[bufPos - 1] != NEW_LINE_CHAR) {
            errCode = expand_buffer(&buf, &bufSize);
            if (errCode == FAILURE_CODE) {
                break;
            }
            continue;
        }
        // truncate the new line character
        buf[bufPos - 1] = STRING_END_CHAR;
        // check if the read line is end command
        if (STRCMP(buf, ==, END_COMMAND)) {
            break;
        }
        // append the line to the list
        errCode = list_append(lineList, buf);
        if (errCode == FAILURE_CODE) {
            break;
        }
        bufPos = 0;
    }
    *lineBuf = buf;
}

void print_list(List *list, FILE *outFile) {
    for (ListNode *node = list->head; node != NULL; node = node->next) {
        fprintf(outFile, "%s\n", node->str);
    }
}
