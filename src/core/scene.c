#include "scene.h"
#include "chunk.h"
#include "game.h"

#include "../engine/logger.h"
#include "../global.h"
#include "../defs.h"
#include "player.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

static Chunk *_curr_chunk = NULL;

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
  { 14      , 55,      0.0f},
  { 15      , 40,      0.0f},
  { 15      , 40 - 28, 0.0f},
  { 15      , 40 - 38, 0.0f},
  { 15 + 150, 40 - 28, 0.0f},
  { 15 + 150, 40 - 38, 0.0f},
  { 5      , -3     , 0.0f},
};

/* static b8 next_dialog   = true; */
/* static b8 animation_end = false; */

static void _load_chunk(Scene *self, Chunks chunkId) {
  pthread_mutex_lock(&global.lock);

  if (_curr_chunk) {
    chunk_destroy(&_curr_chunk);
  }

  self->chunk_id = chunkId;

  switch (chunkId) {
    case CHUNK_SPAWN             :
      _curr_chunk = chunk_load_from_file(CHUNK_SPAWN_PATH);
      break;
    case CHUNK_VILLAGE_ENTRANCE  :
      _curr_chunk = chunk_load_from_file(CHUNK_VILLAGE_ENTRANCE_PATH);
      break;
    case CHUNK_VILLAGE_LEFT      :
      _curr_chunk = chunk_load_from_file(CHUNK_VILLAGE_LEFT_PATH);
      break;
    case CHUNK_VILLAGE_RIGHT     :
      _curr_chunk = chunk_load_from_file(CHUNK_VILLAGE_RIGHT_PATH);
      break;
    case CHUNK_VILLAGE_TOP       :
      _curr_chunk = chunk_load_from_file(CHUNK_VILLAGE_TOP_PATH);
      break;
    case CHUNK_VILLAGE_TOP_END   :
      _curr_chunk = chunk_load_from_file(CHUNK_VILLAGE_TOP_END_PATH);
      break;
    case CHUNK_VILLAGE_TOP_LEFT  :
      _curr_chunk = chunk_load_from_file(CHUNK_VILLAGE_TOP_LEFT_PATH);
      break;
    case CHUNK_VILLAGE_TOP_RIGHT :
      _curr_chunk = chunk_load_from_file(CHUNK_VILLAGE_TOP_RIGHT_PATH);
      break;
    case CHUNK_VILLAGE_TUNNEL    :
      _curr_chunk = chunk_load_from_file(CHUNK_VILLAGE_TUNNEL_PATH);
      break;
    case CHUNK_INSIDE_LIBRARY    :
      _curr_chunk = chunk_load_from_file(CHUNK_INSIDE_LIBRARY_PATH);
      break;
    case CHUNK_INSIDE_RESTAURANT :
      _curr_chunk = chunk_load_from_file(CHUNK_INSIDE_RESTAURANT_PATH);
      break;
    case CHUNK_INSIDE_CHURCH     :
      _curr_chunk = chunk_load_from_file(CHUNK_INSIDE_CHURCH_PATH);
      break;
    case CHUNK_INSIDE_FISH       :
      _curr_chunk = chunk_load_from_file(CHUNK_INSIDE_FISH_PATH);
      break;
    case CHUNK_INSIDE_OG_HOME    :
      _curr_chunk = chunk_load_from_file(CHUNK_INSIDE_OG_HOME_PATH);
      break;
    case CHUNK_INSIDE_LJ_HOME    :
      _curr_chunk = chunk_load_from_file(CHUNK_INSIDE_LJ_HOME_PATH);
      break;
    case CHUNK_INSIDE_VC_HOME    :
      _curr_chunk = chunk_load_from_file(CHUNK_INSIDE_VC_HOME_PATH);
      break;
  }

  pthread_mutex_unlock(&global.lock);
}

static void _fade_update(Scene *self) {
  f32 time = global.dt * 2;

  switch (self->fade_state) {
    case FADE_IN:
      if (self->fade_alpha > 0.0) {
        self->fade_alpha -= time;
      } 

      if (self->fade_alpha < 0.0) {
        self->fade_alpha = 0.0f;
        self->fading     = false;
        scene_fade_reset(self);
      }
      break;
    case FADE_OUT:
      if (self->fade_alpha < 1.0) {
        self->fade_alpha += time;
      } 

      if (self->fade_alpha > 1.0) {
        self->fade_alpha = 1.0f;
        self->fading     = false;
        self->faded      = true;
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

static void _scene_setup_collider(Scene *self) {
  ASSERT(_curr_chunk != NULL, "Curr Chunk is NULL", __FILE__, __LINE__);
  Chunk *chunk = _curr_chunk;

  if (self->chunk_teleporters) { FREE(self->chunk_teleporters); }
  if (self->chunk_dialogs)     { FREE(self->chunk_dialogs); }
  if (self->chunk_colliders)   { FREE(self->chunk_colliders); }

  self->chunk_teleporters = array_list_init_from_list(chunk->teleporter);
  self->chunk_colliders   = array_list_init_from_list(chunk->collider);
  self->chunk_dialogs     = array_list_init_from_list(chunk->dialog);
  physics_static_body_reset(global.physics);

  for (u32 i = 0; i < self->chunk_colliders->len; ++i) {
    ChunkCollider *collider = array_list_get(self->chunk_colliders, i);

    physics_static_body_create(global.physics,
        (vec2s){collider->pos.x, collider->pos.y}, 
        (vec2s){collider->size.x, collider->size.y}, 
        COLLISION_LAYER_PLAYER, COLLISION_LAYER_SOLID, NULL); 
  } 

  for (u32 i = 0; i < self->chunk_teleporters->len; ++i) {
    ChunkTeleporter *teleporter = array_list_get(self->chunk_teleporters, i);

    teleporter->body_id = physics_static_body_create(global.physics,
        (vec2s){teleporter->pos.x, teleporter->pos.y}, 
        (vec2s){teleporter->size.x, teleporter->size.y}, 
        COLLISION_LAYER_PLAYER, COLLISION_LAYER_TELEPORTER, global.teleporter_callback); 
  } 

  for (u32 i = 0; i < self->chunk_dialogs->len; ++i) {
    ChunkDialog *dialog = array_list_get(self->chunk_dialogs, i);

    dialog->body_id = physics_static_body_create(global.physics,
        (vec2s){dialog->pos.x,dialog->pos.y}, 
        DEFAULT_SCALE, COLLISION_LAYER_PLAYER, COLLISION_LAYER_DIALOG, global.dialog_callback); 
  } 
}

static void _scene_dialog_render(Scene *self) {
  DialogNode *curr = self->dialog;

  static u8 size = 6;

  vec3s camera_pos = { self->camera->position.x, self->camera->position.y, 0.0f};
  vec3s pos        = {0};

  quad_renderer_append_quad(self->quad_renderer, camera_pos, DIALOG_FRAME_SIZE, DIALOG_FRAME_COLOR);

  pos = glms_vec3_add(camera_pos, dialog_render_position[DIALOG_POSITION_TITLE]);
  text_renderer_append_text(self->text_renderer, curr->name, pos, 10, DIALOG_TEXT_COLOR);

  pos = glms_vec3_add(camera_pos, dialog_render_position[DIALOG_POSITION_TEXT]);

  if (curr->type == DIALOG_TYPE_QUESTION) {
    DialogQuestion *content = curr->dialog;
    /* dialog_render_text_animation(content->question, DIALOG_TEXT_SIZE, pos, DIALOG_TEXT_COLOR); */
    text_renderer_append_text(self->text_renderer, content->question, pos, size, DIALOG_TEXT_COLOR);

    u8 selected_answer = (self->selected_answer + DIALOG_POSITION_LAST - 5);
    /* if (animation_end) { */

    for (u8 i = 0; i < 4; ++i) {
      pos = glms_vec3_add(camera_pos, dialog_render_position[DIALOG_POSITION_ANSWER1+i]);
      text_renderer_append_text(self->text_renderer, content->answer[i], pos, size, DIALOG_TEXT_COLOR);
    }

    pos = glms_vec3_add(camera_pos, glms_vec3_sub(dialog_render_position[selected_answer], dialog_render_position[DIALOG_POSITION_SELECT]));
    /* renderer_append_quad(LAYER_DIALOG, pos, DIALOG_SELECT_SIZE, DIALOG_TEXT_COLOR); */
    quad_renderer_append_quad(self->quad_renderer, pos, DIALOG_SELECT_SIZE, DIALOG_TEXT_COLOR);
    /* } */
  }
  else {
    DialogText *content = curr->dialog;
    /* dialog_render_text_animation(content->text, DIALOG_TEXT_SIZE, pos, DIALOG_TEXT_COLOR); */
    text_renderer_append_text(self->text_renderer, content->text, pos, size, DIALOG_TEXT_COLOR);
  }
}

void scene_collider_reset(Scene *scene) {
  _scene_setup_collider(scene);
}

Scene* scene_init(void) {
  Scene *scene = malloc(sizeof(*scene));
  ASSERT(scene != NULL, "Failed to allocate memory for scene", __FILE__, __LINE__);

  scene->fade_alpha        = 0.0f;
  scene->fade_state        = FADE_NONE;
  scene->fading            = false;
  scene->faded             = false;
  scene->quad_renderer     = quad_renderer_init();
  scene->text_renderer     = text_renderer_init(global.get_char_coord);
  scene->dialog            = NULL;
  scene->on_dialog         = false;
  scene->selected_answer   = 0;
  scene->camera            = camera_init((vec2s){0,0});
  scene->chunk_id          = 99;
  scene->chunk_dialogs     = NULL;
  scene->chunk_colliders   = NULL;
  scene->chunk_teleporters = NULL;

  _curr_chunk = NULL;

  LOG_TRACE("Scene: Successfully initialized scene");
  return scene;
}

void scene_destroy(Scene *self) {

  if (_curr_chunk) {
    chunk_destroy(&_curr_chunk);
  }

  quad_renderer_destroy(self->quad_renderer);
  text_renderer_destroy(self->text_renderer);
  camera_destroy(self->camera);

  FREE(self->chunk_dialogs);
  FREE(self->chunk_colliders);
  FREE(self->chunk_teleporters);
  FREE(self);

  LOG_TRACE("Scene: Successfully destroyed scene");
}

// Use this when state is ingame
void scene_update(Scene *self, Body *player_body) {

/* #ifdef DEBUG */ 
/*     // relaod chunk from file */
/*     if (global.reload_chunk) { */
/*       pthread_mutex_lock(&global.lock); */
/*       for (u32 i = 0; i < CHUNK_LAST; ++i) { */
/*         chunk_destroy(&_chunks[i]); */
/*       } */              
/*       free(_chunks); */
/*       _chunks = NULL; */

/*       _scene_load_chunk(); */
/*       _scene_setup_collider(self); */
/*       pthread_mutex_unlock(&global.lock); */

/*       global.reload_chunk = false; */
/*     } */

/*     // reset chunk */
/*     if (global.reset_chunk) { */
/*       pthread_mutex_lock(&global.lock); */
/*       _scene_setup_collider(self); */
/*       pthread_mutex_unlock(&global.lock); */

/*       global.reset_chunk = false; */
/*     } */
/* #endif */

  _fade_update(self);

  if (self->scene_state == SCENE_INGAME) {
    /* Chunk *chunk = _chunks[self->chunk_id]; */
    Chunk *chunk = _curr_chunk;


    if (chunk == NULL) {
      LOG_ERROR("Scene: Chunk is NULL, cannot reload or reset the chunk");
      return;
    }

    camera_center_to_obj(self->camera, player_body->position, PLAYER_HITBOX);
    _border_collision(&self->camera->position, (vec2s){PROJECTION_WIDTH, PROJECTION_HEIGHT}, chunk->render_info->position);
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
    case SCENE_MENU:
      text_renderer_append_text(self->text_renderer, "Press SPACE to start game" , (vec3s){PROJECTION_WIDTH*0.5 - (25*3.5*0.5), 50}, 7, WHITE);
      break;
    case SCENE_INTRO:
      text_renderer_append_text(self->text_renderer, "A few days ago, I received a letter, It was written about my" ,     (vec3s){PROJECTION_WIDTH*0.5 - (60*3.5*0.5), 120}, 7, YELLOW);
      text_renderer_append_text(self->text_renderer, "missing grandfather and where I could find him. I was so confused", (vec3s){PROJECTION_WIDTH*0.5 - (65*3.5*0.5), 112}, 7, YELLOW);
      text_renderer_append_text(self->text_renderer, "I had no choice, so I decided to go to the place",                  (vec3s){PROJECTION_WIDTH*0.5 - (48*3.5*0.5), 104}, 7, YELLOW);
      text_renderer_append_text(self->text_renderer, "where it was written, called 'CVillage'.",                          (vec3s){PROJECTION_WIDTH*0.5 - (40*3.5*0.5), 96},  7, YELLOW);
      break;
    case SCENE_INGAME:
      if (self->on_dialog && !self->faded) {
        _scene_dialog_render(self);
      }
      break;
    case SCENE_ENDGAME:
      text_renderer_append_text(self->text_renderer, "Congreatulation! You've cleared the game!" , (vec3s){PROJECTION_WIDTH*0.5 - (25*3.5*0.5), 50}, 7, WHITE);
      text_renderer_append_text(self->text_renderer, "Press SPACE to start game" , (vec3s){PROJECTION_WIDTH*0.25 - (25*3.5*0.5), 50}, 7, WHITE);
      break;
    default:
      break;
  }

  quad_renderer_append_quad(self->quad_renderer, (vec3s){global.scene->camera->position.x,global.scene->camera->position.y,0.0f}, (vec2s){WIDTH, HEIGHT}, (vec4s){0,0,0, self->fade_alpha});

  if (self->dialog != NULL) {
    quad_renderer_render(self->quad_renderer);
    text_renderer_render(self->text_renderer);
  }
  else {
    text_renderer_render(self->text_renderer);
    quad_renderer_render(self->quad_renderer);
  }
}

void scene_chunk_change(Scene *self, Body *player_body, Chunks chunk_id, vec2s target_coord) {
  LOG_DEBUG("Scene: Change chunk from %d->%d (%.2f,%.2f)", self->chunk_id, chunk_id, target_coord.x, target_coord.y);

  _load_chunk(self, chunk_id);
  Chunk *chunk = _curr_chunk;

  player_body->position = 
    (target_coord.x == -1) ? (vec2s){player_body->position.x, chunk->render_info->position.y + target_coord.y * TILE_SIZE}  :
    (target_coord.y == -1) ? (vec2s){chunk->render_info->position.x + target_coord.x * TILE_SIZE, player_body->position.y}  :
                             (vec2s){chunk->render_info->position.x + target_coord.x * TILE_SIZE, chunk->render_info->position.y + target_coord.y * TILE_SIZE};

  _scene_setup_collider(self);

  if (self->faded) scene_fade_in(self);
}

void scene_fade_reset(Scene *self) {
  self->fade_state = FADE_NONE;
  self->faded      = false;
}

void scene_fade_out(Scene *self) {
  self->fade_state = FADE_OUT;
  self->fading     = true;
}

void scene_fade_in(Scene *self) {
  self->fade_state = FADE_IN;
  self->faded      = false;
  self->fading     = true;
}

void scene_change_scene(Scene *self, enum SceneState scene) {
  self->scene_state = scene;

  // TODO: Fix fadeing
  switch (scene) {
    case SCENE_MENU:
      LOG_DEBUG("Scene: Menu state");
      break;
    case SCENE_INTRO:
      scene_fade_in(self);
      LOG_DEBUG("Scene: Intro state");
      break;
    case SCENE_INGAME:
      scene_chunk_change(self, player_get_body(), CHUNK_SPAWN, SPAWN_COORD);
      LOG_DEBUG("Scene: Ingame state");
      break;
    case SCENE_ENDGAME:
      scene_fade_in(self);
      break;
  }
}  

void scene_dialog_attach(Scene *self, Dialog *dialog, char *tag) {
  if (self->dialog != NULL) {
    LOG_ERROR("Scene: Failed to attach dialog to scene, dialog isn't NULL");
    return;
  }

  self->dialog_tag = malloc(strlen(tag) + 1);
  strcpy(self->dialog_tag, tag);

  self->dialog    = dialog->contents;
  self->on_dialog = true;
}


void scene_dialog_next(Scene *self) {
  self->dialog = self->dialog->next;
  self->selected_answer = 0;

  if (!self->dialog)  {
    if (self->dialog_tag) {
      game_update_dialog_state(self->dialog_tag); 
    }

    self->dialog    = NULL;
    self->on_dialog = false;
    FREE(self->dialog_tag);

    LOG_DEBUG("Scene: Dialog ended");
  }
}

void scene_dialog_set(Scene *self, DialogNode *dialog) {
  self->dialog = dialog;

  // temp
  if (self->dialog_tag) { FREE(self->dialog_tag); } 

  LOG_DEBUG("Scene: Set Dialog");
}

void scene_dialog_end(Scene *self) {
    self->dialog    = NULL;
    self->on_dialog = false;
    FREE(self->dialog_tag);

    LOG_DEBUG("Scene: Dialog ended");
}

void scene_chunk_add_dialog(Scene *self, ChunkDialog dialog) {
  dialog.body_id = physics_static_body_create(global.physics, dialog.pos, dialog.size, COLLISION_LAYER_PLAYER, COLLISION_LAYER_DIALOG, global.dialog_callback);
  array_list_append(self->chunk_dialogs, &dialog);
}

ChunkRenderInfo* scene_get_chunk_render_info(void) {
  return (_curr_chunk) ? _curr_chunk->render_info : NULL;
}

void scene_chunk_add_prefab(char *name, vec2s coord) {
  ChunkPrefab prefab = { .coord = coord };
  strcpy(prefab.name, name);

  array_list_append(_curr_chunk->render_info->prefab, &prefab);
}

Chunk* scene_get_curr_chunk(void) {
  return (_curr_chunk) ? _curr_chunk : NULL;
}

DialogNode *scene_get_curr_dialog(Scene *self) {
  return self->dialog; 
}
