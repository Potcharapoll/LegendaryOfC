#ifndef ARRAY_LIST_H
#define ARRAY_LIST_H
#include "types.h"
#include <sys/types.h>

typedef struct {
    size_t len;
    size_t capacity;
    size_t data_size;
    void *data;
} array_list;

array_list* array_list_init(size_t data_size, size_t capacity);
void array_list_destroy(array_list *self);
void array_list_append(array_list *self, void *data);
void array_list_remove(array_list *self, size_t index);
void array_list_pop_first(array_list *self);
void array_list_pop_end(array_list *self);
#endif
