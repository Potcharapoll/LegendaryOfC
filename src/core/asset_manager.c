#include "../util/log.h"
#include "asset_manager.h"

void asset_manager_init(struct AssetManager **self) {
    *self = malloc(sizeof(**self));
    ASSERT_MSG(self != NULL, "Failed to allocate memory for asset manager");

    (*self)->shaders      = hashtable_init(sizeof(struct Shader));
    (*self)->spritesheets = hashtable_init(sizeof(struct Spritesheet));
}

void asset_manager_destroy(struct AssetManager *self) {
    hashtable_destroy(self->shaders);
    hashtable_destroy(self->spritesheets);
    free(self);
}

void asset_manager_push_texture(struct AssetManager *self, char *name, char *path) {
    struct Texture texture = texture_load(path);
    hashtable_insert(self->textures, name, &texture);
}

void asset_manager_push_spritesheet(struct AssetManager *self, char *name, u32 count, u32 rows, u32 cols, u32 stride) {
    struct Spritesheet spritesheet = spritesheet_load(name, count, rows, cols, stride);
    hashtable_insert(self->spritesheets, name, &spritesheet);
}

void asset_manager_push_shader(struct AssetManager *self, char *name, char *vs_path, char *fs_path) {
    struct Shader shader = shader_load(vs_path, fs_path);
    hashtable_insert(self->shaders, name, &shader);
}

struct Spritesheet* asset_manager_get_spritesheet(struct AssetManager *self, char *name) {
    if (name == NULL) return NULL;
    const entry_t* rel = hashtable_search(self->spritesheets, name);
    return rel ? rel->value : NULL;
}

struct Shader* asset_manager_get_shader(struct AssetManager *self, char *name) {
    if (name == NULL) return NULL;
    const entry_t* rel = hashtable_search(self->shaders, name);
    return rel ? rel->value : NULL;
}

struct Texture* asset_manager_get_texture(struct AssetManager *self, char *name) {
    if (name == NULL) return NULL;
    const entry_t* rel = hashtable_search(self->textures, name);
    return rel ? rel->value : NULL;
}
