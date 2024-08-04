#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H
#include "../util/array_list.h"
#include "../util/hashtable.h"
#include "../gfx/tileset.h"
#include "../gfx/shader.h"

typedef struct {
    hash_table_t *tilesets;
    hash_table_t *shaders;
}AssetManager;

void asset_manager_init(AssetManager **self);
void asset_manager_destroy(AssetManager *self);
void asset_manager_push_tileset(AssetManager *self, char *name, u32 stride);
void asset_manager_push_shader(AssetManager *self, char *name, char *vs_path, char *fs_path);
array_list* asset_manager_get_all_tileset(AssetManager *self);
array_list* asset_manager_get_all_shader(AssetManager *self);
tileset_t* asset_manager_get_tileset(AssetManager *self, char *name);
Shader* asset_manager_get_shader(AssetManager *self, char *name);
#endif
