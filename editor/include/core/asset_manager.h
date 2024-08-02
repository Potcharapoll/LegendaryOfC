#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H
#include "../util/array_list.h"
#include "../util/hashtable.h"
#include "../gfx/tileset.h"
#include "../gfx/shader.h"

typedef struct {
    hash_table_t *tilesets;
    hash_table_t *shaders;
} asset_manager_t;

asset_manager_t* asset_manager_init(void);
void asset_manager_destroy(asset_manager_t *self);
void asset_manager_push_tileset(asset_manager_t *self, char *name, u32 stride);
void asset_manager_push_shader(asset_manager_t *self, char *name, char *vs_path, char *fs_path);
array_list* asset_manager_get_all_tileset(asset_manager_t *self);
array_list* asset_manager_get_all_shader(asset_manager_t *self);
tileset_t* asset_manager_get_tileset(asset_manager_t *self, char *name);
shader_t* asset_manager_get_shader(asset_manager_t *self, char *name);
#endif
