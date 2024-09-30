#include "spritesheet.h"
#include "texture.h"

Spritesheet spritesheet_load(char *path, u32 count, ivec2s grid_size, ivec2s cell_size) {
    Spritesheet spritesheet;

    spritesheet.texture = texture_load(path);
    spritesheet.count = count;
    spritesheet.size = (vec2s){spritesheet.texture.size.x, spritesheet.texture.size.y};
    spritesheet.cell_size = cell_size;
    spritesheet.grid_size = grid_size;
    return spritesheet;
}

void spritesheet_destroy(struct Spritesheet self) {
    texture_destroy(self.texture);
}
