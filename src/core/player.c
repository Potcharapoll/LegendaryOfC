#include "player.h"
#include "animation.h"
#include "physics.h"

#include "../util/types.h"
#include "../engine/logger.h"
#include "../global.h"
#include "../defs.h"

static u32 adef_idle[DIRECTION_LAST];
static u32 adef_walk[DIRECTION_LAST];
static u32 astate_idle[DIRECTION_LAST];
static u32 astate_walk[DIRECTION_LAST];

static f32 SPEED = 100.0f;
static Body *player_body;

void player_init(void) {
    struct Spritesheet *player_spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_PLAYER);

    adef_idle[DOWN]  = animation_definition_create(global.animations, player_spritesheet, (f32[]){0},(u8[]){2},(u8[]){0},1);
    adef_idle[UP]    = animation_definition_create(global.animations, player_spritesheet, (f32[]){0},(u8[]){1},(u8[]){0},1);
    adef_idle[RIGHT] = animation_definition_create(global.animations, player_spritesheet, (f32[]){0},(u8[]){0},(u8[]){0},1);
    adef_idle[LEFT]  = animation_definition_create(global.animations, player_spritesheet, (f32[]){0},(u8[]){0},(u8[]){0},1);
    adef_walk[DOWN]  = animation_definition_create(global.animations, player_spritesheet, (f32[]){0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1}, (u8[]){2,2,2,2,2,2,2,2},(u8[]){0,1,2,3,4,5,6,7}, 8);
    adef_walk[UP]    = animation_definition_create(global.animations, player_spritesheet, (f32[]){0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1}, (u8[]){1,1,1,1,1,1,1,1},(u8[]){0,1,2,3,4,5,6,7}, 8);
    adef_walk[RIGHT] = animation_definition_create(global.animations, player_spritesheet, (f32[]){0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1}, (u8[]){0,0,0,0,0,0,0,0},(u8[]){0,1,2,3,4,5,6,7}, 8);
    adef_walk[LEFT]  = animation_definition_create(global.animations, player_spritesheet, (f32[]){0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1}, (u8[]){0,0,0,0,0,0,0,0},(u8[]){0,1,2,3,4,5,6,7}, 8);

    astate_walk[DOWN]  = animation_state_create(global.animations, adef_walk[DOWN], true, false);
    astate_walk[UP]    = animation_state_create(global.animations, adef_walk[UP], true, false);
    astate_walk[RIGHT] = animation_state_create(global.animations, adef_walk[RIGHT], true, false);
    astate_walk[LEFT]  = animation_state_create(global.animations, adef_walk[LEFT], true, true);
    astate_idle[DOWN]  = animation_state_create(global.animations, adef_idle[DOWN], false, false);
    astate_idle[UP]    = animation_state_create(global.animations, adef_idle[UP], false, false);
    astate_idle[RIGHT] = animation_state_create(global.animations, adef_idle[RIGHT], false, false);
    astate_idle[LEFT]  = animation_state_create(global.animations, adef_idle[LEFT], false, true);

    global.PlayerState.direction    = DOWN;
    global.PlayerState.animation_id = astate_idle[DOWN];
    global.PlayerState.body_id      = physics_body_create(global.physics, (vec2s){0,0}, PLAYER_HITBOX, COLLISION_LAYER_SOLID | COLLISION_LAYER_TELEPORTER, COLLISION_LAYER_PLAYER);

    LOG_TRACE("Player: Successfully initialized player");
}

void player_input(void) {
    player_body = physics_body_get(global.physics, global.PlayerState.body_id);

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
    AnimationState* astate = animation_state_get(global.animations, global.PlayerState.animation_id);

    u32 frame = astate->current_frame_index;
    u32 row   = astate->definition->frames[frame].row;
    u32 col   = astate->definition->frames[frame].col;
    f32 cellX = astate->definition->spritesheet->cell_size.x / astate->definition->spritesheet->size.x; 
    f32 cellY = astate->definition->spritesheet->cell_size.y / astate->definition->spritesheet->size.y; 

    f32 *_tex_coord = (f32[]){cellX * col, cellX * col + cellX, cellY * row, cellY * row + cellY};

    if (astate->flipped) {
        tex_coord[0] = _tex_coord[1];
        tex_coord[1] = _tex_coord[0];
    }
    else {
        tex_coord[0] = _tex_coord[0];
        tex_coord[1] = _tex_coord[1];
    }
    tex_coord[2] = _tex_coord[2];
    tex_coord[3] = _tex_coord[3];
}

void player_set_animation(enum PlayerAnimation animation, enum Direction direction) {
    switch (animation) {
        case IDLE:
            global.PlayerState.animation_id = astate_idle[direction];
            global.PlayerState.direction    = direction;
            break;
        case WALK:
            global.PlayerState.animation_id = astate_walk[direction];
            global.PlayerState.direction    = direction;
            break;
    }
}
