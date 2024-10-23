#include "animation.h"

#include "../util/array_list.h"
#include "../engine/logger.h"

Animation* animation_init(void) {
    Animation *animation = malloc(sizeof(*animation));
    animation->animation_state_storage      = array_list_init(sizeof(AnimationState), 0);
    animation->animation_definition_storage = array_list_init(sizeof(AnimationDefinition), 0);

    LOG_TRACE("Animation: Successfully initialized animation");
    return animation;
}

void animation_destroy(Animation *animation) {
    array_list_destroy(animation->animation_state_storage);
    array_list_destroy(animation->animation_definition_storage);
    free(animation);

    LOG_TRACE("Animation: Successfully destroyed animation");
}

void animation_update(Animation *animation, f32 dt) {
    for (u32 i = 0; i < animation->animation_state_storage->len; ++i) {
        AnimationState *astate = array_list_get(animation->animation_state_storage, i);
        AnimationDefinition *adef = astate->definition;
        astate->current_frame_time -= dt;

        if (astate->current_frame_time <= 0) {
            astate->current_frame_index += 1;

            if (astate->current_frame_index == astate->definition->frame_count) {
                if (astate->does_loop) {
                    astate->current_frame_index = 0;
                }
                else {
                    astate->current_frame_index -= 1;
                }
            }

            astate->current_frame_time = adef->frames[astate->current_frame_index].duration;
        }
    }
}

u32 animation_definition_create(Animation *animation, struct Spritesheet *spritesheet, f32 *durations, u8 *rows, u8 *cols, u8 frame_count) {
    if (frame_count > MAX_FRAMES) {
        LOG_ERROR("Animation: frame_count cannot be exceed the max_frames");
        return -1;
    }

    AnimationDefinition adef = {0};

    adef.spritesheet = spritesheet;
    adef.frame_count = (frame_count > MAX_FRAMES) ? MAX_FRAMES : frame_count;

    for (u8 i = 0; i < frame_count; i++) {
        adef.frames[i] = (AnimationFrame) {
            .col = cols[i],
            .row = rows[i],
            .duration = durations[i]
        };
    }

    array_list_append(animation->animation_definition_storage, &adef);
    LOG_DEBUG("Animation: Create new animation definition");
    return animation->animation_definition_storage->len - 1;
}

AnimationDefinition* animation_definition_get(Animation *animation, u32 animation_definition_id) {
    return array_list_get(animation->animation_definition_storage, animation_definition_id);
}

u32 animation_state_create(Animation *animation, u32 animation_definition_id, b8 does_loop, b8 flipped) {
    AnimationDefinition *adef = array_list_get(animation->animation_definition_storage, animation_definition_id);

    if (adef == NULL) {
        LOG_ERROR("Animation: Failed to get animation definition from id %u", animation_definition_id);
        return -1;
    }

    AnimationState astate = (AnimationState){
        .definition = adef,
        .does_loop  = does_loop,
        .flipped    = flipped,
        .current_frame_time = 0.0f,
        .current_frame_index = 0
    };

    array_list_append(animation->animation_state_storage, &astate);
    LOG_DEBUG("Animation: Create new animation state");
    return animation->animation_state_storage->len - 1;
}

AnimationState* animation_state_get(Animation *animation, u32 animation_state_id) {
    return array_list_get(animation->animation_state_storage, animation_state_id);
}
