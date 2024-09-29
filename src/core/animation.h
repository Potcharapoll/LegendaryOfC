#ifndef ANIMATION_H
#define ANIMATION_H
#include "../util/types.h"
#include "../util/array_list.h"
#include "../gfx/spritesheet.h"
#define MAX_FRAMES 16

typedef struct {
    f32 duration;
    u8 row;
    u8 col;
} AnimationFrame;

typedef struct {
    struct Spritesheet *spritesheet;
    AnimationFrame frames[MAX_FRAMES];
    u8 frame_count;
} AnimationDefinition;

typedef struct {
    AnimationDefinition *definition;
    f32 current_frame_time;
    u8 current_frame_index;

    b8 flipped;
    b8 does_loop;
} AnimationState;

typedef struct Animation {
    array_list *animation_state_storage;
    array_list *animation_definition_storage;
} Animation;

Animation* animation_init(void);
void animation_destroy(Animation *animation);
void animation_update(Animation *animation, f32 dt);

u32 animation_definition_create(Animation *animation, struct Spritesheet *spritesheet, f32 *durations, u8 *rows, u8 *col, u8 frame_count);
AnimationDefinition* animation_definition_get(Animation *animation, u32 animation_definition_id);

u32 animation_state_create(Animation *animation, u32 animation_definition_id, b8 does_loop, b8 flipped);
AnimationState* animation_state_get(Animation *animation, u32 animation_state_id);
#endif
