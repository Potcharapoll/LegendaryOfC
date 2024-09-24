#ifndef PLAYER_H
#define PLAYER_H
#include "../util/types.h"

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
void player_set_animation(enum PlayerAnimation animation, enum Direction direction);
void player_get_tex_coord(f32 *tex_coodr);
#endif
