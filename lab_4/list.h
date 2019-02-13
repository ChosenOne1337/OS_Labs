#ifndef LIST_H
#define LIST_H

#include <stddef.h>

typedef struct ListNode {
    char *str;
    struct ListNode *next;
} ListNode;

typedef struct List {
    ListNode *head;
    ListNode *tail;
    size_t length;
} List;

List* list_create(void);

void list_destroy(List *list);

int list_append(List *list, char *str);

int list_prepend(List *list, char *str);

#endif // LIST_H
