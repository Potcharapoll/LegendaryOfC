#ifndef PHYSICS_H
#define PHYSICS_H
#include "../util/types.h"
#include <cglm/types-struct.h>

typedef enum {
    COLLISION_NONE        = 1 << 0,
    COLLISION_PLAYER      = 1 << 1,
    COLLISION_SOLID       = 1 << 2,
    COLLISION_TELEPORTER  = 1 << 3,
    COLLISION_DIALOG      = 1 << 4
} CollisionLayer;

typedef struct {
    vec2s center;
    vec2s half_size;
} AABB;

typedef struct {
    AABB  aabb;
    vec2s velocity;
    vec2s position;
    u8 collision_mask;
    u8 collision_flag;
} Body;

typedef struct Static_Body {
    AABB aabb;
    u8 collision_mask;
    u8 collision_flag;

    void (*on_hit_by_body)(struct Static_Body *body, Body *other);
} Static_Body;

// Explaination: collision_mask is the object that can collide with a specify object
//               collision_flag is the type of object

void physics_init(void);
void physics_destroy(void);
void physics_update(f32 dt);
void physics_render_collider(void);

u64  physics_body_create(vec2s position, vec2s size, u8 collision_mask, u8 collision_flag);
Body* physics_body_get(u64 body_id);

u64  physics_static_body_create(vec2s position, vec2s size, u8 collision_mask, u8 collision_flag, void(*on_hit_by_body)(Static_Body *body, Body *other));
Static_Body* physics_static_body_get(u64 body_id);
void physics_static_body_reset(void);

void aabb_min_max(AABB aabb, vec2s *min, vec2s *max);
AABB aabb_minkowski_diff(AABB a, AABB b);
b8  aabb_intersect_aabb(AABB a, AABB b);
void aabb_penetration_vector(vec2s *pv, AABB aabb);
#endif
