#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H
#include "../util/hashtable.h"
#include "../gfx/spritesheet.h"
#include "../gfx/shader.h"

struct AssetManager{
    hash_table_t *spritesheets;
    hash_table_t *textures;
    hash_table_t *shaders;
};

void asset_manager_init(struct AssetManager **self);
void asset_manager_destroy(struct AssetManager *self);

void asset_manager_push_texture(struct AssetManager *self, char *name, char *path);
void asset_manager_push_spritesheet(struct AssetManager *self, char *name, u32 count, u32 rows, u32 cols, u32 stride);
void asset_manager_push_shader(struct AssetManager *self, char *name, char *vs_path, char *fs_path);

struct Spritesheet* asset_manager_get_spritesheet(struct AssetManager *self, char *name);
struct Shader* asset_manager_get_shader(struct AssetManager *self, char *name);
struct Texture* asset_manager_get_texture(struct AssetManager *self, char *name);
#endif
