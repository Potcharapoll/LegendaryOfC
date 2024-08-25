#include "physics.h"
#include "../util/array_list.h"
#include "../global.h"

#include <cglm/types-struct.h>
#include <stdio.h>

static array_list *_body_list = NULL;

void physics_init(void) {
    _body_list = array_list_init(sizeof(Body), 0);
}

void physics_destroy(void) {
    array_list_destroy(_body_list);
}

void physics_update(f32 dt) {
    static Body *_player_body;
    Body *body;

    // we will have id 0 for player and then check for collision to another body
    _player_body = array_list_get(_body_list, 0);

    {
        _player_body->aabb.center = (vec2s){
            .x = global.PlayerState.position.x + _player_body->aabb.half_size.x,
            .y = global.PlayerState.position.y + _player_body->aabb.half_size.y,
        };
    }

    for (u32 i = 1; i < _body_list->len; i++) {
        body = array_list_get(_body_list, i);

        if (aabb_intersect_aabb(_player_body->aabb, body->aabb)) {
            printf("Collision\n");
        }

    }
}

u64 physics_body_create(vec2s position, vec2s size) {
    Body body = {
        .aabb = { 
            .center    = (vec2s){position.x + size.x * 0.5, position.y + size.y * 0.5},
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

b8  aabb_intersect_aabb(AABB a, AABB b) {
    if ((a.center.x - a.half_size.x < b.center.x + b.half_size.x) && (a.center.x + a.half_size.x > b.center.x - b.half_size.x) 
     && (a.center.y - a.half_size.y < b.center.y + b.half_size.y) && (a.center.y + a.half_size.y > b.center.y - b.half_size.y)) 
        return true;
    return false;    
}
