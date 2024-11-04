#include "scene.h"
#include "chunk.h"
#include "dialog.h"
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
} DialogRenderPosition;
#define DIALOG_POSITION_LAST (DIALOG_POSITION_ANSWER4+1)

static vec3s dialog_render_position[DIALOG_POSITION_LAST] = {
  { 14      , 55,      0.0f},
  { 15      , 40,      0.0f},
  { 15      , 40 - 28, 0.0f},
  { 15      , 40 - 38, 0.0f},
  { 15 + 150, 40 - 28, 0.0f},
  { 15 + 150, 40 - 38, 0.0f},
};

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
    text_renderer_append_text(self->text_renderer, content->question, pos, size, DIALOG_TEXT_COLOR);

    for (u8 i = 0; i < 4; ++i) {
      pos = glms_vec3_add(camera_pos, dialog_render_position[DIALOG_POSITION_ANSWER1+i]);

      if (i == self->selected) {
        text_renderer_append_text(self->text_renderer, content->answer[i], pos, size, DIALOG_TEXT_SELECTED_COLOR);
      }
      else {
        text_renderer_append_text(self->text_renderer, content->answer[i], pos, size, DIALOG_TEXT_COLOR);
      }
    }
  }
  else {
    DialogText *content = curr->dialog;
    text_renderer_append_text(self->text_renderer, content->text, pos, size, DIALOG_TEXT_COLOR);
  }
}

void _scene_menu_render(Scene *self) {
  vec3s cam_pos = { self->camera->position.x, self->camera->position.y, 0.0f};

  const u8 FRAME_SIZE = 100;

  switch (self->menu_state) {
    case MENU_MAIN:
      {
        quad_renderer_append_quad(self->quad_renderer, (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - FRAME_SIZE/2.0f, cam_pos.y + PROJECTION_HEIGHT * 0.5 - FRAME_SIZE/2.0f},  (vec2s){FRAME_SIZE,FRAME_SIZE}, DIALOG_FRAME_COLOR);

        vec4s man_page_color = (game_state_check(GAME_STATE_GET_MAN_PAGE)) ? DIALOG_TEXT_COLOR : (vec4s){0.5,0.5,0.5,1};
        vec4s quest_color = WHITE;
        vec4s exit_color  = WHITE;
        switch (self->selected) {
          case MENU_CHOICE_QUEST:
            {
              quest_color = LIGHT_BLUE;   
              break;
            }
          case MENU_CHOICE_MAN_PAGE:
            {
              man_page_color = LIGHT_BLUE;
              break;
            }
          case MENU_CHOICE_EXIT:
            {
              exit_color = LIGHT_BLUE;   
              break;
            }
        }
        text_renderer_append_text(self->text_renderer, "Quest",    (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - (5*5*0.5), cam_pos.y + PROJECTION_HEIGHT * 0.5 - FRAME_SIZE/2.0f + 60}, 10, quest_color);
        text_renderer_append_text(self->text_renderer, "Man Page", (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - (8*5*0.5), cam_pos.y + PROJECTION_HEIGHT * 0.5 - FRAME_SIZE/2.0f + 40}, 10, man_page_color);
        text_renderer_append_text(self->text_renderer, "Exit",     (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - (4*5*0.5), cam_pos.y + PROJECTION_HEIGHT * 0.5 - FRAME_SIZE/2.0f + 20}, 10, exit_color);
        break;
      }
    case MENU_QUEST:
      {
        quad_renderer_append_quad(self->quad_renderer, (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - FRAME_SIZE/2.0f, cam_pos.y + PROJECTION_HEIGHT * 0.5 - FRAME_SIZE/2.0f},  (vec2s){FRAME_SIZE,FRAME_SIZE}, DIALOG_FRAME_COLOR);
        text_renderer_append_text(self->text_renderer, "Quest",    (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - (5*10*0.5), cam_pos.y + PROJECTION_HEIGHT * 0.5 - FRAME_SIZE/2.0f + 70}, 20, WHITE);

        // SUGGEST: May get the current quest from game module
        switch (game_get_act()) {
          case GAME_ACT1:
            {
              if (game_state_check(ACT1_ROXY_TALKED)) {
                text_renderer_append_text(self->text_renderer, "Go to Yellow house", 
                    (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - (18*3*0.5), cam_pos.y + PROJECTION_HEIGHT * 0.5 - FRAME_SIZE/2.0f + 45}, 6, LIGHT_BLUE);
                text_renderer_append_text(self->text_renderer, "(East side of the village)", 
                    (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - (26*3*0.5), cam_pos.y + PROJECTION_HEIGHT * 0.5 - FRAME_SIZE/2.0f + 39}, 6, LIGHT_BLUE);
              }
              else {
                text_renderer_append_text(self->text_renderer, "Investigate around", 
                    (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - (18*3*0.5), cam_pos.y + PROJECTION_HEIGHT * 0.5 - FRAME_SIZE/2.0f + 45}, 6, LIGHT_BLUE);
              }
              break;
            }
          case GAME_ACT2:
            {
              text_renderer_append_text(self->text_renderer, "Find Information about", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - (22*3*0.5), cam_pos.y + PROJECTION_HEIGHT * 0.5 - FRAME_SIZE/2.0f + 45}, 6, LIGHT_BLUE);
              text_renderer_append_text(self->text_renderer, "grandpa around village", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - (22*3*0.5), cam_pos.y + PROJECTION_HEIGHT * 0.5 - FRAME_SIZE/2.0f + 39}, 6, LIGHT_BLUE);
              break;
            }
          case GAME_ACT3:
            {
              text_renderer_append_text(self->text_renderer, "Go to Tom at Library", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - (20*3*0.5), cam_pos.y + PROJECTION_HEIGHT * 0.5 - FRAME_SIZE/2.0f + 45}, 6, LIGHT_BLUE);
              break;
            }
          case GAME_ACT4:
            {
              if (game_state_check(ACT4_TUNNEL_KEY)) {
                text_renderer_append_text(self->text_renderer, "Go through the tunnel", 
                    (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - (21*3*0.5), cam_pos.y + PROJECTION_HEIGHT * 0.5 - FRAME_SIZE/2.0f + 45}, 6, LIGHT_BLUE);
              }
              else if (game_state_check(ACT4_FISH_GET)) {
                text_renderer_append_text(self->text_renderer, "Give the fish to Village chief", 
                    (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - (30*3*0.5), cam_pos.y + PROJECTION_HEIGHT * 0.5 - FRAME_SIZE/2.0f + 45}, 6, LIGHT_BLUE);
              }
              else if (game_state_check(ACT4_VC_QUEST)) {
                text_renderer_append_text(self->text_renderer, "Get the fish from Parmy", 
                    (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - (23*3*0.5), cam_pos.y + PROJECTION_HEIGHT * 0.5 - FRAME_SIZE/2.0f + 45}, 6, LIGHT_BLUE);
                text_renderer_append_text(self->text_renderer, "(Fish Shop)", 
                    (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - (11*3*0.5), cam_pos.y + PROJECTION_HEIGHT * 0.5 - FRAME_SIZE/2.0f + 39}, 6, LIGHT_BLUE);
              }
              else {
                text_renderer_append_text(self->text_renderer, "Go to Village Chief Home", 
                    (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - (24*3*0.5), cam_pos.y + PROJECTION_HEIGHT * 0.5 - FRAME_SIZE/2.0f + 45}, 6, LIGHT_BLUE);
                text_renderer_append_text(self->text_renderer, "(Northeast of the village)", 
                    (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - (26*3*0.5), cam_pos.y + PROJECTION_HEIGHT * 0.5 - FRAME_SIZE/2.0f + 39}, 6, LIGHT_BLUE);
              }
              break;
            }
        }
        break;
      }
    case MENU_MAN_PAGE:
      {
        // SUGGEST: Change from manual to load from file

        const u8 SIZE = 200;

        quad_renderer_append_quad(self->quad_renderer, (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - SIZE/2.0f, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f},  (vec2s){SIZE,SIZE}, DIALOG_FRAME_COLOR);
        text_renderer_append_text(self->text_renderer, "Man Page", (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - (8*5*0.5), cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 180}, 10, WHITE);

        char page[30];
        snprintf(page, sizeof(page), "%d/%d", self->man_page+1, MAN_PAGE6+1);
        text_renderer_append_text(self->text_renderer, page, (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 + 80, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 190}, 8, WHITE);


        switch (self->man_page) {
          case MAN_PAGE1:
            {
              text_renderer_append_text(self->text_renderer, "Data Type", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 160}, 8, (vec4s){0.6,0.6,0.6,1.0});

              text_renderer_append_text(self->text_renderer, "int: Represents integers (whole numbers). The size typically is 4 bytes,|but this can vary depending on the system.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 152}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "int", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 152}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "float: Represents single-precision floating-point numbers.|It usually takes up to 4 bytes.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 137}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "float", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 137}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "double: Represents double-precision floating-point numbers. It usually|takes up 8 bytes and provides more precision than float.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 122}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "double", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 122}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "char: Represents a single character. It usually occupies 1 byte|and can hold ASCII values.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 107}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "char", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 107}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "short: Typically a smaller version of int, usually 2 bytes.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 92}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "short", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 92}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "long: Typically a larger version of int, usually 4 or 8 bytes,|depending on the system.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 82}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "long", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 82}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "unsigned: Used with integer types to represent only non-negative values.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 67}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "unsigned", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 67}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "signed: The default for integer types; explicitly indicates|a signed type.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 57}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "signed", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 57}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "void: Represents the absence of a value. Used in functions that do not|return a value or pointers that do not have a specific type.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 42}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "void", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 42}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "Structures (struct): A user-defined type that groups|different types together.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 27}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "Structures (struct)", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 27}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "Unions (union): Similar to structures, but can hold only one of its|non-static data members at a time, saving memory.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 12}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "Unions (union)", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 12}, 5, LIGHT_BLUE);
              break;
            }

          case MAN_PAGE2:
            {
              text_renderer_append_text(self->text_renderer, "Enumerations (enum): A user-defined type consisting of a set of|named integer constants.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 160}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "Enumerations (enum)", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 160}, 5, LIGHT_BLUE);



              text_renderer_append_text(self->text_renderer, "Keywords", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 135}, 8, (vec4s){0.6,0.6,0.6,1.0});

              text_renderer_append_text(self->text_renderer, "auto: Indicates automatic storage duration. It is rarely used since|local variables are automatically auto by default.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 127}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "auto", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 127}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "break: Exits a loop or switch statement prematurely.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 112}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "break", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 112}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "case: Defines a branch in a switch statement.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 102}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "case", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 102}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "const: Declares a variable whose value cannot be changed|after initialization.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 92}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "const", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 92}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "continue: Skips the current iteration of a loop and proceeds to the|next iteration.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 77}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "continue", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 77}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "default: Specifies the default case in a switch statement if no cases match.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 62}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "default", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 62}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "do: Used in a do-while loop to execute a block of code at least once.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 52}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "do", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 52}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "else: Specifies the block of code to execute if the condition in an if|statement is false.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 42}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "else", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 42}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "extern: Declares a variable or function that is defined in another|file or module.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 27}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "extern", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 27}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "for: Starts a for loop, which is used for iterating a set number of times.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 12}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "for", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 12}, 5, LIGHT_BLUE);
              break;
            }

          case MAN_PAGE3:
            {
              text_renderer_append_text(self->text_renderer, "goto: Transfers control to a labeled statement. It is generally|discouraged due to potential confusion in code flow.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 160}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "goto", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 160}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "if: Starts a conditional statement to execute code based on whether a|condition is true.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 145}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "if", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 145}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "register: Suggests to the compiler that a variable should be stored in|a CPU register for faster access.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 130}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "register", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 130}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "return: Exits a function and optionally returns a value.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 115}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "return", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 115}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "signed: Specifies that a variable can hold both negative and positive|values.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 105}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "signed", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 105}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "sizeof: Returns the size (in bytes) of a data type or variable.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 90}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "sizeof", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 90}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "static: Declares a variable with static storage duration, retaining its|value between function calls.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 80}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 80}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "switch: Starts a switch statement for multi-way branching.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 65}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "switch", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 65}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "typedef: Creates a new name (alias) for an existing data type.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 55}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "typedef", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 55}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "void: Indicates no value or type; used for functions that do not|return a value.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 45}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "void", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 45}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "volatile: Indicates that a variable may change unexpectedly|(used in multithreading or hardware interactions).", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 30}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "volatile", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 30}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "while: Starts a while loop, which executes a block of code as long as|a condition is true.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 15}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "while", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 15}, 5, LIGHT_BLUE);
              break;
            }

          case MAN_PAGE4:
            {
              text_renderer_append_text(self->text_renderer, "Syntax", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 160}, 8, (vec4s){0.6,0.6,0.6,1.0});

              text_renderer_append_text(self->text_renderer, "Variable Declaration", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 150}, 5, LIGHT_BLUE);
              text_renderer_append_text(self->text_renderer, "int age;|float height = 5.9f;|float x = 1.0, y = 2.0;", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 142}, 5, WHITE);


              text_renderer_append_text(self->text_renderer, "Function Declaration (Prototype)", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 120}, 5, LIGHT_BLUE);
              text_renderer_append_text(self->text_renderer, "returnType functionName(parameterType1 param1, parameterType2 param2);", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 112}, 5, WHITE);


              text_renderer_append_text(self->text_renderer, "If-Else Statement", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 100}, 5, LIGHT_BLUE);
              text_renderer_append_text(self->text_renderer, "if (condition) { // Code to execute if condition is true }|else { // Code to execute if condition is false }",
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 92}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "if (condition1) { // Code for condition1 }|else if (condition2) { // Code for condition2 }|else { // Code if none of the above conditions are true } ",
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 72}, 5, WHITE);


              text_renderer_append_text(self->text_renderer, "While Loop", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 46}, 5, LIGHT_BLUE);
              text_renderer_append_text(self->text_renderer, "while (condition) { // Code to execute repeatedly }", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 38}, 5, WHITE);


              break;
            }

          case MAN_PAGE5:
            {
              text_renderer_append_text(self->text_renderer, "Switch Case", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 160}, 5, LIGHT_BLUE);
              text_renderer_append_text(self->text_renderer, "switch (expression) {|case constant1: // Code to execute if expression equals constant1 break;|case constant2: // Code to execute if expression equals constant2 break;|default: // Code to execute if expression doesn't match any case break;|}", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 152}, 5, WHITE);


              text_renderer_append_text(self->text_renderer, "Functions", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 110}, 8, (vec4s){0.6,0.6,0.6,1.0});

              text_renderer_append_text(self->text_renderer, "printf: Outputs formatted text to the standard output.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 102}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "printf", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 102}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "sprintf: Similar to printf, but instead of printing to the console,|it writes formatted data to a string (buffer).", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 92}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "sprintf", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 92}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "snprintf: A safer version of sprintf, it limits the number of|characters written to the buffer to prevent buffer overflows.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 77}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "snprintf", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 77}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "scanf: Reads formatted input from the standard input.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 62}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "scanf", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 62}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "sscanf: Similar to scanf, but it reads formatted data from a|string instead of standard input.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 52}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "sscanf", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 52}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "gets: Reads a line from standard input into a string until a newline|or EOF is encountered.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 37}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "gets", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 37}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "getchar: Reads the next character from the standard input and returns it.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 22}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "getchar", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 22}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "puts: Outputs a string to the standard output followed by a newline|character.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 12}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "puts", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 12}, 5, LIGHT_BLUE);
              break;
            }

          case MAN_PAGE6:
            {
              text_renderer_append_text(self->text_renderer, "putchar: Outputs a single character to the standard output.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 160}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "putchar", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 160}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "rand: Returns a pseudo-random integer in the range of 0 to RAND_MAX.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 150}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "rand", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 150}, 5, LIGHT_BLUE);

              text_renderer_append_text(self->text_renderer, "srand: Seeds the random number generator used by rand.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 140}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "srand", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 140}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "abort: Causes the program to terminate abnormally. It does not return|a status code but can generate a core dump.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 130}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "abort", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 130}, 5, LIGHT_BLUE);


              text_renderer_append_text(self->text_renderer, "exit: Terminates the program, optionally returning a status code to the|operating system.", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 115}, 5, WHITE);
              text_renderer_append_text(self->text_renderer, "exit", 
                  (vec3s){cam_pos.x + PROJECTION_WIDTH * 0.5 - 90, cam_pos.y + PROJECTION_HEIGHT * 0.5 - SIZE/2.0f + 115}, 5, LIGHT_BLUE);
              break;
            }
        }

        break;
      }
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
  scene->on_menu           = false;
  scene->menu_state        = MENU_MAIN;
  scene->man_page          = 0;
  scene->quad_renderer     = quad_renderer_init();
  scene->text_renderer     = text_renderer_init(global.get_char_coord);
  scene->dialog            = NULL;
  scene->on_dialog         = false;
  scene->selected          = 0;
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

void scene_update(Scene *self, Body *player_body) {
  _fade_update(self);

  if (self->scene_state == SCENE_INGAME) {
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
      {
        text_renderer_append_text(self->text_renderer, "LEGENDARY OF C" ,            (vec3s){PROJECTION_WIDTH*0.5 - (14*15*0.5), 100}, 30, LIGHT_BLUE);
        text_renderer_append_text(self->text_renderer, "Press SPACE to start game" , (vec3s){PROJECTION_WIDTH*0.5 - (25*3.5*0.5), 50}, 7, WHITE);
      }
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
      else if (self->on_menu) {
        _scene_menu_render(self);
      }
      break;
    case SCENE_ENDGAME:
      {
        vec2s cam_pos = self->camera->position;
        text_renderer_append_text(self->text_renderer, "Congreatulation!" ,             (vec3s){cam_pos.x + PROJECTION_WIDTH*0.5 - (16*7.5*0.5), cam_pos.y + 120}, 15, YELLOW);
        text_renderer_append_text(self->text_renderer, "You've cleared the game." ,     (vec3s){cam_pos.x + PROJECTION_WIDTH*0.5 - (24*7.5*0.5), cam_pos.y + 100}, 15, YELLOW);
        text_renderer_append_text(self->text_renderer, "Press SPACE to exit the game" , (vec3s){cam_pos.x + PROJECTION_WIDTH*0.5 - (28*3.5*0.5), cam_pos.y +  50},  7, WHITE);
        break;
      }
    default:
      break;
  }

  quad_renderer_append_quad(self->quad_renderer, (vec3s){global.scene->camera->position.x,global.scene->camera->position.y,0.0f}, (vec2s){WIDTH, HEIGHT}, (vec4s){0,0,0, self->fade_alpha});

  if (self->dialog != NULL || self->on_menu) {
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
      LOG_DEBUG("Scene: Engame state");
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
  self->selected = 0;

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
