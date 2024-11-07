#ifndef DEFS_H
#define DEFS_H

#define FREE(x) free(x); x = NULL;

#define VERSION   0.90

#define WIDTH     1280
#define HEIGHT    768
#define TITLE     "LegendaryOfC"

#define TILE_SIZE      16
#define CHUNK_SIZE_Y   24
#define CHUNK_SIZE_X   30
#define CHUNK_DEF_SIZE (vec2s){24,30}

#define PROJECTION_WIDTH  (16*18) // 288
#define PROJECTION_HEIGHT (16*13) // 208

#define DEFAULT_SCALE (vec2s){16,16}
#define PLAYER_SIZE   (vec2s){16,22}
#define PLAYER_HITBOX (vec2s){16,8}

#define WHITE      (vec4s){1,1,1,1}
#define BLACK      (vec4s){0,0,0,1}
#define RED        (vec4s){1,0,0,1}
#define GREEN      (vec4s){0,1,0,1}
#define BLUE       (vec4s){0,0,1,1}
#define ORANGE     (vec4s){1,0.6,0.3,1}
#define CYAN       (vec4s){0,1,1,1}
#define YELLOW     (vec4s){1,1,0.4,1}
#define LIGHT_BLUE (vec4s){0,0.5,1,1}

#define SPAWN_COORD (vec2s){5,3}

#define TEXTURE_TEXT       "../res/images/font.png"
#define TEXTURE_LOGO       "../res/images/logo.png"
#define TEXTURE_PLAYER     "../res/images/characters/character.png"
#define TEXTURE_NPC        "../res/images/characters/npc.png"
#define TEXTURE_TILE       "../res/images/tilesets/tileset.png"
#define TEXTURE_INSIDE     "../res/images/tilesets/inside.png"
#define TEXTURE_STRUCTURES "../res/images/tilesets/structures.png"
#define TEXTURE_INTERACT   "../res/images/interact.png"

#define CHUNK_SPAWN_PATH             "../res/data/chunks/chunk_spawn"
#define CHUNK_VILLAGE_ENTRANCE_PATH  "../res/data/chunks/chunk_village_entrance"
#define CHUNK_VILLAGE_LEFT_PATH      "../res/data/chunks/chunk_village_left"
#define CHUNK_VILLAGE_RIGHT_PATH     "../res/data/chunks/chunk_village_right"
#define CHUNK_VILLAGE_TOP_PATH       "../res/data/chunks/chunk_village_top"
#define CHUNK_VILLAGE_TOP_END_PATH   "../res/data/chunks/chunk_village_top_end"
#define CHUNK_VILLAGE_TOP_LEFT_PATH  "../res/data/chunks/chunk_village_top_left"
#define CHUNK_VILLAGE_TOP_RIGHT_PATH "../res/data/chunks/chunk_village_top_right"
#define CHUNK_VILLAGE_TUNNEL_PATH    "../res/data/chunks/chunk_village_tunnel"
#define CHUNK_INSIDE_LIBRARY_PATH    "../res/data/chunks/chunk_inside_library"
#define CHUNK_INSIDE_RESTAURANT_PATH "../res/data/chunks/chunk_inside_restaurant"
#define CHUNK_INSIDE_CHURCH_PATH     "../res/data/chunks/chunk_inside_church"
#define CHUNK_INSIDE_FISH_PATH       "../res/data/chunks/chunk_inside_fish"
#define CHUNK_INSIDE_LJ_HOME_PATH    "../res/data/chunks/chunk_inside_lj_home"
#define CHUNK_INSIDE_OG_HOME_PATH    "../res/data/chunks/chunk_inside_og_home"
#define CHUNK_INSIDE_VC_HOME_PATH    "../res/data/chunks/chunk_inside_vc_home"

#define INPUT_DELAY 0.18f

#define TAG_TUNNEL     'a'
#define TAG_LIBRARY    'b'
#define TAG_RESTAURANT 'c'
#define TAG_FISH       'd'
#define TAG_CHURCH     'e'
#define TAG_LJ_HOME    'f'
#define TAG_OG_HOME    'g'
#define TAG_VC_HOME    'h'
#define TAG_NORMAL     't'
#endif
