#include "physics.h"
#include "../util/array_list.h"
#include "cglm/types-struct.h"

static array_list *_body_list = NULL;

void physics_init(void) {
    _body_list = array_list_init(sizeof(Body), 0);
}

void physics_destroy(void) {
    array_list_destroy(_body_list);
}

void physics_update(f32 dt) {
    Body *body;

    for (u32 i = 0; i < _body_list->len; i++) {
        body = array_list_get(_body_list, i);

        body->velocity.x += body->accel.x * dt;
        body->velocity.y += body->accel.y * dt;
        body->aabb.position.x += body->velocity.x * dt;
        body->aabb.position.y += body->velocity.y * dt;
    }
}

u64 physics_body_create(vec2s position, vec2s size) {
    Body body = {
        .aabb = { 
            .position  = position,
            .half_size = {size.x * 0.5, size.y * 0.5}
        },
        .velocity = {0,0},
        .accel    = {0,0},
        .active   = true
    };
    
    array_list_append(_body_list, &body);
    return _body_list->len - 1;
}

void physics_body_destroy(u64 body_id) {
    Body *body   = array_list_get(_body_list, body_id);
    body->active = false;
}

Body* physics_body_get(u64 body_id) {
    return array_list_get(_body_list, body_id);
}
