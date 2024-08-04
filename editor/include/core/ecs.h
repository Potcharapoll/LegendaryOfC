#ifndef ECS_H
#define ECS_H
#include "core/components.h"
#include "util/types.h"
#include "util/array_stack.h"

// This Entity Component System is assume that every entity 
// will have all same component.

#define ENTITY_ALIVE_FLAG (1 << 0)
#define INITIAL_CAPACITY  32
#define MAX_COMPONENTS    10

typedef u64 ecs_id_t;
typedef ecs_id_t ecs_entity_t;
typedef u8  ecs_flag_t;
typedef size_t ecs_size_t;

typedef struct {
    ecs_size_t len;
    ecs_entity_t *list;
} ecs_query_t;

typedef struct {
    array_stack *entity_pool; 
    ecs_flag_t *entity_flag;
    ecs_flag_t *entity_mask;
    u64 count;
    u64 count_alive;
} ecs_entities_t;

typedef struct { 
    ecs_size_t *offset;
    ecs_size_t *size;
    ecs_size_t stride;
    void *data;
    u8 count;
} ecs_components_t;

typedef struct {
    ecs_size_t capacity;
    ecs_entities_t *entities;
    ecs_components_t *components;
    ecs_query_t *query;
} ecs_world_t;

typedef struct {
    u8 n;
    Components components[MAX_COMPONENTS];
} ecs_desc_t;

ecs_world_t* ecs_init(void);
void ecs_destroy(ecs_world_t *world);
void ecs_killall(ecs_world_t *world);

ecs_entity_t ecs_create(ecs_world_t *world, ecs_desc_t desc);
void ecs_kill(ecs_world_t *world, ecs_entity_t entity_id);
void ecs_add(ecs_world_t *world, ecs_entity_t entity_id, ecs_id_t component_id, void *data);
const void* ecs_get(ecs_world_t *world, ecs_entity_t entity_id, ecs_id_t component_id);
void ecs_set(ecs_world_t *world, ecs_entity_t entity_id, ecs_id_t component_id, void *data);
void ecs_remove(ecs_world_t *world, ecs_entity_t entity_id, ecs_id_t component_id);
b8 ecs_is_alive(ecs_world_t *world, ecs_entity_t entity_id);
b8 ecs_has(ecs_world_t *world, ecs_entity_t entity_id, ecs_id_t component_id);
ecs_query_t* ecs_query(ecs_world_t *world);

#endif

