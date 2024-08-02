#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include "util/hashtable.h"
#include "util/log.h"

u64 hash(char *key) {
    u64 idx = 0;
    for (u32 i = 0; i < strlen(key); i++) {
        idx += key[i]; 
    }
    return idx % HT_CAPACITY;
}

hash_table_t* hashtable_init(size_t data_size) {
    hash_table_t *table = malloc(sizeof(*table));
    table->count        = 0;
    table->data_size    = data_size;
    table->entries      = calloc(HT_CAPACITY, sizeof(entry_t*));

    for (int i = 0; i < HT_CAPACITY; i++) {
        table->entries[i] = NULL;
    }
    return table;
}

bool hashtable_insert(hash_table_t *self, char *key, void* value) {
    assert(self != NULL);
    assert(key != NULL);
    assert(value != NULL);

    entry_t *new_pair = malloc(sizeof(*new_pair));
    new_pair->value   = malloc(self->data_size);
    new_pair->key     = malloc(strlen(key)+1);
    new_pair->next    = NULL;
    memcpy(new_pair->value, value, self->data_size);
    strcpy(new_pair->key, key);

    uint64_t idx = hash(key);
    if (self->entries[idx] == NULL) {
        self->entries[idx] = new_pair;
    }
    else {
        new_pair->next = self->entries[idx];
        self->entries[idx] = new_pair;
    }

    self->count++;
    return true;
}

const entry_t* hashtable_search(hash_table_t *self, char *key) {
    assert(self != NULL);
    assert(key != NULL);

    uint64_t idx = hash(key);
    entry_t *item = self->entries[idx];

    while (item != NULL) {
        if (strcmp(item->key, key) == 0) {
            return item;
        }
        item = item->next;
    }

    return NULL;
}

bool hashtable_delete(hash_table_t *self, char *key) {
    assert(self != NULL);
    assert(key != NULL);

    uint64_t idx = hash(key);
    entry_t *prev = NULL;
    entry_t *item = self->entries[idx];

    bool found = false;
    while (item != NULL) {
        if (strcmp(item->key, key) == 0) {
            found = true;
            break;
        }
        prev = item;
        item = item->next;
    }

    if (!found) {
        LOG_DEBUG("Not found key named \'%s\'", key);
        return found;
    }

    if (!prev && item->next) {
            self->entries[idx] = item->next;
    }
    else if (prev && item->next) {
        prev->next = item->next;
    }
    else {
        self->entries[idx] = NULL;
    }

    free(item->value);
    free(item->key);
    free(item);

    return found;
}

bool hashtable_destroy(hash_table_t *self) {
    assert(self != NULL);

    for (int i = 0; i < HT_CAPACITY; i++) {
         entry_t *curr = self->entries[i];
         while (curr != NULL) {
             entry_t *tmp = curr;
             curr = curr->next;

             free(tmp->value);
             free(tmp->key);
             free(tmp);
         }
    }
    free(self->entries);
    free(self);

    return true;
}
