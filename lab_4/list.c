#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

List* list_create(void) {
    List *list = (List*)malloc(sizeof(List));
    if (list == NULL) {
        perror("Failed to create a list");
        return NULL;
    }

    list->head = list->tail = NULL;
    list->length = 0;

    return list;
}

ListNode* list_node_create(char *str) {
    ListNode *node = (ListNode*)malloc(sizeof(ListNode));
    if (node == NULL) {
        perror("Failed to create a list node");
        return NULL;
    }

    unsigned long strLength = strlen(str);
    node->str = (char*)calloc(strLength + 1, sizeof(char));
    if (node->str == NULL) {
        perror("Failed to copy the given string");
        free(node);
        return NULL;
    }
    strcpy(node->str, str);
    node->next = NULL;

    return node;
}

void list_node_destroy(ListNode *node) {
    if (node == NULL) {
        return;
    }

    free(node->str);
    free(node);
}

void list_destroy(List *list) {
    if (list == NULL) {
        return;
    }

    ListNode *currNode = list->head;
    while (currNode != NULL) {
        list->head = currNode->next;
        list_node_destroy(currNode);
        currNode = list->head;
    }

    free(list);
}

int list_append(List *list, char *str) {
    if (list == NULL) {
        return FAILURE_CODE;
    }

    ListNode *node = list_node_create(str);
    if (node == NULL) {
        return FAILURE_CODE;
    }

    if (list->length == 0) {
        list->head = list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
    list->length++;

    return SUCCESS_CODE;
}

int list_prepend(List *list, char *str) {
    if (list == NULL) {
        return FAILURE_CODE;
    }

    ListNode *node = list_node_create(str);
    if (node == NULL) {
        return FAILURE_CODE;
    }

    if (list->length == 0) {
        list->head = list->tail = node;
    } else {
        node->next = list->head;
        list->head = node;
    }
    list->length++;

    return SUCCESS_CODE;
}
