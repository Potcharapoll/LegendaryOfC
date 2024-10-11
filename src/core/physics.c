#include "physics.h"

#include "../util/array_list.h"
#include "../engine/logger.h"
#include "../global.h"

static void collision_response(Body *body, Static_Body *static_body, AABB minkowski) {
#ifdef DEBUG
    if (global.toggle_collision) return;
#endif

    if ((static_body->collision_mask & body->collision_flag) != body->collision_flag) return;

    if (static_body->collision_flag & COLLISION_LAYER_SOLID) {
        vec2s pv;
        aabb_penetration_vector(&pv, minkowski);
        body->position.x += pv.x;
        body->position.y += pv.y;
    }

    if (static_body->on_hit_by_body) { static_body->on_hit_by_body(static_body, body); }
}

static void collision_check(Physics *self, Body *body) {
    Static_Body *static_body;

    for (u32 i = 0; i < self->static_body_list->len; i++) {
        static_body = physics_static_body_get(self, i);

        if (static_body->collision_mask == COLLISION_LAYER_NONE) continue;

        if (aabb_intersect_aabb(static_body->aabb, body->aabb)) {
            collision_response(body, static_body, aabb_minkowski_diff(static_body->aabb, body->aabb));
        }
    }
}

Physics* physics_init(u8 iterations) {
    Physics *physics = malloc(sizeof(*physics));

    physics->iterations = iterations;
    physics->body_list = array_list_init(sizeof(Body), 0);
    physics->static_body_list = array_list_init(sizeof(Static_Body), 0);

    LOG_TRACE("Physics: Successfully initialized physics");

    return physics;
}

void physics_destroy(Physics *self) {
    array_list_destroy(self->body_list);
    array_list_destroy(self->static_body_list);
    free(self);

    LOG_TRACE("Physics: Successfully destroyed physics");
}

// we suppose to have only 1 movable body so we don't need to loop through the movable and check collision for it
void physics_update(Physics *self, f32 dt) {
    Body *body;

    for (u32 i = 0; i < self->body_list->len; ++i ) {
        body = physics_body_get(self, i);
        vec2s scaled_velocity = glms_vec2_scale(body->velocity, dt * (1.0 / self->iterations));
        for (u8 j = 0; j < self->iterations; ++j) {

            // update position
            body->position.x += scaled_velocity.x;
            body->position.y += scaled_velocity.y;

            // update center
            body->aabb.center.x = body->position.x + body->aabb.half_size.x;
            body->aabb.center.y = body->position.y + body->aabb.half_size.y;

            collision_check(self, body);
        }
    }
}

u64 physics_body_create(Physics *self, vec2s position, vec2s size, u8 collision_mask, u8 collision_flag) {
    Body body = {
        .position       = position,
        .velocity       = {0,0},
        .collision_mask = collision_mask,
        .collision_flag = collision_flag,
        .aabb = { 
            .center    = (vec2s){position.x + size.x * 0.5, position.y + size.y * 0.5},
            .half_size = {size.x * 0.5, size.y * 0.5}
        },
    };
    
    array_list_append(self->body_list, &body);
    return self->body_list->len - 1;
}

Body* physics_body_get(Physics *self, u64 body_id) {
    return array_list_get(self->body_list, body_id);
}

void physics_body_reset(Physics *self) {
    self->body_list->len = 0;
}

u64  physics_static_body_create(Physics *self, vec2s position, vec2s size, u8 collision_mask, u8 collision_flag, 
        void(*on_hit_by_body)(Static_Body *body, Body *other)) {
    Static_Body body = {
        .aabb = { 
            .center    = (vec2s){position.x + size.x * 0.5, position.y + size.y * 0.5},
            .half_size = {size.x * 0.5, size.y * 0.5}
        },
        .collision_mask = collision_mask,
        .collision_flag = collision_flag,
        .on_hit_by_body = (on_hit_by_body) ? on_hit_by_body : NULL,
    };

    array_list_append(self->static_body_list, &body);
    return self->static_body_list->len - 1;
}

Static_Body* physics_static_body_get(Physics *self, u64 body_id) {
    return array_list_get(self->static_body_list, body_id); 
}

void physics_static_body_reset(Physics *self) {
    self->static_body_list->len = 0;
}

b8 aabb_intersect_aabb(AABB a, AABB b) {
    vec2s min, max;
    aabb_min_max(aabb_minkowski_diff(a,b), &min, &max);

    return (min.x <= 0 && max.x >= 0 && min.y <= 0 && max.y >= 0);
}

void aabb_min_max(AABB aabb, vec2s *min, vec2s *max) {
    *min = glms_vec2_sub(aabb.center, aabb.half_size);
    *max = glms_vec2_add(aabb.center, aabb.half_size);
}

AABB aabb_minkowski_diff(AABB a, AABB b) {
    AABB res = {
        .center    = glms_vec2_sub(a.center, b.center),
        .half_size = glms_vec2_add(a.half_size, b.half_size)
    };
    return res;
}

void aabb_penetration_vector(vec2s *pv, AABB aabb) {
    vec2s min, max;
    aabb_min_max(aabb, &min, &max);

    f32 min_dist = fabsf(min.x);
    pv->x = min.x;
    pv->y = 0;

    if (fabsf(max.x) < min_dist) {
        min_dist = fabsf(max.x);
        pv->x     = max.x;
    }

    if (fabsf(min.y) < min_dist) {
        min_dist = fabsf(min.y);
        pv->x     = 0;
        pv->y     = min.y;
    }

    if (fabsf(max.y) < min_dist) {
        pv->x     = 0;
        pv->y     = max.y;
    }
}
