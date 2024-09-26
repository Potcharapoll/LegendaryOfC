#include "animation.h"

#include "../util/array_list.h"
#include "../engine/logger.h"

static array_list *_animation_storage;
static array_list *_animation_definition_storage;

void animation_init(void) {
    _animation_storage            = array_list_init(sizeof(animation_t), 0);
    _animation_definition_storage = array_list_init(sizeof(animation_definition_t), 0);

    LOG_TRACE("Animation: Successfully initialized animation");
}

void animation_destroy(void) {
    array_list_destroy(_animation_storage);
    array_list_destroy(_animation_definition_storage);

    LOG_TRACE("Animation: Successfully destroyed animation");
}

void animation_update(f32 dt) {
    for (u32 i = 0; i < _animation_storage->len; ++i) {
        animation_t *animation = array_list_get(_animation_storage, i);
        animation_definition_t *adef = animation->definition;
        animation->current_frame_time -= dt;

        if (animation->current_frame_time <= 0) {
            animation->current_frame_index += 1;

            // Loop or stay on last frame
            if (animation->current_frame_index == animation->definition->frame_count) {
                if (animation->does_loop) {
                    animation->current_frame_index = 0;
                }
                else {
                    animation->current_frame_index -= 1;
                }
            }

            animation->current_frame_time = adef->frames[animation->current_frame_index].duration;
        }
    }
}

u32 animation_definition_create(struct Spritesheet *spritesheet, f32 *durations, u8 *rows, u8 *cols, u8 frame_count) {
    if (frame_count > MAX_FRAMES) {
        LOG_ERROR("Animation: frame_count cannot be exceed the max_frames");
    }

    animation_definition_t def = {0};

    def.spritesheet = spritesheet;
    def.frame_count = (frame_count > MAX_FRAMES) ? MAX_FRAMES : frame_count;

    for (u8 i = 0; i < frame_count; i++) {
        def.frames[i] = (animation_frame_t) {
            .col = cols[i],
            .row = rows[i],
            .duration = durations[i]
        };
    }

    return array_list_append(_animation_definition_storage, &def);
}

u32 animation_create(u32 animation_definition_id, b8 does_loop, b8 flipped) {
    animation_definition_t *adef = array_list_get(_animation_definition_storage, animation_definition_id);
    /* ASSERT_MSG(adef != NULL, "Failed to get animation definition of id %u", animation_definition_id); */

    u32 id = array_list_append(_animation_storage, &(animation_t){0});

    animation_t *animation = array_list_get(_animation_storage, id);

    *animation = (animation_t){
        .definition = adef,
        .does_loop  = does_loop,
        .flipped    = flipped,
        .active     = true,
    };

    return id;
}

animation_t* animation_get(u32 animation_id) {
    return array_list_get(_animation_storage, animation_id);
}
