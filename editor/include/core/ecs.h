#ifndef ECS_H
#define ECS_H
#include "../util/types.h"

typedef struct {
    u32 Id;
} ecs_entity_t;

typedef struct {
    size_t len;
    size_t capacity;
    u32 *list;
} ecs_query_t;

void ecs_init(u32 n, ...);
void ecs_destroy(void);
ecs_entity_t ecs_create(void);
void ecs_kill(u32 entity_id);
void ecs_killall(void);
void ecs_add(u32 entity_id, u32 component_id, void *data);
void ecs_set(u32 entity_id, u32 component_id, void *data);
void ecs_remove(u32 entity_id, u32 component_id);
void* ecs_get(u32 entity_id, u32 component_id);
u32 ecs_has(u32 entity_id, u32 component_id);
ecs_query_t ecs_query(u32 n, ...);
#endif
