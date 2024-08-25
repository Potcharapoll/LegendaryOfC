#include "spritesheet.h"
#include "texture.h"

struct Spritesheet spritesheet_load(char *path, u32 count, u32 rows, u32 cols, u32 stride) {
    struct Spritesheet spritesheet;

    spritesheet.texture = texture_load(path);
    spritesheet.stride  = stride;
    spritesheet.count   = count;
    spritesheet.cols    = cols;
    spritesheet.rows    = rows;
    spritesheet.size    = (vec2s){spritesheet.texture.size.x, spritesheet.texture.size.y};

    return spritesheet;
}

void spritesheet_destroy(struct Spritesheet self) {
    texture_destroy(self.texture);
}
