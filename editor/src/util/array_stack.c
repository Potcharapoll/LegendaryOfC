#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "util/array_stack.h"

array_stack* array_stack_init(size_t data_size, size_t capacity) {
    array_stack *as = malloc(sizeof(*as));
    assert(as != NULL);

    as->capacity    = capacity;
    as->data_size   = data_size;
    as->len         = 0;
    as->data        = malloc(capacity * data_size);
    assert(as->data != NULL);
    return as;
}

void array_stack_destroy(array_stack *self) {
    assert(self != NULL);    
    assert(self->data != NULL);

    free(self->data);
    free(self);
}

u32 array_stack_push(array_stack *self ,void *data) {
    assert(self != NULL);    
    assert(self->data != NULL);
    assert(data != NULL);

    if (self->len == self->capacity) {
        self->capacity = (self->capacity != 0) ? self->capacity * 2 : 1;
        self->data = realloc(self->data, self->capacity * self->data_size);
    }

    u32 index = self->len++;
    memcpy((u8*)self->data + index * self->data_size, data, self->data_size);

    return index;
}

u32 array_stack_pop(array_stack *self) {
    assert(self != NULL);    
    assert(self->data != NULL);

    u32 index = self->len--;
    return index;
}

