#include <stdio.h>
#include "asset_manager.h"
#include "array_list.h"
#include "hashtable.h"

asset_manager_t* asset_manager_init(void) {
    asset_manager_t *am = malloc(sizeof(*am));
    am->textures = hashtable_init(sizeof(struct Texture));
    am->shaders  = hashtable_init(sizeof(struct Shader));
    return am;
}

void asset_manager_destroy(asset_manager_t *self) {
    hashtable_destroy(self->shaders);
    hashtable_destroy(self->textures);
    free(self);
}

void asset_manager_push_texture(asset_manager_t *self, char *name) {
    struct Texture texture = texture_load(name);
    hashtable_insert(self->textures, name, &texture); 
}

void asset_manager_push_shader(asset_manager_t *self, char *name, char *vs_path, char *fs_path) {
    struct Shader shader = shader_load(vs_path, fs_path);
    hashtable_insert(self->shaders, name, &shader);
}

array_list* asset_manager_get_all_texture(asset_manager_t *self) {
    if (self->textures->count <= 0) return NULL;

    array_list *list = array_list_init(sizeof(entry_t), HT_CAPACITY);
    for (int idx = 0; idx < HT_CAPACITY; idx++) {
        entry_t *items = self->textures->entries[idx];
        while(items != NULL) {
            array_list_append(list, items);
            items = items->next;

        }
    }
    return list;
}

array_list* asset_manager_get_all_shader(asset_manager_t *self) {
    if (self->shaders->count <= 0) return NULL;

    array_list *list = array_list_init(sizeof(entry_t), HT_CAPACITY);
    for (int idx = 0; idx < HT_CAPACITY; idx++) {
        entry_t *items = self->shaders->entries[idx];
        while(items != NULL) {
            array_list_append(list, items);
            items = items->next;

        }
    }
    return list;
}

struct Texture* asset_manager_get_texture(asset_manager_t *self, char *name) {
    if (name == NULL) return NULL;
    const entry_t* rel = hashtable_search(self->textures, name);
    return rel ? rel->value : NULL;
}

struct Shader* asset_manager_get_shader(asset_manager_t *self, char *name) {
    if (name == NULL) return NULL;
    const entry_t* rel = hashtable_search(self->shaders, name);
    return rel ? rel->value : NULL;
}
