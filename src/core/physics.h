#ifndef PHYSICS_H
#define PHYSICS_H
#include "../util/types.h"
#include <cglm/types-struct.h>

typedef struct {
    vec2s center;
    vec2s half_size;
} AABB;

typedef struct {
    AABB  aabb;
    vec2s velocity;
    vec2s accel;
    bool  active;
} Body;

void physics_init(void);
void physics_destroy(void);
void physics_update(f32 dt);
u64 physics_body_create(vec2s position, vec2s size);
void physics_body_destroy(u64 body_id);
Body* physics_body_get(u64 body_id);
b8  aabb_intersect_aabb(AABB a, AABB b);
#endif
