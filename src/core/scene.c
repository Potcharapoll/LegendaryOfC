#include "scene.h"
#include "dialog.h"

#include "../engine/logger.h"
#include "../global.h"
#include "../defs.h"
#include "game.h"
#include "renderer.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

static Chunk **_chunks = NULL;

typedef enum {
    DIALOG_POSITION_TITLE,
    DIALOG_POSITION_TEXT,
    DIALOG_POSITION_ANSWER1,
    DIALOG_POSITION_ANSWER2,
    DIALOG_POSITION_ANSWER3,
    DIALOG_POSITION_ANSWER4,
    DIALOG_POSITION_SELECT,

    DIALOG_POSITION_LAST,
} DialogRenderPosition;

static vec3s dialog_render_position[DIALOG_POSITION_LAST] = {
    { 14,       55,      0.0f},
    { 15,       40,      0.0f},
    { 15 + 50,  40 - 18, 0.0f},
    { 15 + 50,  40 - 38, 0.0f},
    { 15 + 150, 40 - 18, 0.0f},
    { 15 + 150, 40 - 38, 0.0f},
    { 10,       -6,      0.0f},
};

/* static b8 next_dialog   = true; */
/* static b8 animation_end = false; */

static void _fade_update(Scene *self) {
    f32 time = global.dt * 2;

    switch (self->fade_state) {
        case FADE_IN:
            if (self->fade_alpha > 0.0) {
                self->fade_alpha -= time;
            } 

            if (self->fade_alpha < 0.0) {
                self->fade_alpha = 0.0f;
                self->fading = false;
                scene_fade_reset(self);
            }
            break;
        case FADE_OUT:
            if (self->fade_alpha < 1.0) {
                self->fade_alpha += time;
            } 

            if (self->fade_alpha > 1.0) {
                self->fade_alpha = 1.0f;
                self->fading = false;
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

static inline void _scene_load_chunk(void) {

    pthread_mutex_lock(&global.lock);

    _chunks = malloc(CHUNK_LAST * sizeof(_chunks));
    _chunks[CHUNK_SPAWN]             = chunk_load_from_file(CHUNK_SPAWN_PATH);
    _chunks[CHUNK_VILLAGE_ENTRANCE]  = chunk_load_from_file(CHUNK_VILLAGE_ENTRANCE_PATH);
    _chunks[CHUNK_VILLAGE_LEFT]      = chunk_load_from_file(CHUNK_VILLAGE_LEFT_PATH);
    _chunks[CHUNK_VILLAGE_RIGHT]     = chunk_load_from_file(CHUNK_VILLAGE_RIGHT_PATH);
    _chunks[CHUNK_VILLAGE_TOP]       = chunk_load_from_file(CHUNK_VILLAGE_TOP_PATH);
    _chunks[CHUNK_VILLAGE_TOP_END]   = chunk_load_from_file(CHUNK_VILLAGE_TOP_END_PATH);
    _chunks[CHUNK_VILLAGE_TOP_LEFT]  = chunk_load_from_file(CHUNK_VILLAGE_TOP_LEFT_PATH);
    _chunks[CHUNK_VILLAGE_TOP_RIGHT] = chunk_load_from_file(CHUNK_VILLAGE_TOP_RIGHT_PATH);
    _chunks[CHUNK_VILLAGE_TUNNEL]    = chunk_load_from_file(CHUNK_VILLAGE_TUNNEL_PATH);
    _chunks[CHUNK_INSIDE_LIBRARY]    = chunk_load_from_file(CHUNK_INSIDE_LIBRARY_PATH);
    _chunks[CHUNK_INSIDE_RESTAURANT] = chunk_load_from_file(CHUNK_INSIDE_RESTAURANT_PATH);
    _chunks[CHUNK_INSIDE_CHURCH]     = chunk_load_from_file(CHUNK_INSIDE_CHURCH_PATH);
    _chunks[CHUNK_INSIDE_FISH]       = chunk_load_from_file(CHUNK_INSIDE_FISH_PATH);
    _chunks[CHUNK_INSIDE_LJ_HOME]    = chunk_load_from_file(CHUNK_INSIDE_LJ_HOME_PATH);
    _chunks[CHUNK_INSIDE_OG_HOME]    = chunk_load_from_file(CHUNK_INSIDE_OG_HOME_PATH);
    _chunks[CHUNK_INSIDE_VC_HOME]    = chunk_load_from_file(CHUNK_INSIDE_VC_HOME_PATH);

    pthread_mutex_unlock(&global.lock);
}

static inline void _scene_setup_collider(Scene *self) {
    physics_static_body_reset(global.physics);

    for (u32 i = 0; i < self->chunk->collider_count; ++i) {
        physics_static_body_create(global.physics,
                (vec2s){self->chunk->collider[i].pos.x, self->chunk->collider[i].pos.y}, 
                (vec2s){self->chunk->collider[i].size.x, self->chunk->collider[i].size.y}, 
                COLLISION_LAYER_PLAYER, COLLISION_LAYER_SOLID, NULL); 
    } 

    for (u32 i = 0; i < self->chunk->teleporter_count; ++i) {
        self->chunk->teleporter[i].body_id = physics_static_body_create(global.physics,
                (vec2s){self->chunk->teleporter[i].pos.x, self->chunk->teleporter[i].pos.y}, 
                (vec2s){self->chunk->teleporter[i].size.x, self->chunk->teleporter[i].size.y}, 
                COLLISION_LAYER_PLAYER, COLLISION_LAYER_TELEPORTER, global.teleporter_callback); 
    } 

    for (u32 i = 0; i < self->chunk->dialog_count; ++i) {
        self->chunk->dialog[i].body_id = physics_static_body_create(global.physics,
                (vec2s){self->chunk->dialog[i].pos.x,self->chunk->dialog[i].pos.y}, 
                DEFAULT_SCALE, COLLISION_LAYER_PLAYER, COLLISION_LAYER_DIALOG, global.dialog_callback); 
    } 
}

static void _scene_dialog_render(Scene *self) {
    DialogNode *curr = self->dialog;

    vec3s camera_pos = { self->camera->position.x, self->camera->position.y, 0.0f};
    vec3s pos        = {0};

    quad_renderer_append_quad(self->quad_renderer, camera_pos, DIALOG_FRAME_SIZE, DIALOG_FRAME_COLOR);

    pos = glms_vec3_add(camera_pos, dialog_render_position[DIALOG_POSITION_TITLE]);
    text_renderer_append_text(self->text_renderer, curr->name, pos, 10, BLUE);
    text_renderer_append_text(self->text_renderer, curr->name, pos, 10, DIALOG_TEXT_COLOR);

    pos = glms_vec3_add(camera_pos, dialog_render_position[DIALOG_POSITION_TEXT]);

    if (curr->type == DIALOG_TYPE_QUESTION) {
        DialogQuestion *content = curr->dialog;
        /* dialog_render_text_animation(content->question, DIALOG_TEXT_SIZE, pos, DIALOG_TEXT_COLOR); */
        text_renderer_append_text(self->text_renderer, content->question, pos, 8, DIALOG_TEXT_COLOR);

        u8 selected_answer = (self->selected_answer + DIALOG_POSITION_LAST - 5);
        /* if (animation_end) { */

            for (u8 i = 0; i < 4; ++i) {
                pos = glms_vec3_add(camera_pos, dialog_render_position[DIALOG_POSITION_ANSWER1+i]);
                text_renderer_append_text(self->text_renderer, content->answer[i], pos, 8, DIALOG_TEXT_COLOR);
            }

            pos = glms_vec3_add(camera_pos, glms_vec3_sub(dialog_render_position[selected_answer], dialog_render_position[DIALOG_POSITION_SELECT]));
            /* renderer_append_quad(LAYER_DIALOG, pos, DIALOG_SELECT_SIZE, DIALOG_TEXT_COLOR); */
            quad_renderer_append_quad(self->quad_renderer, pos, DIALOG_SELECT_SIZE, DIALOG_TEXT_COLOR);
        /* } */
    }
    else {
        DialogText *content = curr->dialog;
        /* dialog_render_text_animation(content->text, DIALOG_TEXT_SIZE, pos, DIALOG_TEXT_COLOR); */
        text_renderer_append_text(self->text_renderer, content->text, pos, 8, DIALOG_TEXT_COLOR);
    }
}


Scene* scene_init(void) {
    Scene *scene = malloc(sizeof(*scene));
    ASSERT(scene != NULL, "Failed to allocate memory for scene", __FILE__, __LINE__);

    scene->fade_alpha = 0.0f;
    scene->fade_state = FADE_NONE;
    scene->fading     = false;
    scene->faded      = false;

    scene->gradient   = glms_vec4_zero();

    scene->quad_renderer = quad_renderer_init();
    scene->text_renderer = text_renderer_init(global.get_char_coord);

    scene->dialog = NULL;
    scene->on_dialog = false;
    scene->selected_answer = 0;

    scene->camera = camera_init((vec2s){0,0});
    scene->chunk  = NULL;

    LOG_TRACE("Scene: Successfully initialized scene");
    return scene;
}

void scene_destroy(Scene *self) {

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

    quad_renderer_destroy(self->quad_renderer);
    text_renderer_destroy(self->text_renderer);
    camera_destroy(self->camera);
    free(self);

    LOG_TRACE("Scene: Successfully destroyed scene");
}

// Use this when state is ingame
void scene_update(Scene *self, Body *player_body) {

#ifdef DEBUG 
    if (self->chunk == NULL) {
        LOG_WARN("Scene: Chunk is NULL, cannot reload or reset the chunk");
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

            _scene_load_chunk();
            self->chunk = _chunks[self->chunk_id];

            _scene_setup_collider(self);
            pthread_mutex_unlock(&global.lock);

            global.reload_chunk = false;
        }

        // reset chunk
        if (global.reset_chunk) {
            pthread_mutex_lock(&global.lock);
            _scene_setup_collider(self);
            pthread_mutex_unlock(&global.lock);

            global.reset_chunk = false;
        }
    }
#endif
    _fade_update(self);

    if (self->scene_state == INGAME) {
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

void scene_render(Scene *self) {

    switch (self->scene_state) {
        case MENU:
            text_renderer_append_text(self->text_renderer, "Press SPACE to start game" , (vec3s){PROJECTION_WIDTH*0.5 - (25*3.5*0.5), 50}, 7, WHITE);
            break;
        case INTRO:
            text_renderer_append_text(self->text_renderer, "A few days ago, I received a letter, It was written about my" ,     (vec3s){PROJECTION_WIDTH*0.5 - (60*3.5*0.5), 120}, 7, YELLOW);
            text_renderer_append_text(self->text_renderer, "missing grandfather and where I could find him. I was so confused", (vec3s){PROJECTION_WIDTH*0.5 - (65*3.5*0.5), 112}, 7, YELLOW);
            text_renderer_append_text(self->text_renderer, "I had no choice, so I decided to go to the place",                  (vec3s){PROJECTION_WIDTH*0.5 - (48*3.5*0.5), 104}, 7, YELLOW);
            text_renderer_append_text(self->text_renderer, "where it was written, called 'CVillage'.",                          (vec3s){PROJECTION_WIDTH*0.5 - (40*3.5*0.5), 96},  7, YELLOW);
            break;
        case ENDGAME:
            text_renderer_append_text(self->text_renderer, "Congreatulation! You've cleared the game!" , (vec3s){PROJECTION_WIDTH*0.5 - (25*3.5*0.5), 50}, 7, WHITE);
            break;
        default:
            break;
    }

    if (self->scene_state == INGAME && self->on_dialog && !self->faded) {
        _scene_dialog_render(self);
    }

    { 
        quad_renderer_append_quad(self->quad_renderer, (vec3s){global.scene->camera->position.x,global.scene->camera->position.y,0.0f}, 
                (vec2s){WIDTH, HEIGHT}, self->gradient);
        quad_renderer_append_quad(self->quad_renderer, (vec3s){global.scene->camera->position.x,global.scene->camera->position.y,0.0f}, 
                (vec2s){WIDTH, HEIGHT}, (vec4s){0,0,0, self->fade_alpha});
    }

    text_renderer_render(self->text_renderer);
    quad_renderer_render(self->quad_renderer);
}

void scene_change_chunk(Scene *self, Body *player_body, Chunks chunk_id, vec2s target_coord) {
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

    _scene_setup_collider(self);
    if (self->fade_state == FADE_OUT) scene_fade_in(self);
    LOG_DEBUG("Scene: Change chunk from %d->%d ", self->chunk_id, chunk_id);
}

void scene_fade_reset(Scene *self) {
    self->fade_state = FADE_NONE;
    self->faded = false;
}

void scene_fade_out(Scene *self) {
    self->fade_state = FADE_OUT;
    self->fading = true;
}

void scene_fade_in(Scene *self) {
    self->fade_state = FADE_IN;
    self->fading = true;
}

void scene_attach_dialog(Scene *self, Dialog *dialog, char *tag) {
    if (self->dialog != NULL) {
        LOG_ERROR("Scene: Failed to attach dialog to scene, dialog isn't NULL");
        return;
    }

    self->dialog_tag = malloc(strlen(tag) + 1);
    strcpy(self->dialog_tag, tag);
    LOG_WARN("Scene: Tag %s", self->dialog_tag);

    self->dialog = dialog->contents;
    self->on_dialog = true;
}

void scene_change_scene(Scene *self, enum SceneState scene) {
    self->scene_state = scene;

    // TODO: Fix fadeing
    switch (scene) {
        case MENU:
            LOG_DEBUG("Scene: Menu state");
            break;
        case INTRO:
            LOG_DEBUG("Scene: Intro state");
            scene_fade_in(self);
            break;
        case INGAME:
            LOG_DEBUG("Scene: Ingame state");
            _scene_load_chunk();
            scene_change_chunk(self, physics_body_get(global.physics, global.PlayerState.body_id), CHUNK_SPAWN, SPAWN_COORD);
            scene_fade_in(self);
            break;
        case ENDGAME:
            break;
    }
}  

DialogNode *scene_get_curr_dialog(Scene *self) {
    return self->dialog; 
}

void scene_dialog_next(Scene *self) {
    self->dialog = self->dialog->next;

    if (!self->dialog)  {
        LOG_DEBUG("Scene: Dialog ended");
        game_update_dialog_state(self->dialog_tag); 

        self->dialog = NULL;
        self->on_dialog = false;
        free(self->dialog_tag);
    }
}
