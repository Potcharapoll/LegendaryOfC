#include "prefab.h"
#include "../util/hashtable.h"

hash_table_t *prefabs = NULL;

void prefab_init(void) {
    prefabs = hashtable_init(sizeof(Prefab));
}

void prefab_destroy(void){ 
    hashtable_destroy(prefabs);
}

void prefab_create(char *name, struct Spritesheet *spritesheet, vec4s color, vec2s size, vec4s grid_coord) {
    Prefab prefab = {
        .spritesheet = spritesheet,
        .size  = size,
        .color = color,
    };

    vec2s cell_size = {(f32)spritesheet->stride / spritesheet->texture.size.x, (f32)spritesheet->stride / spritesheet->texture.size.y };
    prefab.tex_coord[0] = cell_size.x * grid_coord.x;
    prefab.tex_coord[1] = cell_size.x * grid_coord.z;
    prefab.tex_coord[2] = cell_size.x * grid_coord.y;
    prefab.tex_coord[3] = cell_size.x * grid_coord.w;

    hashtable_insert(prefabs, name, &prefab);
}
