#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H
#include "../util/hashtable.h"
#include "../gfx/spritesheet.h"
#include "../gfx/shader.h"

typedef struct {
    hash_table_t *spritesheets;
    hash_table_t *shaders;
}AssetManager;

void asset_manager_init(AssetManager **self);
void asset_manager_destroy(AssetManager *self);
void asset_manager_push_spritesheet(AssetManager *self, char *name, u32 count, u32 rows, u32 cols, u32 stride);
void asset_manager_push_tileset(AssetManager *self, char *name, u32 stride);
void asset_manager_push_shader(AssetManager *self, char *name, char *vs_path, char *fs_path);

spritesheet_t* asset_manager_get_spritesheet(AssetManager *self, char *name);
Shader* asset_manager_get_shader(AssetManager *self, char *name);
#endif
