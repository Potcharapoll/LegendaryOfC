#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H
#include "array_list.h"
#include "hashtable.h"
#include "texture.h"
#include "shader.h"

typedef struct {
    hash_table_t *textures;
    hash_table_t *shaders;
} asset_manager_t;

asset_manager_t* asset_manager_init(void);
void asset_manager_destroy(asset_manager_t *self);
void asset_manager_push_texture(asset_manager_t *self, char *name);
void asset_manager_push_shader(asset_manager_t *self, char *name, char *vs_path, char *fs_path);
array_list* asset_manager_get_all_texture(asset_manager_t *self);
array_list* asset_manager_get_all_shader(asset_manager_t *self);
struct Texture* asset_manager_get_texture(asset_manager_t *self, char *name);
struct Shader* asset_manager_get_shader(asset_manager_t *self, char *name);
#endif
