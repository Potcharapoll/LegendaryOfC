#ifndef PLAYER_H
#define PLAYER_H
#include "animation.h"
#include "physics.h"

enum Direction { 
    UP, 
    DOWN, 
    LEFT, 
    RIGHT, 

    DIRECTION_LAST 
};

enum PlayerAnimation {
    IDLE,
    WALK
};

void player_init(void);
void player_render(void);
void player_input(void);

void player_get_tex_coord(f32 *tex_coodr);
Body *player_get_body(void);
AnimationState *player_get_animation(void);
enum Direction player_get_direction(void);

void player_set_animation(enum PlayerAnimation animation, enum Direction direction);
void player_set_direction(enum Direction direction);
#endif
