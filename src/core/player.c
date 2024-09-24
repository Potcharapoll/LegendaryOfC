#include "../util/types.h"
#include "../global.h"
#include "../defs.h"

#include "player.h"
#include "animation.h"
#include "physics.h"

#include <string.h>

static u32 adef_idle[DIRECTION_LAST];
static u32 adef_walk[DIRECTION_LAST];
static u32 animation_idle[DIRECTION_LAST];
static u32 animation_walk[DIRECTION_LAST];

static f32 SPEED = 100.0f;
static Body *player_body;

void player_init(void) {
    struct Spritesheet *player_spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_PLAYER);

    adef_idle[DOWN]  = animation_definition_create(player_spritesheet, (f32[]){0},(u8[]){3},(u8[]){0},1);
    adef_idle[UP]    = animation_definition_create(player_spritesheet, (f32[]){0},(u8[]){2},(u8[]){0},1);
    adef_idle[RIGHT] = animation_definition_create(player_spritesheet, (f32[]){0},(u8[]){1},(u8[]){0},1);
    adef_idle[LEFT]  = animation_definition_create(player_spritesheet, (f32[]){0},(u8[]){0},(u8[]){0},1);

    adef_walk[DOWN]  = animation_definition_create(player_spritesheet, (f32[]){0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1}, (u8[]){3,3,3,3,3,3,3,3},(u8[]){0,1,2,3,4,5,6,7}, 8);
    adef_walk[UP]    = animation_definition_create(player_spritesheet, (f32[]){0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1}, (u8[]){2,2,2,2,2,2,2,2},(u8[]){0,1,2,3,4,5,6,7}, 8);
    adef_walk[RIGHT] = animation_definition_create(player_spritesheet, (f32[]){0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1}, (u8[]){1,1,1,1,1,1,1,1},(u8[]){0,1,2,3,4,5,6,7}, 8);
    adef_walk[LEFT]  = animation_definition_create(player_spritesheet, (f32[]){0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1}, (u8[]){0,0,0,0,0,0,0,0},(u8[]){0,1,2,3,4,5,6,7}, 8);

    animation_walk[DOWN]  = animation_create(adef_walk[DOWN], true, false);
    animation_walk[UP]    = animation_create(adef_walk[UP], true, false);
    animation_walk[RIGHT] = animation_create(adef_walk[RIGHT], true, false);
    animation_walk[LEFT]  = animation_create(adef_walk[LEFT], true, false);

    animation_idle[DOWN]  = animation_create(adef_idle[DOWN], false, false);
    animation_idle[UP]    = animation_create(adef_idle[UP], false, false);
    animation_idle[RIGHT] = animation_create(adef_idle[RIGHT], false, false);
    animation_idle[LEFT]  = animation_create(adef_idle[LEFT], false, false);

    global.PlayerState.direction    = DOWN;
    global.PlayerState.animation_id = animation_idle[DOWN];
    global.PlayerState.body_id      = physics_body_create(
            (vec2s){0,0}, PLAYER_HITBOX, COLLISION_LAYER_SOLID | COLLISION_LAYER_TELEPORTER, COLLISION_LAYER_PLAYER);
}

void player_input(void) {
    player_body = physics_body_get(global.PlayerState.body_id);

    s32 up    = window_get_key(global.window, GLFW_KEY_W);
    s32 down  = window_get_key(global.window, GLFW_KEY_S);
    s32 right = window_get_key(global.window, GLFW_KEY_D);
    s32 left  = window_get_key(global.window, GLFW_KEY_A);

    if (up) {
        player_body->velocity.y = SPEED;
        player_set_animation(WALK, UP);
    }
    else if (down) {
        player_body->velocity.y = -SPEED;
        player_set_animation(WALK, DOWN);
    }

    if (!up && !down) {
        player_body->velocity.y = 0;
    }

    if (right) {
        player_body->velocity.x = SPEED;
        player_set_animation(WALK, RIGHT);
    }
    else if (left) {
        player_body->velocity.x = -SPEED;
        player_set_animation(WALK, LEFT);
    }

    if (!right && !left) {
        player_body->velocity.x = 0;
    }

    if (!up && !down && !right && !left) {
        player_set_animation(IDLE, global.PlayerState.direction);
    }
}

void player_get_tex_coord(f32 *tex_coord) {
    animation_t *animation = animation_get(global.PlayerState.animation_id);

    u32 frame = animation->current_frame_index;
    u32 row   = animation->definition->frames[frame].row;
    u32 col   = animation->definition->frames[frame].col;
    f32 cellX = animation->definition->spritesheet->stride / animation->definition->spritesheet->size.x; 
    f32 cellY = 32 / animation->definition->spritesheet->size.y; 

    f32 *_tex_coord = (f32[]){cellX * col, cellX * col + cellX, cellY * row, cellY * row + cellY};
    memcpy(tex_coord, _tex_coord, sizeof(f32[4]));
}

void player_set_animation(enum PlayerAnimation animation, enum Direction direction) {
    switch (animation) {
        case IDLE:
            global.PlayerState.animation_id = animation_idle[direction];
            global.PlayerState.direction    = direction;
            break;
        case WALK:
            global.PlayerState.animation_id = animation_walk[direction];
            global.PlayerState.direction    = direction;
            break;
    }
}
