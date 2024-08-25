#include "player.h"
#include "animation.h"
#include "../util/types.h"
#include "../global.h"
#include "../defs.h"
#include "physics.h"

#include <string.h>

static u32 adef_idle[DIRECTION_LAST];
static u32 adef_walk[DIRECTION_LAST];
static u32 animation_idle[DIRECTION_LAST];
static u32 animation_walk[DIRECTION_LAST];

void player_init(void) {

    // player animation
    struct Spritesheet *player_spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_PLAYER);

    adef_idle[LEFT]  = animation_definition_create(player_spritesheet, (f32[]){0},(u8[]){0},(u8[]){0},1);
    adef_idle[RIGHT] = animation_definition_create(player_spritesheet, (f32[]){0},(u8[]){1},(u8[]){0},1);
    adef_idle[UP]    = animation_definition_create(player_spritesheet, (f32[]){0},(u8[]){2},(u8[]){0},1);
    adef_idle[DOWN]  = animation_definition_create(player_spritesheet, (f32[]){0},(u8[]){3},(u8[]){0},1);

    adef_walk[LEFT]  = animation_definition_create(player_spritesheet, (f32[]){0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1}, (u8[]){0,0,0,0,0,0,0,0},(u8[]){0,1,2,3,4,5,6,7}, 8);
    adef_walk[RIGHT] = animation_definition_create(player_spritesheet, (f32[]){0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1}, (u8[]){1,1,1,1,1,1,1,1},(u8[]){0,1,2,3,4,5,6,7}, 8);
    adef_walk[UP]    = animation_definition_create(player_spritesheet, (f32[]){0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1}, (u8[]){2,2,2,2,2,2,2,2},(u8[]){0,1,2,3,4,5,6,7}, 8);
    adef_walk[DOWN]  = animation_definition_create(player_spritesheet, (f32[]){0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1}, (u8[]){3,3,3,3,3,3,3,3},(u8[]){1,1,2,3,4,5,6,7}, 8);

    animation_walk[LEFT]  = animation_create(adef_walk[LEFT], true);
    animation_walk[RIGHT] = animation_create(adef_walk[RIGHT], true);
    animation_walk[UP]    = animation_create(adef_walk[UP], true);
    animation_walk[DOWN]  = animation_create(adef_walk[DOWN], true);

    animation_idle[LEFT]  = animation_create(adef_idle[LEFT], false);
    animation_idle[RIGHT] = animation_create(adef_idle[RIGHT], false);
    animation_idle[UP]    = animation_create(adef_idle[UP], false);
    animation_idle[DOWN]  = animation_create(adef_idle[DOWN], false);

    global.PlayerState.position     = (vec3s){50,50,0};
    global.PlayerState.direction    = DOWN;

    global.PlayerState.animation_id = animation_idle[DOWN];
    global.PlayerState.body_id      = physics_body_create((vec2s){global.PlayerState.position.x, global.PlayerState.position.y}, PLAYER_SIZE);
}

void player_input(void) {
    s32 up    = glfwGetKey(global.window->handle, GLFW_KEY_W);
    s32 down  = glfwGetKey(global.window->handle, GLFW_KEY_S);
    s32 right = glfwGetKey(global.window->handle, GLFW_KEY_D);
    s32 left  = glfwGetKey(global.window->handle, GLFW_KEY_A);

    if (up == GLFW_PRESS) {
        global.PlayerState.position.y  += floor(200 * global.dt);
        global.PlayerState.animation_id = animation_walk[UP];
        global.PlayerState.direction    = UP;
    }
    else if (down == GLFW_PRESS) {
        global.PlayerState.position.y  -= floor(200 * global.dt);
        global.PlayerState.animation_id = animation_walk[DOWN];
        global.PlayerState.direction    = DOWN;
    }

    if (right == GLFW_PRESS) {
        global.PlayerState.position.x  += floor(200 * global.dt);
        global.PlayerState.animation_id = animation_walk[RIGHT];
        global.PlayerState.direction    = RIGHT;
    }
    else if (left == GLFW_PRESS) {
        global.PlayerState.position.x  -= floor(200 * global.dt);
        global.PlayerState.animation_id = animation_walk[LEFT];
        global.PlayerState.direction    = LEFT;
    }

    if (!up && !down && !right && !left) {
        global.PlayerState.animation_id = animation_idle[global.PlayerState.direction];
    }
}

void player_get_tex_coord(f32 *tex_coord) {
    animation_t *animation = animation_get(global.PlayerState.animation_id);

    u32 frame = animation->current_frame_index;
    u32 row   = animation->definition->frames[frame].row;
    u32 col   = animation->definition->frames[frame].col;
    f32 cellX = animation->definition->spritesheet->stride / animation->definition->spritesheet->size.x; 
    f32 cellY = 32 / animation->definition->spritesheet->size.y; 
    printf("%i, %i, %i\n", frame, row, col);

    f32 *_tex_coord = (f32[]){cellX * col, cellX * col + cellX, cellY * row, cellY * row + cellY};
    memcpy(tex_coord, _tex_coord, sizeof(f32[4]));
}
