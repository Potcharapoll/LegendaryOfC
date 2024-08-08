#include "../util/log.h"
#include "asset_manager.h"

void asset_manager_init(AssetManager **self) {
    *self = malloc(sizeof(**self));
    ASSERT_MSG(self != NULL, "Failed to allocate memory for asset manager");

    (*self)->shaders      = hashtable_init(sizeof(Shader));
    (*self)->spritesheets = hashtable_init(sizeof(spritesheet_t));
}

void asset_manager_destroy(AssetManager *self) {
    hashtable_destroy(self->shaders);
    hashtable_destroy(self->spritesheets);
    free(self);
}

void asset_manager_push_spritesheet(AssetManager *self, char *name, u32 count, u32 rows, u32 cols, u32 stride) {
    spritesheet_t spritesheet = spritesheet_load(name, count, rows, cols, stride);
    hashtable_insert(self->spritesheets, name, &spritesheet);
}

void asset_manager_push_shader(AssetManager *self, char *name, char *vs_path, char *fs_path) {
    Shader shader = shader_load(vs_path, fs_path);
    hashtable_insert(self->shaders, name, &shader);
}

spritesheet_t* asset_manager_get_spritesheet(AssetManager *self, char *name) {
    if (name == NULL) return NULL;
    const entry_t* rel = hashtable_search(self->spritesheets, name);
    return rel ? rel->value : NULL;
}

Shader* asset_manager_get_shader(AssetManager *self, char *name) {
    if (name == NULL) return NULL;
    const entry_t* rel = hashtable_search(self->shaders, name);
    return rel ? rel->value : NULL;
}
