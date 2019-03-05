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
    char *newBuf = (char*)realloc(*buf, (size_t) (*bufSize) * 2);
    if (newBuf == NULL) {
        perror("expand_buffer(..) error");
        return 1;
    }
    *buf = newBuf;
    *bufSize *= 2;

    return 0;
}

void read_file(FILE *inFile, List *lineList, char **lineBuf, int bufSize) {
    static const char END_COMMAND[] = ".";

    int bufPos = 0;
    int errCode = 0;
    char *buf = *lineBuf;
    while (fgets(&buf[bufPos], bufSize - bufPos, inFile) != NULL) {
        // check buffer overflow
        bufPos += strlen(&buf[bufPos]);
        if (buf[bufPos - 1] != '\n') {
            errCode = expand_buffer(&buf, &bufSize);
            if (errCode) {
                break;
            }
            continue;
        }
        // truncate the new line character
        buf[bufPos - 1] = '\0';
        // check if the read line is end command
        if (strcmp(buf, END_COMMAND) == 0) {
            break;
        }
        // append the line to the list
        errCode = list_append(lineList, buf);
        if (errCode != 0) {
            break;
        }
        bufPos = 0;
    }
    *lineBuf = buf;
}

void print_list(List *list, FILE *outFile) {
    int i = 1;
    for (ListNode *node = list->head; node != NULL; node = node->next) {
        fprintf(outFile, "%d) %s\n", i++, node->str);
    }
}
