#ifndef LIST_H
#define LIST_H

#include <stddef.h>
#include "errorcodes.h"

typedef struct ListNode {
    long value;
    struct ListNode *next;
} ListNode;

typedef struct List {
    ListNode *head;
    ListNode *tail;
    size_t length;
} List;

#define LIST_CREATE_ERROR (NULL)
#define LIST_NO_ELEMENT (NULL)

List* list_create(void);
void list_destroy(List *list);

int list_append(List *list, long value);
int list_prepend(List *list, long value);

int list_remove_element(List *list, long value);
ListNode* list_find_element(List *list, long value);

#endif // LIST_H
