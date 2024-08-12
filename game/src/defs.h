#ifndef DEFS_H
#define DEFS_H
#define WIDTH     1280
#define HEIGHT    768
#define TITLE     "LegendaryOfC"
#define TILE_SIZE (16*2)

#define PROJECTION_WIDTH  1280
#define PROJECTION_HEIGHT  768

#define DEFAULT_SCALE (vec2s){16*2,16*2}
#define HUMAN_SCALE   (vec2s){16*2,32*2}

#define WHITE (vec4s){1,1,1,1}
#define BLACK (vec4s){0,0,0,1}
#define RED   (vec4s){1,0,0,1}
#define GREEN (vec4s){0,1,0,1}
#define BLUE  (vec4s){0,0,1,1}

#define TEXTURE_PLAYER "res/characters/fix1.png"
#define TEXTURE_NPC    "res/characters/fix2.png"
#define TEXTURE_NPC2   "res/characters/hero-edit.png"
#define TEXTURE_CHUNK  "res/tilesets/chunk1_test.png"
#define TEXTURE_CHUNK2 "res/tilesets/tilemap.png"

#define FONT1 "res/fonts/UniversCondensed.ttf"
#define FONT2 "res/fonts/JetBrainsMonoNerdFont-Medium.ttf"
#endif
