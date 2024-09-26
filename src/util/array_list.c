#include "array_list.h"
#include "../engine/logger.h"

#include <string.h>
#include <stdlib.h>

array_list* array_list_init(size_t data_size, size_t capacity) {
    array_list *al = malloc(sizeof(*al));
    ASSERT(al != NULL, "Failed to allocate memory for array_list", __FILE__, __LINE__);

    al->len       = 0;
    al->capacity  = capacity;
    al->data_size = data_size;
    al->data      = malloc(capacity * data_size);
    ASSERT(al->data != NULL, "Failed to allocate memory for array_list entry", __FILE__, __LINE__);
    return al;
}

void array_list_destroy(array_list *self) {
    free(self->data);
    free(self);
}

u32 array_list_append(array_list *self, void *data) {
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

    return self->len - 1;
}

void *array_list_get(array_list *self, size_t index) {
    return (u8*)self->data + index * self->data_size;
}

void array_list_remove(array_list *self, size_t index) {
    if (self->len == 0) return;
    if (index >= self->len) return;
    if (self->len == 1) {
        self->len--;
        return;
    }

	memcpy((u8*)self->data + index * self->data_size, self->data + self->len * self->data_size, self->data_size);
}

void array_list_pop_first(array_list *self) {
    memcpy((u8*)self->data, self->data + 1 * self->data_size, (self->len -1) * self->data_size);
    self->len--;
}

void array_list_pop_end(array_list *self) {
    self->len--;
}
