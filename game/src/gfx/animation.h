#ifndef ANIMATION_H
#define ANIMATION_H
#include "../util/types.h"
#include "spritesheet.h"
#define MAX_FRAMES 16

typedef struct {
    f32 duration;
    u8 row;
    u8 col;
} animation_frame_t;

typedef struct {
    spritesheet_t *spritesheet;
    animation_frame_t frames[MAX_FRAMES];
    u8 frame_count;
} animation_definition_t;

typedef struct {
    animation_definition_t *definition;
    f32 current_frame_time;
    u8 current_frame_index;
    b8 does_loop;
    b8 active;
    b8 flipped;
} animation_t;

void animation_init(void);
void animation_destroy(void);
void animation_update(f32 dt);
u32 animation_definition_create(spritesheet_t *spritesheet, f32 *durations, u8 *rows, u8 *col, u8 frame_count);
u32 animation_create(u32 animation_definition_id, b8 does_loop);
animation_t* animation_get(u32 animation_id);
#endif
