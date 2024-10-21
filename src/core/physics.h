#ifndef PHYSICS_H
#define PHYSICS_H
#include "../util/types.h"
#include "../util/array_list.h"
#include <cglm/types-struct.h>

typedef enum {
  COLLISION_LAYER_NONE        = 1 << 0,
  COLLISION_LAYER_PLAYER      = 1 << 1,
  COLLISION_LAYER_SOLID       = 1 << 2,
  COLLISION_LAYER_TELEPORTER  = 1 << 3,
  COLLISION_LAYER_DIALOG      = 1 << 4,

  COLLISION_LAYER_LAST = 5
} CollisionLayer;

typedef struct {
  vec2s center;
  vec2s half_size;
} AABB;

typedef struct {
  AABB  aabb;
  vec2s velocity;
  vec2s position;
  u8    collision_mask;
  u8    collision_flag;
} Body;

typedef struct Static_Body {
  AABB aabb;
  u8   collision_mask;
  u8   collision_flag;

  void (*on_hit_by_body)(struct Static_Body *body, Body *other);
} Static_Body;

typedef struct {
  array_list *body_list;
  array_list *static_body_list;
  u8          iterations;
} Physics;

Physics* physics_init(u8 iterations);
void     physics_destroy(Physics *self);
void     physics_update(Physics *self, f32 dt);

u64   physics_body_create(Physics *self, vec2s position, vec2s size, u8 collision_mask, u8 collision_flag);
Body* physics_body_get(Physics *self, u64 body_id);
void  physics_body_reset(Physics *self);

u64          physics_static_body_create(Physics *self, vec2s position, vec2s size, u8 collision_mask, u8 collision_flag, void(*on_hit_by_body)(Static_Body *body, Body *other));
Static_Body* physics_static_body_get(Physics *self, u64 body_id);
void         physics_static_body_reset(Physics *self);

void aabb_min_max(AABB aabb, vec2s *min, vec2s *max);
AABB aabb_minkowski_diff(AABB a, AABB b);
b8   aabb_intersect_aabb(AABB a, AABB b);
void aabb_penetration_vector(vec2s *pv, AABB aabb);
#endif
