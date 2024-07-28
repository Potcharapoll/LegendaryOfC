#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "ecs.h"
#include "array_stack.h"

#define COMPONENT_CAPACITY 32
#define INITIAL_CAPACITY 32

#define ENTITY_FLAG_ALIVE (1lu << 0)

typedef struct {
    size_t *component_size_arr;
    size_t *component_offset_arr;
    size_t capacity;
    size_t count;
    size_t size;
    void *data;
} component_state;

typedef struct {
    u32 *entity_mask;
    u32 *entity_flag;
    u32 capacity;
    u32 count;
} entity_state;

typedef struct {
    component_state components_state; 
    entity_state entity_state;
    ecs_query_t query;
    array_stack *entity_pool; // store killed enitty for reuse
} ecs_state;

static ecs_state state = {0};

void ecs_init(u32 n, ...) {
    if (n > COMPONENT_CAPACITY) {
        fprintf(stderr, "Components over maxmimum! (Maximum: 32)\n");
        return;
    }

    va_list ap;
    size_t offsets[n];
    size_t sizes[n];
    size_t size = 0;

    va_start(ap, n);
    for (u32 i = 0; i < n; i++) {
        sizes[i] = va_arg(ap, size_t);
        offsets[i] = size;
        size += sizes[i];
    }
    va_end(ap);

    size_t array_size = n * sizeof(size_t);
    state.components_state.component_size_arr = malloc(array_size);
    state.components_state.component_offset_arr = malloc(array_size);
    state.components_state.data = malloc(INITIAL_CAPACITY * size);
    state.components_state.size = size;
    state.components_state.count = n;
    state.components_state.capacity = COMPONENT_CAPACITY;
    memcpy(state.components_state.component_size_arr, sizes, array_size);
    memcpy(state.components_state.component_offset_arr, offsets, array_size);

    state.entity_state.entity_mask = malloc(INITIAL_CAPACITY * sizeof(u32));
    state.entity_state.entity_flag = malloc(INITIAL_CAPACITY * sizeof(u32));
    state.entity_state.count = 0;
    state.entity_state.capacity = INITIAL_CAPACITY;

    state.query.len = 0;
    state.query.list = malloc(INITIAL_CAPACITY * sizeof(*state.query.list));
    state.entity_pool = array_stack_init(0, sizeof(u32));
}

// destroy ecs
void ecs_destroy(void) {
    free(state.entity_state.entity_mask);
    free(state.entity_state.entity_flag);
    free(state.components_state.data);
    array_stack_destroy(state.entity_pool);
}

// create new entity
ecs_entity_t ecs_create(void) {
    u32 id;

    if (state.entity_pool->len > 0) {
        id = array_stack_pop(state.entity_pool);
    }
    else {
        id = state.entity_state.count++;

        if (id == state.entity_state.capacity) {
            state.entity_state.capacity *= 2;
            state.components_state.capacity *= 2;
            state.query.capacity *= 2;
            void *new_entity_mask_arr = realloc(state.entity_state.entity_mask, state.entity_state.capacity * sizeof(u32));
            void *new_entity_flag_arr = realloc(state.entity_state.entity_flag, state.entity_state.capacity * sizeof(u32));
            void *new_data = realloc(state.components_state.data, state.components_state.capacity * state.components_state.size);
            void *new_query = realloc(state.query.list, state.query.capacity);

            if (new_entity_mask_arr == NULL || new_entity_flag_arr == NULL || new_data == 0) exit(-1);
            state.entity_state.entity_mask = new_entity_mask_arr;
            state.entity_state.entity_flag = new_entity_flag_arr;
            state.components_state.data = new_data;
            state.query.list = new_query;
        }        

    }
    state.entity_state.entity_mask[id] = 0;
    state.entity_state.entity_flag[id] = ENTITY_FLAG_ALIVE;
    ecs_entity_t e = {.Id = id};

    return e;
}

// return component_id of entity_id 
void* ecs_get(u32 entity_id, u32 component_id) {
    return state.components_state.data + (entity_id * state.components_state.size + state.components_state.component_offset_arr[component_id]);
}

// add component data to entity
void ecs_add(u32 entity_id, u32 component_id, void *data) {
    size_t size = state.components_state.component_size_arr[component_id];
    void *ptr = ecs_get(entity_id, component_id);
    state.entity_state.entity_mask[entity_id] |= (1 << component_id);

    assert(ptr != NULL);
    assert(size > 0);

    memcpy(ptr, data, size); 
}

// remove component from entity
void ecs_remove(u32 entity_id, u32 component_id) {
    state.entity_state.entity_mask[entity_id] &= ~(1 << component_id);
}

u32 ecs_has(u32 entity_id, u32 component_id) {
    return (state.entity_state.entity_mask[entity_id] & (1 << component_id));
}

// kill the entity and push it to entity_pool
void ecs_kill(u32 entity_id) {
    if (state.entity_state.entity_flag[entity_id] & ENTITY_FLAG_ALIVE) {
        state.entity_state.entity_flag[entity_id] &= ~ENTITY_FLAG_ALIVE;
        state.entity_state.entity_mask[entity_id] = 0;
        array_stack_push(state.entity_pool, &entity_id);
    }
}

ecs_query_t ecs_query(u32 n, ...) {
    va_list components; 
    u32 mask = 0;

    state.query.len = 0;

    va_start(components, n);
    for (u32 i = 0; i < n; i++) {
        u32 id = va_arg(components, size_t);
        mask |= (1 << id);
    }
    va_end(components);

    for (u32 i = 0; i < state.entity_state.count; i++) {
        if (state.entity_state.entity_flag[i] & ENTITY_FLAG_ALIVE && (state.entity_state.entity_mask[i] & mask) == mask) {
            state.query.list[state.query.len++] = i; 
        }
    }
    return state.query;
}
