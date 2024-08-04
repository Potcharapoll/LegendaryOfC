#include "asset_manager.h"

void asset_manager_init(AssetManager **self) {
    *self = malloc(sizeof(**self));
    (*self)->tilesets = hashtable_init(sizeof(tileset_t));
    (*self)->shaders  = hashtable_init(sizeof(Shader));
}

void asset_manager_destroy(AssetManager *self) {
    hashtable_destroy(self->shaders);
    hashtable_destroy(self->tilesets);
    free(self);
}

void asset_manager_push_tileset(AssetManager *self, char *name, u32 stride) {
    tileset_t tileset = tileset_load(name, stride);
    hashtable_insert(self->tilesets, name, &tileset); 
}

void asset_manager_push_shader(AssetManager *self, char *name, char *vs_path, char *fs_path) {
    Shader shader = shader_load(vs_path, fs_path);
    hashtable_insert(self->shaders, name, &shader);
}

array_list* asset_manager_get_all_tileset(AssetManager *self) {
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

array_list* asset_manager_get_all_shader(AssetManager *self) {
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

tileset_t* asset_manager_get_tileset(AssetManager *self, char *name) {
    if (name == NULL) return NULL;
    const entry_t* rel = hashtable_search(self->tilesets, name);
    return rel ? rel->value : NULL;
}

Shader* asset_manager_get_shader(AssetManager *self, char *name) {
    if (name == NULL) return NULL;
    const entry_t* rel = hashtable_search(self->shaders, name);
    return rel ? rel->value : NULL;
}
