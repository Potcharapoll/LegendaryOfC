#ifndef HASHTABLE_H
#define HASHTABLE_H
#define HT_CAPACITY 11
#include "types.h"

typedef struct entry_t {
    char *key;
    void *value;
    struct entry_t *next;
} entry_t;

typedef struct {
    size_t count;
    size_t data_size;
    entry_t **entries;
} hash_table_t;

hash_table_t* hashtable_init(size_t data_size);
bool hashtable_destroy(hash_table_t *self);
bool hashtable_insert(hash_table_t *self, char *key, void* value);
const entry_t* hashtable_search(hash_table_t *self, char *key);
bool hashtable_delete(hash_table_t *self, char *key);
#endif


