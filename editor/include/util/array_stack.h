#ifndef ARRAY_STACK_H
#define ARRAY_STACK_H

#include "types.h"

typedef struct {
    size_t capacity;
    size_t len;
    size_t data_size;
    void  *data;
} array_stack;

array_stack* array_stack_init(size_t data_size, size_t capacity);
void array_stack_destroy(array_stack *self);
u32 array_stack_push(array_stack *self ,void *data);
u32 array_stack_pop(array_stack *self);
#endif
