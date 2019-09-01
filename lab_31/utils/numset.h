#ifndef NUMSET_H
#define NUMSET_H

#include <stddef.h>
#include "errorcodes.h"

#define NUMSET_CREATE_ERROR (NULL)

typedef unsigned long BufferElemType;
typedef unsigned long NumberType;

typedef struct NumSet {
    BufferElemType *buffer;
    size_t bufferSize;
    size_t valuesNumber;
} NumSet;

NumSet *numset_create(void);
void numset_destroy(NumSet *set);

int numset_insert(NumSet *set, NumberType val);
void numset_remove(NumSet *set, NumberType val);
void numset_clear(NumSet *set);
int numset_value_exists(NumSet *set, NumberType val);

void numset_find_first_missing(NumSet *set, NumberType startVal, NumberType *foundValue);
int numset_find_first_existing(NumSet *set, NumberType startVal, NumberType *foundValue);

#endif // NUMSET_H
