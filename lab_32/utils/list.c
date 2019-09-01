#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

#define MALLOC_ERROR (NULL)

List* list_create(void) {
    List *list = (List*) malloc(sizeof(List));
    if (list == MALLOC_ERROR) {
        perror("malloc error");
        return NULL;
    }

    list->head = list->tail = NULL;
    list->length = 0;

    return list;
}

static ListNode* list_node_create(long value) {
    ListNode *node = (ListNode*) malloc(sizeof(ListNode));
    if (node == MALLOC_ERROR) {
        perror("malloc error");
        return NULL;
    }

    node->value = value;
    node->next = NULL;

    return node;
}


void list_destroy(List *list) {
    ListNode *currNode = list->head;
    while (currNode != NULL) {
        list->head = currNode->next;
        free(currNode);
        currNode = list->head;
    }

    free(list);
}

int list_append(List *list, long value) {
    ListNode *node = list_node_create(value);
    if (node == NULL) {
        return FAILURE_CODE;
    }

    if (list->length == 0) {
        list->head = node;
    } else {
        list->tail->next = node;
    }
    list->tail = node;
    list->length++;

    return SUCCESS_CODE;
}

int list_prepend(List *list, long value) {
    ListNode *node = list_node_create(value);
    if (node == NULL) {
        return FAILURE_CODE;
    }

    if (list->length == 0) {
        list->tail = node;
    }
    node->next = list->head;
    list->head = node;
    list->length++;

    return SUCCESS_CODE;
}

static void list_remove_node(List *list, ListNode *node, ListNode *prevNode) {
    if (node == list->tail) {
        list->tail = prevNode;
    }

    if (node == list->head) {
        list->head = node->next;
        free(node);
        return;
    }

    prevNode->next = node->next;
    free(node);
}

ListNode* list_find_element(List *list, long value) {
    ListNode *currNode = list->head;
    while (currNode != NULL) {
        if (currNode->value == value) {
            return currNode;
        }
        currNode = currNode->next;
    }
    return LIST_NO_ELEMENT;
}

int list_remove_element(List *list, long value) {
    ListNode *prevNode = NULL;
    ListNode *currNode = list->head;
    while (currNode != NULL) {
        if (currNode->value == value) {
            list_remove_node(list, currNode, prevNode);
            list->length--;
            return SUCCESS_CODE;
        }
        prevNode = currNode;
        currNode = currNode->next;
    }

    return FAILURE_CODE;
}
