#include <stdlib.h>
#include <string.h>

#include "array_stack.h"

array_stack* array_stack_init(size_t data_size, size_t capacity) {
    array_stack *as = malloc(sizeof(*as));
    as->capacity  = capacity;
    as->data_size = data_size;
    as->len = 0;
    as->data = malloc(capacity * data_size);
    return as;
}

void array_stack_destroy(array_stack *self) {
    free(self->data);
    free(self);
}

u32 array_stack_push(array_stack *self ,void *data) {
    if (self->len == self->capacity) {
        self->capacity = (self->capacity != 0) ? self->capacity * 2 : 1;
        self->data = realloc(self->data, self->capacity * self->data_size);
    }

    u32 index = self->len++;
    memcpy(self->data + index * self->data_size, data, self->data_size);

    return index;
}

u32 array_stack_pop(array_stack *self) {
    u32 index = self->len--;
    return index;
}

