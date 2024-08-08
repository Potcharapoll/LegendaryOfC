#include "spritesheet.h"
#include "texture.h"

spritesheet_t spritesheet_load(char *path, u32 count, u32 rows, u32 cols, u32 stride) {
    spritesheet_t spritesheet;

    spritesheet.texture = texture_load(path);
    spritesheet.stride  = stride;
    spritesheet.count   = count;
    spritesheet.cols    = cols;
    spritesheet.rows    = rows;
    spritesheet.size    = (vec2s){spritesheet.texture.size.x, spritesheet.texture.size.y};

    return spritesheet;
}

void spritesheet_destroy(spritesheet_t self) {
    texture_destroy(self.texture);
}
