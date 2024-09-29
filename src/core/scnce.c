#include "scnce.h"
#include "camera.h"
#include "chunk.h"
#include "physics.h"

#include "../engine/logger.h"
#include "../global.h"
#include "../defs.h"

#include <pthread.h>
#include <stdlib.h>

static Chunk **_chunks = NULL;

static void _fade_update(Scnce *self) {
    f32 time = global.dt * 2;

    switch (self->fade_state) {
        case FADE_IN:
            if (self->fade_alpha > 0.0) {
                self->fade_alpha -= time;
            } 

            if (self->fade_alpha < 0.0) {
                self->fade_alpha = 0.0f;
                self->fade_state = FADE_NONE;
                self->faded = false;
            }
            break;
        case FADE_OUT:
            if (self->fade_alpha < 1.0) {
                self->fade_alpha += time;
            } 

            if (self->fade_alpha > 1.0) {
                self->fade_alpha = 1.0f;
                self->faded = true;
            }
            break;
        default:
            break;
    }
}

static void _border_collision(vec2s *a, vec2s size, vec4s position) {
    if (a->y < position.y) a->y = position.y;
    if (a->x < position.x) a->x = position.x;

    if (a->x + size.x > position.z) a->x = position.z - size.x;
    if (a->y + size.y > position.w) a->y = position.w - size.y;
}

static inline void _scnce_load_chunk(void) {

    pthread_mutex_lock(&global.lock);

    _chunks = malloc(CHUNK_LAST * sizeof(_chunks));
    _chunks[CHUNK_SPAWN]             = chunk_load_from_file("res/data/chunk_spawn");
    _chunks[CHUNK_VILLAGE_ENTRANCE]  = chunk_load_from_file("res/data/chunk_village_entrance");
    _chunks[CHUNK_VILLAGE_LEFT]      = chunk_load_from_file("res/data/chunk_village_left");
    _chunks[CHUNK_VILLAGE_RIGHT]     = chunk_load_from_file("res/data/chunk_village_right");
    _chunks[CHUNK_VILLAGE_TOP]       = chunk_load_from_file("res/data/chunk_village_top");
    _chunks[CHUNK_VILLAGE_TOP_END]   = chunk_load_from_file("res/data/chunk_village_top_end");
    _chunks[CHUNK_VILLAGE_TOP_LEFT]  = chunk_load_from_file("res/data/chunk_village_top_left");
    _chunks[CHUNK_VILLAGE_TOP_RIGHT] = chunk_load_from_file("res/data/chunk_village_top_right");
    _chunks[CHUNK_VILLAGE_TUNNEL]    = chunk_load_from_file("res/data/chunk_village_tunnel");
    _chunks[CHUNK_INSIDE_LIBRARY]    = chunk_load_from_file("res/data/chunk_inside_library");
    _chunks[CHUNK_INSIDE_RESTAURANT] = chunk_load_from_file("res/data/chunk_inside_restaurant");
    _chunks[CHUNK_INSIDE_CHURCH]     = chunk_load_from_file("res/data/chunk_inside_church");
    _chunks[CHUNK_INSIDE_FISH]       = chunk_load_from_file("res/data/chunk_inside_fish");
    _chunks[CHUNK_INSIDE_LJ_HOME]    = chunk_load_from_file("res/data/chunk_inside_lj_home");
    _chunks[CHUNK_INSIDE_OG_HOME]    = chunk_load_from_file("res/data/chunk_inside_og_home");
    _chunks[CHUNK_INSIDE_VC_HOME]    = chunk_load_from_file("res/data/chunk_inside_vc_home");

    pthread_mutex_unlock(&global.lock);
}

static inline void _scnce_setup_collider(Scnce *self) {
    physics_static_body_reset(global.physics);

    vec2s pos = (vec2s){self->chunk->position.x, self->chunk->position.y};
    for (u32 i = 0; i < self->chunk->collider_count; ++i) {
        physics_static_body_create(global.physics,
                (vec2s){self->chunk->collider[i].pos.x, self->chunk->collider[i].pos.y}, 
                (vec2s){self->chunk->collider[i].size.x, self->chunk->collider[i].size.y}, 
                COLLISION_LAYER_PLAYER, COLLISION_LAYER_SOLID, global.collision_callback); 
    } 

    for (u32 i = 0; i < self->chunk->teleporter_count; ++i) {
        self->chunk->teleporter[i].body_id = physics_static_body_create(global.physics,
                (vec2s){self->chunk->teleporter[i].pos.x, self->chunk->teleporter[i].pos.y}, 
                (vec2s){self->chunk->teleporter[i].size.x, self->chunk->teleporter[i].size.y}, 
                COLLISION_LAYER_PLAYER, COLLISION_LAYER_TELEPORTER, global.collision_callback); 
    } 

    for (u32 i = 0; i < self->chunk->dialog_count; ++i) {
        self->chunk->dialog[i].body_id = physics_static_body_create(global.physics,
                (vec2s){pos.x + TILE_SIZE * self->chunk->dialog[i].coord.x, pos.y + TILE_SIZE * self->chunk->dialog[i].coord.y}, 
                DEFAULT_SCALE, COLLISION_LAYER_PLAYER, COLLISION_LAYER_DIALOG, global.collision_callback); 
    } 
}

Scnce* scnce_init(void) {
    Scnce *scnce = malloc(sizeof(*scnce));
    ASSERT(scnce != NULL, "cannnot allocate memory for scnce", __FILE__, __LINE__);

    scnce->fade_alpha = 0.0f;
    scnce->fade_state = FADE_NONE;
    scnce->faded = false;

    scnce->camera = camera_init((vec2s){0,0});
    scnce->chunk  = NULL;

    LOG_TRACE("Scnce: Successfully initialized scnce");
    return scnce;
}

void scnce_destroy(Scnce *self) {

    if (_chunks) {
        for (u32 i = 0; i < CHUNK_LAST; ++i) {
            free(_chunks[i]->uv);
            free(_chunks[i]->prefab);
            free(_chunks[i]->dialog);
            free(_chunks[i]->collider);
            free(_chunks[i]->teleporter);
            free(_chunks[i]);
        }              
    }

    camera_destroy(self->camera);
    free(self);

    LOG_TRACE("Scnce: Successfully destroyed scnce");
}

// Use this when state is ingame
void scnce_update(Scnce *self, Body *player_body) {

#ifdef DEBUG 
    if (self->chunk == NULL) {
        LOG_WARN("Scnce: Chunk is NULL, cannot reload or reset the chunk");
    }

    { // debugging
        // relaod chunk from file
        if (global.reload_chunk) {
            pthread_mutex_lock(&global.lock);
            for (u32 i = 0; i < CHUNK_LAST; ++i) {
                free(_chunks[i]->uv);
                free(_chunks[i]->prefab);
                free(_chunks[i]->dialog);
                free(_chunks[i]->collider);
                free(_chunks[i]->teleporter);
                free(_chunks[i]);
            }              

            _scnce_load_chunk();
            self->chunk = _chunks[self->chunk_id];

            _scnce_setup_collider(self);
            pthread_mutex_unlock(&global.lock);

            global.reload_chunk = false;
        }

        // reset chunk
        if (global.reset_chunk) {
            pthread_mutex_lock(&global.lock);
            _scnce_setup_collider(self);
            pthread_mutex_unlock(&global.lock);

            global.reset_chunk = false;
        }
    }
#endif
    _fade_update(self);

    if (self->scnce_state == INGAME) {
        camera_center_to_obj(self->camera, player_body->position, PLAYER_HITBOX);
        _border_collision(&self->camera->position, (vec2s){PROJECTION_WIDTH, PROJECTION_HEIGHT}, self->chunk->position);
    }

    camera_update(self->camera); 

    {
        mat4s inv_vp      = glms_mat4_mul(self->camera->inverse_view_proj.view, self->camera->inverse_view_proj.proj);
        vec4s ortho_mouse = glms_mat4_mulv(inv_vp, (vec4s){global.window->mouse.normalx, global.window->mouse.normaly, 0.0f, 1.0f});
        global.window->mouse.orthox = ortho_mouse.x;
        global.window->mouse.orthoy = ortho_mouse.y;
    }
}

void scnce_change_chunk(Scnce *self, Body *player_body, Chunks chunk_id, vec2s target_coord) {
    self->chunk = _chunks[chunk_id];
    self->chunk_id = chunk_id;

    if (target_coord.x == -1) {
        player_body->position = (vec2s){player_body->position.x, self->chunk->position.y + target_coord.y * TILE_SIZE};
    }
    else if (target_coord.y == -1) {
        player_body->position = (vec2s){self->chunk->position.x + target_coord.x * TILE_SIZE, player_body->position.y};
    }
    else {
        player_body->position = (vec2s){self->chunk->position.x + target_coord.x * TILE_SIZE, self->chunk->position.y + target_coord.y * TILE_SIZE};
    }

    _scnce_setup_collider(self);
    if (self->fade_state == FADE_OUT) self->fade_state = FADE_IN;

    LOG_DEBUG("Scnce: Change chunk from %d->%d ", self->chunk_id, chunk_id);
}

void scnce_fade_reset(Scnce *self) {
    self->fade_state = FADE_NONE;
}

void scnce_fade_out(Scnce *self) {
    self->fade_state = FADE_OUT;
}

void scnce_fade_in(Scnce *self) {
    self->fade_state = FADE_IN;
}

void scnce_change_scnce(Scnce *self, enum ScnceState scnce) {
    self->scnce_state = scnce;

    switch (scnce) {
        case MENU:
            LOG_DEBUG("Scnce: Menu state");
            break;
        case INTRO:
            LOG_DEBUG("Scnce: Intro state");
            scnce_fade_in(self);
            break;
        case INGAME:
            LOG_DEBUG("Scnce: Ingame state");
            _scnce_load_chunk();
            scnce_change_chunk(self, physics_body_get(global.physics, global.PlayerState.body_id), CHUNK_SPAWN, SPAWN_COORD);

            scnce_fade_in(self);
            break;
    }
}  
