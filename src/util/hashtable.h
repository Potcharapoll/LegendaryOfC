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

  void(*free_value)(void*);
} hash_table_t;

hash_table_t* hashtable_init(size_t data_size, void(*free_value)(void*));
b8 hashtable_destroy(hash_table_t *self);
b8 hashtable_insert(hash_table_t *self, char *key, void* value);
b8 hashtable_delete(hash_table_t *self, char *key);

const entry_t* hashtable_search(hash_table_t *self, char *key);

#define HASHTABLE_INIT(value_type) \
  typedef struct entry_##value_type##_t { \
    char *key; \
    value_type *value; \
    struct entry_##value_type##_t *next; \
  } entry_##value_type##_t;\
  typedef struct hashtable_##value_type##_t { \
    size_t count; \
    entry_##value_type##_t **entries; \
    void(*free_value)(void*);\
  } hashtable_##value_type##_t;\
  \
  u64 hash_##value_type(char *key) {\
    u64 idx = 0;\
    for (u32 i = 0; i < strlen(key); i++) {\
      idx += key[i]; \
    }\
    return idx % HT_CAPACITY;\
  }\
  \
  hashtable_##value_type##_t* hashtable_##value_type##_init(void(*free_value)(void*)) {\
      hashtable_##value_type##_t *table = malloc(sizeof(*table));\
      table->count   = 0;\
      table->entries = calloc(HT_CAPACITY, sizeof(entry_##value_type##_t*));\
  \
      if (free_value) {\
        table->free_value = free_value;\
      }\
      else {\
        table->free_value = NULL;\
      }\
  \
      for (int i = 0; i < HT_CAPACITY; i++) {\
          table->entries[i] = NULL;\
      }\
      return table;\
  }\
  \
  b8 hashtable_##value_type##_insert(hashtable_##value_type##_t *self, char *key, value_type* value) {\
      assert(self != NULL);\
      assert(key != NULL);\
      assert(value != NULL);\
  \
      entry_##value_type##_t *new_pair = malloc(sizeof(*new_pair));\
      new_pair->value = value; \
      new_pair->key   = malloc(strlen(key)+1);\
      new_pair->next  = NULL;\
      strcpy(new_pair->key, key);\
      \
      u64 idx = hash_##value_type(key);\
      if (self->entries[idx] == NULL) {\
          self->entries[idx] = new_pair;\
      }\
      else {\
          new_pair->next = self->entries[idx];\
          self->entries[idx] = new_pair;\
      }\
  \
      self->count++;\
      return true;\
  }\
  \
  const entry_##value_type##_t* hashtable_##value_type##_search(hashtable_##value_type##_t *self, char *key) {\
      assert(self != NULL);\
      assert(key != NULL);\
  \
      u64 idx = hash_##value_type(key);\
      entry_##value_type##_t *item = self->entries[idx];\
  \
      while (item != NULL) {\
          if (strcmp(item->key, key) == 0) {\
              return item;\
          }\
          item = item->next;\
      }\
  \
      return NULL;\
  }\
  \
  b8 hashtable_##value_type##_delete(hashtable_##value_type##_t *self, char *key) {\
      assert(self != NULL);\
      assert(key != NULL);\
  \
      u64 idx = hash_##value_type(key);\
      entry_##value_type##_t *prev = NULL;\
      entry_##value_type##_t *item = self->entries[idx];\
  \
      b8 found = false;\
      while (item != NULL) {\
          if (strcmp(item->key, key) == 0) {\
              found = true;\
              break;\
          }\
          prev = item;\
          item = item->next;\
      }\
  \
      if (!found) { return found; }\
  \
      if (!prev && item->next) {\
              self->entries[idx] = item->next;\
      }\
      else if (prev && item->next) {\
          prev->next = item->next;\
      }\
      else {\
          self->entries[idx] = NULL;\
      }\
  \
      if (self->free_value) { \
        self->free_value(item->value); \
      } \
      else { \
        FREE(item->value);\
      } \
      FREE(item->key);\
      FREE(item);\
  \
      return found;\
  }\
  \
  b8 hashtable_##value_type##_destroy(hashtable_##value_type##_t *self) {\
      assert(self != NULL);\
  \
      for (int i = 0; i < HT_CAPACITY; i++) {\
           entry_##value_type##_t *curr = self->entries[i];\
           while (curr != NULL) {\
               entry_##value_type##_t *tmp = curr;\
               curr = curr->next;\
  \
               if (self->free_value) {\
                self->free_value(tmp->value);\
               }\
               else {\
                 FREE(tmp->value);\
               }\
               FREE(tmp->key);\
               FREE(tmp);\
           }\
      }\
      FREE(self->entries);\
      FREE(self);\
  \
      return true;\
  }\

#endif


