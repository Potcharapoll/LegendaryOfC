#include "gfx/tileset.h"

tileset_t tileset_load(char *path, u32 stride) {
    tileset_t tileset; 
    tileset.texture    = texture_load(path);
    tileset.stride     = stride;
    tileset.rows       = tileset.texture.size.y / stride;
    tileset.cols       = tileset.texture.size.x / stride;
    tileset.tile_count = tileset.rows * tileset.cols;
    return tileset;
}
void tileset_destroy(tileset_t self) {
    texture_destroy(self.texture);
}
 
