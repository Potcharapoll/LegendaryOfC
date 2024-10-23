#include "game.h"

#include "../engine/logger.h"
#include "../global.h"
#include "../defs.h"

#include "dialog.h"
#include "scene.h"

#include <string.h>

typedef enum {
  LAYER_BASE,
  LAYER_BASE_UPPER,
  LAYER_STRUCTURE,
  LAYER_TOP,

  LAYER_LAST
} RenderLayer;


static b8 update = false;
static b8 change = false;

HASHTABLE_INIT(Dialog)

static hashtable_Dialog_t *test_dialog = NULL;
static QuadRenderer **renderer = NULL;

static void free_dialog(void *self) {
  Dialog *dialog = self;
  dialog_delete(dialog);
}

static void _load_dialog(enum GameAct act) {
  if (test_dialog) { hashtable_Dialog_destroy(test_dialog); }
  test_dialog = hashtable_Dialog_init(free_dialog);

  switch (act) {
    case GAME_ACT1:
      {
        hashtable_Dialog_insert(test_dialog, "dialog_start",  dialog_load_from_file("res/data/dialog/dialog_start"));
        hashtable_Dialog_insert(test_dialog, "dialog_locked", dialog_load_from_file("res/data/dialog/dialog_locked"));
        hashtable_Dialog_insert(test_dialog, "dialog_act1a_roxy", dialog_load_from_file("res/data/dialog/dialog_act1a_roxy"));
        hashtable_Dialog_insert(test_dialog, "dialog_act1b_roxy", dialog_load_from_file("res/data/dialog/dialog_act1b_roxy"));
        hashtable_Dialog_insert(test_dialog, "dialog_act1_emma", dialog_load_from_file("res/data/dialog/dialog_act1_emma"));
        hashtable_Dialog_insert(test_dialog, "dialog_act1_image", dialog_load_from_file("res/data/dialog/dialog_act1_image"));
        break;
      }
    case GAME_ACT2:
      {
        //
        DialogQuestion **questions = NULL;

        questions = malloc(sizeof(questions) * 10);
        for (u8 i = 1; i <= 10; ++i) {
          char buf[50];
          snprintf(buf, 50, "res/data/question/act2/q%d",i);

          questions[i] = dialog_load_question_from_file(buf);
        }

        Dialog *d = dialog_load_from_file("res/data/dialog/dialog_act2_roxy");
        hashtable_Dialog_insert(test_dialog, "dialog_act2_roxy",  d);

        d = dialog_load_from_file("res/data/dialog/dialog_act2_emma");
        dialog_append(d, "Emma", DIALOG_TYPE_QUESTION, questions[0]);
        dialog_append(d, "Emma", DIALOG_TYPE_QUESTION, questions[1]);
        hashtable_Dialog_insert(test_dialog, "dialog_act2_emma",  d);

        d = dialog_load_from_file("res/data/dialog/dialog_act2_parmy");
        dialog_append(d, "Emma", DIALOG_TYPE_QUESTION, questions[2]);
        dialog_append(d, "Emma", DIALOG_TYPE_QUESTION, questions[3]);
        hashtable_Dialog_insert(test_dialog, "dialog_act2_parmy", d);

        break;
      }
    case GAME_ACT3:
      break;
    case GAME_ACT4:
      break;
  }

  /* for (int i = 0; i < HT_CAPACITY; i++) { */
  /*   entry_t *curr = global.dialogs->entries[i]; */

  /*   while (curr != NULL) { */
  /*     entry_t *tmp = curr; */
  /*     curr = curr->next; */

  /*     Dialog *d = tmp->value; */
  /*     dialog_list(d); */
  /*   } */
  /* } */

  LOG_DEBUG("Game: Dialog loaded");
}

void game_setup_act(enum GameAct act) {
  u32 lock = 0, chr = 0, state = 0;
  global.game_state_flag = 0;

  scene_reset_collider(global.scene);

  switch (act) {
    case GAME_ACT1:
      lock  = (GAME_STATE_LOCK_FISH | GAME_STATE_LOCK_CHURCH | GAME_STATE_LOCK_TUNNEL | GAME_STATE_LOCK_LIBRARY | GAME_STATE_LOCK_LJ_HOME | GAME_STATE_LOCK_OG_HOME | GAME_STATE_LOCK_VC_HOME);
      chr   = (GAME_STATE_SHOW_ROXY | GAME_STATE_SHOW_EMMA);
      state = GAME_STATE_ACT1;
      break;
    case GAME_ACT2:
      lock  = GAME_STATE_LOCK_TUNNEL;
      chr   = (GAME_STATE_SHOW_ROXY | GAME_STATE_SHOW_EMMA | GAME_STATE_SHOW_PARMY | GAME_STATE_SHOW_NATHAN);
      state = GAME_STATE_ACT2;
      break;
    case GAME_ACT3:
      lock  = GAME_STATE_LOCK_TUNNEL;
      chr   = (GAME_STATE_SHOW_ROXY | GAME_STATE_SHOW_EMMA | GAME_STATE_SHOW_PARMY | GAME_STATE_SHOW_NATHAN | GAME_STATE_SHOW_TOM);
      state = GAME_STATE_ACT3;
      break;
    case GAME_ACT4:
      lock  = GAME_STATE_LOCK_TUNNEL;
      chr   = (GAME_STATE_SHOW_ROXY | GAME_STATE_SHOW_EMMA | GAME_STATE_SHOW_PARMY | GAME_STATE_SHOW_NATHAN | GAME_STATE_SHOW_TOM | GAME_STATE_SHOW_VC | GAME_STATE_SHOW_JOEY);
      state = GAME_STATE_ACT4;
      break;
  }

  global.game_state_flag |= (lock | chr | state);
  _load_dialog(act);

  scene_fade_in(global.scene);
  LOG_DEBUG("Game: Setup Act");
}

void game_update_dialog_state(char *tag) {
  if (game_state_check(GAME_STATE_ACT1)) {
    if (strcmp("dialog_act1a_roxy", tag) == 0) {
      game_state_toggle(ACT1_ROXY_TALKED);
      game_state_toggle(ACT1_OG_HOME_KEY);
    } 
    if (strcmp("dialog_act1_image", tag) == 0) {
      game_state_toggle(ACT1_GET_G_IMAGE);
    } 
  }

  LOG_DEBUG("Game: Update Dialog State (tag: %s)", tag);
}

void game_init(void) {
  global.game_state_flag = 0;

  renderer = malloc(LAYER_LAST * sizeof(renderer));
  ASSERT(renderer != NULL, "Game: Cannot allocate memory for renderer", __FILE__, __LINE__);

  for (u8 i = 0; i < LAYER_LAST; ++i) {
    renderer[i] = quad_renderer_init();
  }
}

void game_destroy(void) {
  for (u8 i = 0; i < LAYER_LAST; ++i) {
    quad_renderer_destroy(renderer[i]);
  }
  FREE(renderer);

  if (test_dialog) { hashtable_Dialog_destroy(test_dialog); }
}

void game_update(void) {
  if (change && global.scene->faded) { game_setup_act(game_get_act() + 1); change = false; }
  if (!update) return;

  if (game_state_check(GAME_STATE_ACT1)) {
    if (game_state_check(ACT1_ROXY_TALKED)) {
      LOG_DEBUG("Game: Finish talking to Roxy at act1");
    }

    if (game_state_check(ACT1_OG_HOME_KEY)) {
      game_state_toggle(GAME_STATE_LOCK_OG_HOME);
      LOG_DEBUG("Game: Get the key from Roxy at act1, unlock OG home");
    }

    if (game_state_check(ACT1_GET_G_IMAGE)) {
      scene_fade_out(global.scene);
      change = true;
      LOG_DEBUG("Game: Get G image");
    }
  }
  else if (game_state_check(GAME_STATE_ACT2)) {
  }
  else if (game_state_check(GAME_STATE_ACT3)) {
  }
  else if (game_state_check(GAME_STATE_ACT4)) {
  }

  update = false;
}

u8 game_get_act(void) {
  u8 act = 0;

  if ((global.game_state_flag & GAME_STATE_ACT1) == GAME_STATE_ACT1)      act = GAME_ACT1;
  else if ((global.game_state_flag & GAME_STATE_ACT2) == GAME_STATE_ACT2) act = GAME_ACT2;
  else if ((global.game_state_flag & GAME_STATE_ACT3) == GAME_STATE_ACT3) act = GAME_ACT3;
  else if ((global.game_state_flag & GAME_STATE_ACT4) == GAME_STATE_ACT4) act = GAME_ACT4;
  else LOG_ERROR("Game: Invalid Game Act");

  return act;
}

void game_state_toggle(u32 flag) {
  global.game_state_flag ^= flag;
  update = true;
}

void game_state_on(u32 flag) {
  global.game_state_flag |= flag;
}

void game_state_off(u32 flag) {
  global.game_state_flag &= ~flag;
}

b8 game_state_check(u32 flag) {
  return ((global.game_state_flag & flag) == flag);
}

void game_get_dialog_tag(char buf[static 60], DialogPacket *packet) {
  u8 act = game_get_act();

  switch (act) {
    case GAME_ACT1:
      if (strcmp(packet->dialog_tag, "roxy") == 0) {
          if (game_state_check(ACT1_ROXY_TALKED)) {
            snprintf(buf, 60, "dialog_act1b_roxy");
          }
          else {
            snprintf(buf, 60, "dialog_act1a_roxy");
          }
      }
      else {
        snprintf(buf, 60, "dialog_act1_%s",packet->dialog_tag);
      }
      break;
    case GAME_ACT2:
      snprintf(buf, 60, "dialog_act2_%s",packet->dialog_tag);
      break;
    case GAME_ACT3:
      snprintf(buf, 60, "dialog_act3_%s",packet->dialog_tag);
      break;
    case GAME_ACT4:
      snprintf(buf, 60, "dialog_act4_%s",packet->dialog_tag);
      break;
  }
}

// get curr chunk from scene to render
void game_render(Body *player_body) {
  ChunkRenderInfo *render_info = scene_get_chunk_render_info();
  if(render_info == NULL) return;

  { // render
    struct Spritesheet *spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_TILE);
    for (s32 y = 0; y < CHUNK_SIZE_Y; y++) {
      for (s32 x = 0; x < CHUNK_SIZE_X; x++) {
        u32 uv = render_info->uv[CHUNK_SIZE_X * y + x]; 

        if (uv == (u32)-1) { continue; }

        u32 row    = uv / spritesheet->grid_size.x;
        u32 col    = uv % spritesheet->grid_size.x;
        f32 cellx  = spritesheet->cell_size.x / spritesheet->size.x;
        f32 celly  = spritesheet->cell_size.y / spritesheet->size.y; 

        f32 tex_coord[4] = {
          (cellx * col), 
          (cellx * col) + cellx, 
          (celly * row), 
          (celly * row) + celly
        };
        vec3s position   = {
          render_info->position.x + x * TILE_SIZE, 
          render_info->position.y + y * TILE_SIZE, 
          0.0
        };
        quad_renderer_append_quad_texture(renderer[LAYER_BASE], position, DEFAULT_SCALE, WHITE, spritesheet->texture, tex_coord);
      }
    }

    u32 offset = CHUNK_SIZE_Y * CHUNK_SIZE_X;
    for (s32 y = 0; y < CHUNK_SIZE_Y; y++) {
      for (s32 x = 0; x < CHUNK_SIZE_X; x++) {
        u32 uv = render_info->uv[offset + CHUNK_SIZE_X * y + x]; 

        if (uv == 0) continue;

        u32 row    = uv / spritesheet->grid_size.x;
        u32 col    = uv % spritesheet->grid_size.x;
        f32 cellx  = spritesheet->cell_size.x / spritesheet->size.x;
        f32 celly  = spritesheet->cell_size.y / spritesheet->size.y; 

        f32 tex_coord[4] = {
          (cellx * col), 
          (cellx * col) + cellx, 
          (celly * row), 
          (celly * row) + celly
        };
        vec3s position   = {render_info->position.x + x * TILE_SIZE, render_info->position.y + y * TILE_SIZE, 0.0};
        quad_renderer_append_quad_texture(renderer[LAYER_BASE_UPPER], position, DEFAULT_SCALE, WHITE, spritesheet->texture, tex_coord);
      }
    }

    // prefab
    for (u8 i = 0; i < render_info->prefab->len; ++i) {
      ChunkPrefab *prefab = array_list_get(render_info->prefab, i);
      quad_renderer_append_prefab(renderer[LAYER_STRUCTURE], prefab->coord, prefab->name);
    }

    { // append player to LAYER_TOP
      struct Spritesheet *player_spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_PLAYER);
      f32 tex_coord[4];

      player_get_tex_coord(tex_coord);
      quad_renderer_append_quad_texture(renderer[LAYER_TOP], (vec3s){player_body->position.x,player_body->position.y, 0.0f}, 
          PLAYER_SIZE, WHITE, player_spritesheet->texture, tex_coord);
    }

    if (game_state_check(GAME_STATE_SHOW_INTERACT) && !global.scene->on_dialog) {
      struct Texture *tex = asset_manager_get_texture(global.asset_manager, TEXTURE_INTERACT);

      vec3s pos = {
        .x = global.scene->camera->position.x + PROJECTION_WIDTH/2.0f - tex->size.y/4.0f,
        .y = global.scene->camera->position.y + 5,
      };
      quad_renderer_append_quad_texture(renderer[LAYER_TOP], pos, (vec2s){tex->size.x/4.0f, tex->size.y/4.0f}, WHITE, *tex, NULL);

      game_state_off(GAME_STATE_SHOW_INTERACT);
    }

#ifdef DEBUG
    { // append start_point, end_point, and cursor to LAYER_TOP
      quad_renderer_append_quad(renderer[LAYER_TOP], (vec3s){global.start_point[0], global.start_point[1], 0.0f}, (vec2s){1,1}, GREEN);
      quad_renderer_append_quad(renderer[LAYER_TOP], (vec3s){global.end_point[0], global.end_point[1], 0.0f}, (vec2s){1,1}, BLUE);
      quad_renderer_append_quad(renderer[LAYER_TOP], (vec3s){global.window->mouse.orthox, global.window->mouse.orthoy, 0.0f}, (vec2s){1,1}, WHITE);
    }
#endif

  }

  for (u8 i = 0; i < LAYER_LAST; ++i) { quad_renderer_render(renderer[i]); }
}

void game_change_chunk(Body *player_body, Chunks chunk_id, vec2s target_coord) {
  scene_change_chunk(global.scene, player_body, chunk_id, target_coord);

  switch (chunk_id) {
    case CHUNK_SPAWN            :
      LOG_DEBUG("Change to CHUNK_SPAWN");
      break;
    case CHUNK_VILLAGE_ENTRANCE :
      LOG_DEBUG("Change to CHUNK_VILLAGE_ENTRANCE");
      break;
    case CHUNK_VILLAGE_LEFT     :
      LOG_DEBUG("Change to CHUNK_LEFT");
      break;
    case CHUNK_VILLAGE_RIGHT    :
      LOG_DEBUG("Change to CHUNK_RIGHT");
      break;
    case CHUNK_VILLAGE_TOP      :
      LOG_DEBUG("Change to CHUNK_TOP");
      break;
    case CHUNK_VILLAGE_TOP_END  :
      LOG_DEBUG("Change to CHUNK_TOP_END");
      break;
    case CHUNK_VILLAGE_TOP_LEFT :
      LOG_DEBUG("Change to CHUNK_TOP_LEFT");
      break;
    case CHUNK_VILLAGE_TOP_RIGHT:
      LOG_DEBUG("Change to CHUNK_TOP_RIGHT");
      break;
    case CHUNK_VILLAGE_TUNNEL   :
      LOG_DEBUG("Change to CHUNK_TUNNEL");
      break;
    case CHUNK_INSIDE_LIBRARY   :
      LOG_DEBUG("Change to CHUNK_INSIDE_LIBRARY");
      break;
    case CHUNK_INSIDE_RESTAURANT:
      LOG_DEBUG("Change to CHUNK_INSIDE_RESTAURANT");
      break;
    case CHUNK_INSIDE_CHURCH    :
      LOG_DEBUG("Change to CHUNK_INSIDE_CHURCH");
      break;
    case CHUNK_INSIDE_FISH      :
      LOG_DEBUG("Change to CHUNK_INSIDE_FISH");
      break;
    case CHUNK_INSIDE_OG_HOME   :
      if (game_get_act() == GAME_ACT1) {
        scene_add_chunk_dialog(global.scene, (ChunkDialog){(vec2s){73,135}, (vec2s){13,4}, "image", .body_id = -1});
      }
      LOG_DEBUG("Change to CHUNK_INSIDE_OG_HOME");
      break;
    case CHUNK_INSIDE_LJ_HOME   :
      LOG_DEBUG("Change to CHUNK_INSIDE_LJ_HOME");
      break;
    case CHUNK_INSIDE_VC_HOME   : 
      LOG_DEBUG("Change to CHUNK_INSIDE_VC_HOME");
      break;
  }
}

void game_attach_dialog(DialogPacket *packet) {
  char buf[60];
  const entry_Dialog_t *item = NULL;

  if (packet->append_act) {
    game_get_dialog_tag(buf, packet);
  } else {
    strcpy(buf, packet->dialog_tag);
  }

  item = hashtable_Dialog_search(test_dialog, buf);
  LOG_DEBUG("Dialog tag: %s", buf);

  scene_attach_dialog(global.scene, item->value, buf);
}
