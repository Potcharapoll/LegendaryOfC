#include "core/asset_manager.h"

asset_manager_t* asset_manager_init(void) {
    asset_manager_t *am = malloc(sizeof(*am));
    am->tilesets = hashtable_init(sizeof(tileset_t));
    am->shaders  = hashtable_init(sizeof(shader_t));
    return am;
}

void asset_manager_destroy(asset_manager_t *self) {
    hashtable_destroy(self->shaders);
    hashtable_destroy(self->tilesets);
    free(self);
}

void asset_manager_push_tileset(asset_manager_t *self, char *name, u32 stride) {
    tileset_t tileset = tileset_load(name, stride);
    hashtable_insert(self->tilesets, name, &tileset); 
}

void asset_manager_push_shader(asset_manager_t *self, char *name, char *vs_path, char *fs_path) {
    shader_t shader = shader_load(vs_path, fs_path);
    hashtable_insert(self->shaders, name, &shader);
}

array_list* asset_manager_get_all_tileset(asset_manager_t *self) {
    if (self->tilesets->count <= 0) return NULL;

    array_list *list = array_list_init(sizeof(entry_t), HT_CAPACITY);
    for (int idx = 0; idx < HT_CAPACITY; idx++) {
        entry_t *items = self->tilesets->entries[idx];
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

tileset_t* asset_manager_get_tileset(asset_manager_t *self, char *name) {
    if (name == NULL) return NULL;
    const entry_t* rel = hashtable_search(self->tilesets, name);
    return rel ? rel->value : NULL;
}

shader_t* asset_manager_get_shader(asset_manager_t *self, char *name) {
    if (name == NULL) return NULL;
    const entry_t* rel = hashtable_search(self->shaders, name);
    return rel ? rel->value : NULL;
}
