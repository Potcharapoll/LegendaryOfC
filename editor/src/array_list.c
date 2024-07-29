#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <strings.h>
#include "array_list.h"
#include "log.h"

array_list* array_list_init(size_t data_size, size_t capacity) {
    array_list *al = malloc(sizeof(*al));
    assert(al != NULL);

    al->len       = 0;
    al->capacity  = capacity;
    al->data_size = data_size;
    al->data      = malloc(capacity * data_size);
    assert(al->data != NULL);
    return al;
}

void array_list_destroy(array_list *self) {
    assert(self != NULL);
    assert(self->data != NULL);

    free(self->data);
    free(self);
}

void array_list_append(array_list *self, void *data) {
    assert(self != NULL);
    assert(self->data != NULL);
    assert(data != NULL);

    if (self->len == self->capacity) {
        self->capacity = (self->capacity != 0) ? self->capacity * 2 : 1;
        void *new_data = realloc(self->data, self->capacity * self->data_size);

        if (!new_data) { 
            LOG_FETAL("Cannot allocate memory for array list\n"); 
            abort();
        }
        self->data = new_data;
    }

    memcpy((u8*)self->data + self->len * self->data_size, data, self->data_size);
    self->len++;
}

void *array_list_get(array_list *self, size_t index) {
    assert(self != NULL);
    assert(self->data != NULL);

    return (u8*)self->data + index * self->data_size;
}

void array_list_remove(array_list *self, size_t index) {
    assert(self != NULL);
    assert(self->data != NULL);

    if (self->len == 0) return;
    if (index >= self->len) return;
    if (self->len == 1) {
        self->len--;
        return;
    }

	memcpy((u8*)self->data + index * self->data_size, self->data + self->len * self->data_size, self->data_size);
}

void array_list_pop_first(array_list *self) {
    assert(self != NULL);
    assert(self->data != NULL);

    memcpy((u8*)self->data, self->data + 1 * self->data_size, (self->len -1) * self->data_size);
    self->len--;
}

void array_list_pop_end(array_list *self) {
    assert(self != NULL);
    assert(self->data != NULL);

    self->len--;
}
