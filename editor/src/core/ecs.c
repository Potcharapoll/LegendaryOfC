#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "core/ecs.h"
#include "util/log.h"

#define ECS_COMPONENT(world, component) ecs_add_component(world, sizeof(component));
#define GET_COMPO_TXT(x) (x == POSITION_COMPONENT) ? "POSITION_COMPONENT" : (x == SPRITE_COMPONENT) ? "SPRITE_COMPONENT" : "UPDATABLE_COMPONENT"
#define GET_ALIVE_TXT(x) (x & ENTITY_ALIVE_FLAG) ? "Alive" : "Die"

static void ecs_add_component(ecs_world_t *world, ecs_size_t size) {
    assert(world != NULL);
    assert(world->components->count != MAX_COMPONENTS);

    ecs_size_t idx = world->components->count;

    ecs_size_t offset = 0;
    for (ecs_size_t i = 0; i < idx; i++) {
        offset += world->components->size[i];
    }

    world->components->offset[idx] = offset;
    world->components->size[idx]   = size;
    world->components->stride     += size;
    world->components->count++;
}

ecs_world_t* ecs_init(void) {
    ecs_world_t *world = malloc(sizeof(*world));
    assert(world != NULL);

    world->capacity = INITIAL_CAPACITY;

    world->entities = malloc(sizeof(*world->entities));
    assert(world->entities != NULL);

    world->entities->count       = 0;
    world->entities->count_alive = 0;
    world->entities->entity_flag = calloc(INITIAL_CAPACITY, sizeof(ecs_flag_t));
    world->entities->entity_mask = calloc(INITIAL_CAPACITY, sizeof(ecs_flag_t));
    world->entities->entity_pool = array_stack_init(sizeof(ecs_id_t), INITIAL_CAPACITY);

    world->components = malloc(sizeof(*world->components));
    assert(world->components != NULL);

    world->components->count  = 0;
    world->components->stride = 0;
    world->components->size   = calloc(INITIAL_CAPACITY, sizeof(ecs_size_t));
    world->components->offset = calloc(INITIAL_CAPACITY, sizeof(ecs_size_t));
    assert(world->components->offset != NULL);
    assert(world->components->size != NULL);

    // add components to ecs
    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Sprite);
    ECS_COMPONENT(world, Updatable);

    world->components->data = calloc(INITIAL_CAPACITY, world->components->stride);
    assert(world->components->data != NULL);

    world->query = malloc(sizeof(*world->query));
    assert(world->query != NULL);

    world->query->len  = 0;
    world->query->list = calloc(INITIAL_CAPACITY, sizeof(ecs_id_t));
    return world;
}

void ecs_destroy(ecs_world_t *world) {
    assert(world != NULL);

    array_stack_destroy(world->entities->entity_pool);
    free(world->entities->entity_mask);
    free(world->entities->entity_flag);
    free(world->entities);

    free(world->components->size);
    free(world->components->offset);
    free(world->components->data);
    free(world->components);

    free(world->query->list);
    free(world->query);
    free(world);
}

ecs_entity_t ecs_create(ecs_world_t *world, ecs_desc_t desc) {
    assert(world != NULL);

    ecs_entity_t id;
    if (world->entities->entity_pool->len > 0) {
        id = array_stack_pop(world->entities->entity_pool);
    }
    else {
        if (world->entities->count == world->capacity) {
            world->capacity *= 2;
            void *new_entity_flag = realloc(world->entities->entity_flag, world->capacity * sizeof(ecs_flag_t));
            void *new_entity_mask = realloc(world->entities->entity_mask, world->capacity * sizeof(ecs_flag_t));
            void *new_offset      = realloc(world->components->offset, world->capacity * sizeof(ecs_size_t));
            void *new_size        = realloc(world->components->size, world->capacity * sizeof(ecs_size_t));
            void *new_data        = realloc(world->components->data, world->capacity * world->components->stride);
            void *new_query       = realloc(world->query->list, world->capacity * sizeof(ecs_id_t));

            if (!new_entity_flag || !new_entity_mask || !new_offset || !new_size || !new_data || !new_query) {
                LOG_FETAL("Could not reallocate memory for the ecs");
                abort();
            }

            world->entities->entity_flag = new_entity_flag;
            world->entities->entity_mask = new_entity_mask;
            world->components->offset    = new_offset;
            world->components->size      = new_size;
            world->components->data      = new_data;
            world->query->list           = new_query;
        }

        id = world->entities->count;
    }
    
    ecs_flag_t mask = 0;
    for (ecs_size_t i = 0; i < desc.n; i++) {
        mask |= (1 << desc.components[i]);
    }

    world->entities->entity_flag[id] = ENTITY_ALIVE_FLAG;
    world->entities->entity_mask[id] = mask;

    world->entities->count++;
    world->entities->count_alive++;
    return id;
}

void ecs_kill(ecs_world_t *world, ecs_entity_t entity_id) {
    assert(world != NULL);

    world->entities->entity_flag[entity_id] &= ~ENTITY_ALIVE_FLAG;
    world->entities->count_alive--;
}

void ecs_killall(ecs_world_t *world) {
    assert(world != NULL);

    for (ecs_entity_t id = 0; id < world->entities->count; id++) {
        if (ecs_is_alive(world, id)) {
            ecs_kill(world, id);
        }
    }
}

void ecs_add(ecs_world_t *world, ecs_entity_t entity_id, ecs_id_t component_id, void *data) {
    assert(world != NULL);
    assert(data != NULL);

    if (ecs_has(world, entity_id, component_id) || !ecs_is_alive(world, entity_id))  {
        LOG_ERROR("Entity %lu already have %s or Entity %lu is not alive", entity_id, GET_COMPO_TXT(component_id), entity_id);
        return;
    }

    ecs_flag_t mask = (1 << component_id);
    world->entities->entity_mask[entity_id] |= mask;

    void *ptr = (u8*)world->components->data + entity_id * world->components->stride + world->components->offset[component_id];
    memcpy(ptr, data, world->components->size[component_id]);
}

const void* ecs_get(ecs_world_t *world, ecs_entity_t entity_id, ecs_id_t component_id) {
    assert(world != NULL);

    if (!ecs_has(world, entity_id, component_id) || !ecs_is_alive(world, entity_id)) {
        LOG_ERROR("Entity %lu doesn't have %s or Entity %lu is not alive", entity_id, GET_COMPO_TXT(component_id), entity_id);
        return NULL;
    }
    return (u8*)world->components->data + entity_id * world->components->stride + world->components->offset[component_id];
}

void ecs_set(ecs_world_t *world, ecs_entity_t entity_id, ecs_id_t component_id, void *data) {
    assert(world != NULL);

    if (!ecs_has(world, entity_id, component_id) || !ecs_is_alive(world, entity_id))  {
        LOG_ERROR("Entity %lu doesn't have %s or Entity %lu is not alive", entity_id, GET_COMPO_TXT(component_id), entity_id);
        return;
    }

    void *ptr = (u8*)world->components->data + entity_id * world->components->stride + world->components->offset[component_id];
    memcpy(ptr, data, world->components->size[component_id]);
}

void ecs_remove(ecs_world_t *world, ecs_entity_t entity_id, ecs_id_t component_id) {
    assert(world != NULL);

    if (!ecs_has(world, entity_id, component_id) || !ecs_is_alive(world, entity_id)) {
        LOG_ERROR("Entity %lu doesn't have %s or Entity %lu is not alive", entity_id, GET_COMPO_TXT(component_id), entity_id);
        return;
    }

    ecs_flag_t mask = (1 << component_id);
    world->entities->entity_mask[entity_id] ^= mask;
}

b8 ecs_is_alive(ecs_world_t *world, ecs_entity_t entity_id) {
    assert(world != NULL);

    return (world->entities->entity_flag[entity_id] & ENTITY_ALIVE_FLAG) ? true : false;
}

b8 ecs_has(ecs_world_t *world, ecs_entity_t entity_id, ecs_id_t component_id) {
    assert(world != NULL);

    ecs_flag_t mask = (1 << component_id);
    return ((world->entities->entity_mask[entity_id] & mask) == mask) ? true : false;
}

// query only alive enitty
ecs_query_t* ecs_query(ecs_world_t *world) {
    assert(world != NULL);

    ecs_size_t idx = 0;
    for (ecs_entity_t id = 0; id < world->entities->count; id++) {
        if (ecs_is_alive(world, id)) {
            world->query->list[idx++] = id; 
        }
    }
    world->query->len = idx;
    return world->query;
}
