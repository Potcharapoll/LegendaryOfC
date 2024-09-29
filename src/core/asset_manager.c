#include "asset_manager.h"

#include "../engine/logger.h"

struct AssetManager* asset_manager_init(void) {
    struct AssetManager *am = malloc(sizeof(*am));
    ASSERT(am != NULL, "Failed to allocate memory for AssetManager", __FILE__, __LINE__);

    am->shaders      = hashtable_init(sizeof(struct Shader));
    am->spritesheets = hashtable_init(sizeof(struct Spritesheet));

    LOG_TRACE("AssetManager: Successfully initialized AssetManager");
    return am;
}

void asset_manager_destroy(struct AssetManager *self) {
    hashtable_destroy(self->shaders);
    hashtable_destroy(self->spritesheets);
    free(self);

    LOG_TRACE("AssetManager: Successfully destroyed AssetManager");
}

void asset_manager_push_texture(struct AssetManager *self, char *name, char *path) {
    struct Texture texture = texture_load(path);
    hashtable_insert(self->textures, name, &texture);

    LOG_DEBUG("AssetManager: Successfully pushed texture name \'%s\' from path \'%s\'", name, path);
}

void asset_manager_push_spritesheet(struct AssetManager *self, char *name, u32 count, u32 rows, u32 cols, u32 stride) {
    struct Spritesheet spritesheet = spritesheet_load(name, count, rows, cols, stride);
    hashtable_insert(self->spritesheets, name, &spritesheet);

    LOG_DEBUG("AssetManager: Successfully pushed spritesheet name \'%s\' from path \'%s\'", name, name);
}

void asset_manager_push_shader(struct AssetManager *self, char *name, char *vs_path, char *fs_path) {
    struct Shader shader = shader_load(vs_path, fs_path);
    hashtable_insert(self->shaders, name, &shader);

    LOG_DEBUG("AssetManager: Successfully pushed shader name \'%s\' from path \'{%s,%s}\'", name, vs_path, fs_path);
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
