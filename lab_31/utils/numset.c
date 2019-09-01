#include <stdlib.h>
#include "numset.h"
#include "errorcodes.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define INITIAL_SIZE (16)
#define MALLOC_ERROR (NULL)
#define CALLOC_ERROR (NULL)
#define REALLOC_ERROR (NULL)

#define NOT_FOUND (SIZE_MAX)
#define BITS_PER_BUF_ELEM (sizeof (BufferElemType) * CHAR_BIT)
#define FULL_BIT_MASK  (~((BufferElemType)0))
#define EMPTY_BIT_MASK (0)

static size_t get_elem_index(NumberType val) {
    return val / BITS_PER_BUF_ELEM;
}

static size_t get_bit_index(NumberType val) {
    return val % BITS_PER_BUF_ELEM;
}

static NumberType get_value(size_t elemIndex, size_t bitIndex) {
    return BITS_PER_BUF_ELEM * elemIndex + bitIndex;
}

NumSet *numset_create(void) {
    NumSet *set = (NumSet*) malloc(sizeof (NumSet));
    if (set == MALLOC_ERROR) {
        perror("numset_create::malloc(..) error");
        return NUMSET_CREATE_ERROR;
    }

    set->buffer = NULL;
    set->valuesNumber = 0;
    set->bufferSize = 0;

    return set;
}

void numset_destroy(NumSet *set) {
    if (set == NULL) {
        return;
    }
    free(set->buffer);
    free(set);
}

static void clear_buffer(BufferElemType *buffer, size_t bufferSize) {
    static const int EMPTY_ELEM_VAL = 0;
    memset(buffer, EMPTY_ELEM_VAL, bufferSize * sizeof (BufferElemType));
}

static int numset_resize_buffer(NumSet *set, size_t newBufferSize) {
    BufferElemType *newBuffer = realloc(set->buffer, newBufferSize * sizeof (BufferElemType));
    if (newBuffer == REALLOC_ERROR) {
        perror("numset_resize::realloc(..) error");
        return FAILURE_CODE;
    }

    size_t oldBufferSize = set->bufferSize;
    if (newBufferSize > oldBufferSize) {
        clear_buffer(newBuffer + oldBufferSize, newBufferSize - oldBufferSize);
    }

    set->buffer = newBuffer;
    set->bufferSize = newBufferSize;

    return SUCCESS_CODE;
}


int numset_insert(NumSet *set, unsigned long val) {
    int valueExists = numset_value_exists(set, val);
    if (valueExists) {
        return SUCCESS_CODE;
    }

    size_t elemIndex = get_elem_index(val);
    size_t bitIndex = get_bit_index(val);

    if (elemIndex >= set->bufferSize) {
        int returnCode = numset_resize_buffer(set, elemIndex + 1);
        if (returnCode == FAILURE_CODE) {
            return FAILURE_CODE;
        }
    }

    set->buffer[elemIndex] |= (1ul << bitIndex);
    set->valuesNumber++;

    return SUCCESS_CODE;
}

int numset_value_exists(NumSet *set, unsigned long val) {
    size_t elemIndex = get_elem_index(val);
    size_t bitIndex = get_bit_index(val);
    if (elemIndex >= set->bufferSize) {
        return FALSE;
    }

    size_t bitIsSet = set->buffer[elemIndex] & (1ul << bitIndex);
    return bitIsSet ? TRUE : FALSE;
}

void numset_remove(NumSet *set, unsigned long val) {
    int valueExists = numset_value_exists(set, val);
    if (!valueExists) {
        return;
    }

    size_t elemIndex = get_elem_index(val);
    size_t bitIndex = get_bit_index(val);
    set->buffer[elemIndex] &= ~(1ul << bitIndex);
    set->valuesNumber--;
}

static size_t find_first_set_bit(BufferElemType bufElem, size_t startIndex) {
    BufferElemType bitMask = ((BufferElemType) 1) << startIndex;
    for (size_t bitIndex = startIndex; bitIndex < BITS_PER_BUF_ELEM; ++bitIndex) {
        if (bufElem & bitMask) {
            return bitIndex;
        }
        bitMask <<= 1ul;
    }
    return NOT_FOUND;
}

static size_t find_first_unset_bit(BufferElemType bufElem, size_t startIndex) {
    return find_first_set_bit(~bufElem, startIndex);
}

static size_t find_first_not_equal(NumSet *set, BufferElemType val, size_t startElemIndex) {
    for (size_t elemIndex = startElemIndex; elemIndex < set->bufferSize; ++elemIndex) {
        if (set->buffer[elemIndex] != val) {
            return elemIndex;
        }
    }
    return NOT_FOUND;
}

void numset_find_first_missing(NumSet *set, NumberType startVal, NumberType *foundValue) {
    size_t startElemIndex = get_elem_index(startVal);
    if (startElemIndex >= set->bufferSize) {
        *foundValue = startVal;
        return;
    }

    size_t startBitIndex = get_bit_index(startVal);
    size_t bitIndex = find_first_unset_bit(set->buffer[startElemIndex], startBitIndex);
    if (bitIndex != NOT_FOUND) {
        *foundValue = get_value(startElemIndex, bitIndex);
        return;
    }

    size_t elemIndex = find_first_not_equal(set, FULL_BIT_MASK, startElemIndex + 1);
    if (elemIndex == NOT_FOUND) {
        bitIndex = 0;
        elemIndex = set->bufferSize;
    } else {
        startBitIndex = 0;
        bitIndex = find_first_unset_bit(set->buffer[elemIndex], startBitIndex);
    }
    *foundValue = get_value(elemIndex, bitIndex);
}

int numset_find_first_existing(NumSet *set, NumberType startVal, NumberType *foundValue) {
    size_t startElemIndex = get_elem_index(startVal);
    if (startElemIndex >= set->bufferSize) {
        return FAILURE_CODE;
    }

    size_t startBitIndex = get_bit_index(startVal);
    size_t bitIndex = find_first_set_bit(set->buffer[startElemIndex], startBitIndex);
    if (bitIndex != NOT_FOUND) {
        *foundValue = get_value(startElemIndex, bitIndex);
        return SUCCESS_CODE;
    }

    size_t elemIndex = find_first_not_equal(set, EMPTY_BIT_MASK, startElemIndex + 1);
    if (elemIndex == NOT_FOUND) {
        return FAILURE_CODE;
    }

    startBitIndex = 0;
    bitIndex = find_first_set_bit(set->buffer[elemIndex], startBitIndex);
    *foundValue = get_value(elemIndex, bitIndex);

    return SUCCESS_CODE;
}

void numset_clear(NumSet *set) {
    clear_buffer(set->buffer, set->bufferSize);
    set->valuesNumber = 0;
}
