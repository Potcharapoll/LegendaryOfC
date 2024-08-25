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

void player_init(void);
void player_render(void);
void player_input(void);
void player_get_tex_coord(f32 *tex_coodr);
#endif
