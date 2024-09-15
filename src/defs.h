#ifndef DEFS_H
#define DEFS_H
#define WIDTH     1280
#define HEIGHT    768
#define TITLE     "LegendaryOfC"

#define TILE_SIZE      32
#define CHUNK_SIZE_Y   24
#define CHUNK_SIZE_X   30
#define CHUNK_DEF_SIZE (vec2s){24,30}

#define PROJECTION_WIDTH  (32*16)
#define PROJECTION_HEIGHT (32*16)

#define DEFAULT_SCALE (vec2s){32,32}
#define PLAYER_SIZE   (vec2s){24,64}
#define PLAYER_HITBOX (vec2s){24,64-24} // 32 - 12 | 16 - 6

#define WHITE  (vec4s){1,1,1,1}
#define BLACK  (vec4s){0,0,0,1}
#define RED    (vec4s){1,0,0,1}
#define GREEN  (vec4s){0,1,0,1}
#define BLUE   (vec4s){0,0,1,1}
#define ORANGE (vec4s){1,0.6,0.3,1}
#define CYAN   (vec4s){0,1,1,1}

#define TEXTURE_PLAYER  "res/characters/fix1.png"
#define TEXTURE_NPC     "res/characters/fix2.png"
#define TEXTURE_TESTING "res/tilesets/test.png"
#define TEXTURE_BASIC   "res/tilesets/chunk1_test.png"
#define TEXTURE_TEXT    "res/images/font.png"
#endif
