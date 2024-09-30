#include "prefab.h"
#include "../util/hashtable.h"
#include "../engine/logger.h"

static hash_table_t *prefabs = NULL;

void prefab_init(void) {
    prefabs = hashtable_init(sizeof(Prefab));

    LOG_TRACE("Prefab: Successfully initialized prefab");
}

void prefab_destroy(void){ 
    hashtable_destroy(prefabs);

    LOG_TRACE("Prefab: Successfully destroyed prefab");
}

void prefab_create(char *name, struct Spritesheet *spritesheet, vec4s color, vec2s size, vec4s grid_coord) {
    Prefab prefab = {
        .spritesheet = spritesheet,
        .size        = size,
        .color       = color,
    };

    vec2s cell_size = {
        (f32)spritesheet->cell_size.x / spritesheet->texture.size.x, 
        (f32)spritesheet->cell_size.y / spritesheet->texture.size.y 
    };

    prefab.tex_coord[0] = cell_size.x * grid_coord.x;
    prefab.tex_coord[1] = cell_size.x * grid_coord.z;

    prefab.tex_coord[2] = cell_size.y * grid_coord.y;
    prefab.tex_coord[3] = cell_size.y * grid_coord.w;

    hashtable_insert(prefabs, name, &prefab);

    LOG_DEBUG("Prefab: Create prefab named \'%s\'", name);
}

Prefab* prefab_get(char *name) {
    const entry_t *entry = hashtable_search(prefabs, name);
    return entry->value;
}
