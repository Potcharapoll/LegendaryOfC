#include "game.h"

#include "../engine/logger.h"
#include "../global.h"
#include "../defs.h"

#include "chunk.h"
#include "dialog.h"
#include "physics.h"
#include "player.h"
#include "scene.h"
#include "timer.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
  LAYER_BASE,
  LAYER_BASE_UPPER,
  LAYER_STRUCTURE,
  LAYER_TOP,

  LAYER_LAST
} RenderLayer;


static b8 update = false;
static b8 change = false;
static u32 question_flag = 0;

HASHTABLE_INIT(Dialog)

static hashtable_Dialog_t *test_dialog = NULL;
static QuadRenderer **renderer = NULL;

static void free_dialog(void *self) {
  Dialog *dialog = self;
  dialog_delete(dialog);
}

static int _get_act_question_number(enum GameAct act) {
  int num = -1;
  int total = 0;

  switch (act) {
    case GAME_ACT2:
      total = ACT2_TOTAL_QUESTION;
      break;
    case GAME_ACT3:
      total = ACT3_TOTAL_QUESTION;
      break;
    case GAME_ACT4:
      total = ACT4_TOTAL_QUESTION;
      break;
    default:
      break;
  }

  srand(time(NULL));

  int ran = (rand() % total) + 1;
  while (true) {
    if ((question_flag >> ran & 1) == 1) {
      ran = (rand() % total) + 1;
    }
    else {
      break;
    }
  }

  question_flag |= (1 << ran);
  num = ran;

  return num;
}

static void _load_dialog(enum GameAct act, Body *player_body) {
  if (test_dialog) { hashtable_Dialog_destroy(test_dialog); }
  test_dialog = hashtable_Dialog_init(free_dialog);

  switch (act) {
    case GAME_ACT1:
      {
        hashtable_Dialog_insert(test_dialog, "dialog_start",      dialog_load_from_file("../res/data/dialog/dialog_start"));
        hashtable_Dialog_insert(test_dialog, "dialog_locked",     dialog_load_from_file("../res/data/dialog/dialog_locked"));
        hashtable_Dialog_insert(test_dialog, "dialog_act1_emma",  dialog_load_from_file("../res/data/dialog/act1/dialog_act1_emma"));
        hashtable_Dialog_insert(test_dialog, "dialog_act1_image", dialog_load_from_file("../res/data/dialog/act1/dialog_act1_image"));
        hashtable_Dialog_insert(test_dialog, "dialog_act1a_roxy", dialog_load_from_file("../res/data/dialog/act1/dialog_act1a_roxy"));
        hashtable_Dialog_insert(test_dialog, "dialog_act1b_roxy", dialog_load_from_file("../res/data/dialog/act1/dialog_act1b_roxy"));
        break;
      }
    case GAME_ACT2:
      {
        hashtable_Dialog_insert(test_dialog, "dialog_locked",     dialog_load_from_file("../res/data/dialog/dialog_locked"));
        hashtable_Dialog_insert(test_dialog, "dialog_act2_roxy",    dialog_load_from_file("../res/data/dialog/act2/dialog_act2_roxy"));
        hashtable_Dialog_insert(test_dialog, "dialog_act2b_emma",   dialog_load_from_file("../res/data/dialog/act2/dialog_act2b_emma"));
        hashtable_Dialog_insert(test_dialog, "dialog_act2b_parmy",  dialog_load_from_file("../res/data/dialog/act2/dialog_act2b_parmy"));
        hashtable_Dialog_insert(test_dialog, "dialog_act2b_nathan", dialog_load_from_file("../res/data/dialog/act2/dialog_act2b_nathan"));


        char npc[3][10]  = {"emma","parmy","nathan"};
        char name[3][10] = {"Emma", "Parmy", "Nathan"};
        Dialog *question = NULL;

        char buf[100];
        for (u8 i = 0; i < 3; ++i) {
          snprintf(buf, sizeof(buf), "../res/data/dialog/act2/dialog_act2a_%s", npc[i]);
          question = dialog_load_from_file(buf);

          for (u8 j = 0; j < ACT2_QUESTION_COUNT; ++j) {
            snprintf(buf, sizeof(buf), "../res/data/question/act2/q%i", _get_act_question_number(GAME_ACT2));
            dialog_append_question_from_file(question, name[i], buf);
          }

          snprintf(buf, sizeof(buf), "dialog_act2a_%s", npc[i]);
          hashtable_Dialog_insert(test_dialog, buf,  question);
        }
        break;
      }
    case GAME_ACT3:
      {
        player_set_direction(RIGHT);
        game_change_chunk(player_body, CHUNK_INSIDE_OG_HOME, (vec2s){70.0f / TILE_SIZE, 121.0f / TILE_SIZE});

        hashtable_Dialog_insert(test_dialog, "dialog_locked",     dialog_load_from_file("../res/data/dialog/dialog_locked"));
        hashtable_Dialog_insert(test_dialog, "dialog_act3_roxy",   dialog_load_from_file("../res/data/dialog/act3/dialog_act3_roxy"));
        hashtable_Dialog_insert(test_dialog, "dialog_act3_emma",   dialog_load_from_file("../res/data/dialog/act3/dialog_act3_emma"));
        hashtable_Dialog_insert(test_dialog, "dialog_act3_parmy",  dialog_load_from_file("../res/data/dialog/act3/dialog_act3_parmy"));
        hashtable_Dialog_insert(test_dialog, "dialog_act3_nathan", dialog_load_from_file("../res/data/dialog/act3/dialog_act3_nathan"));

        Dialog *dialog = dialog_load_from_file("../res/data/dialog/act3/dialog_act3_tom"); 

        char buf[30];
        for (int j = 0; j < ACT3_QUESTION_COUNT; ++j) {
          snprintf(buf, sizeof(buf), "../res/data/question/act3/q%i", _get_act_question_number(GAME_ACT3));
          dialog_append_question_from_file(dialog, "Tom", buf);
        }

        hashtable_Dialog_insert(test_dialog, "dialog_act3_tom", dialog);
        break;
      }
    case GAME_ACT4:
      {
        player_set_direction(RIGHT);
        game_change_chunk(player_body, CHUNK_INSIDE_OG_HOME, (vec2s){70.0f / TILE_SIZE, 121.0f / TILE_SIZE});

        hashtable_Dialog_insert(test_dialog, "dialog_locked",     dialog_load_from_file("../res/data/dialog/dialog_locked"));
        hashtable_Dialog_insert(test_dialog, "dialog_act4_roxy",   dialog_load_from_file("../res/data/dialog/act4/dialog_act4_roxy"));
        hashtable_Dialog_insert(test_dialog, "dialog_act4_emma",   dialog_load_from_file("../res/data/dialog/act4/dialog_act4_emma"));
        hashtable_Dialog_insert(test_dialog, "dialog_act4_tom",   dialog_load_from_file("../res/data/dialog/act4/dialog_act4_tom"));
        hashtable_Dialog_insert(test_dialog, "dialog_act4_nathan", dialog_load_from_file("../res/data/dialog/act4/dialog_act4_nathan"));
        
        hashtable_Dialog_insert(test_dialog, "dialog_act4a_john",  dialog_load_from_file("../res/data/dialog/act4/dialog_act4a_john"));
        hashtable_Dialog_insert(test_dialog, "dialog_act4b_john",  dialog_load_from_file("../res/data/dialog/act4/dialog_act4b_john"));
        hashtable_Dialog_insert(test_dialog, "dialog_act4c_john",  dialog_load_from_file("../res/data/dialog/act4/dialog_act4c_john"));
        hashtable_Dialog_insert(test_dialog, "dialog_act4d_john",  dialog_load_from_file("../res/data/dialog/act4/dialog_act4d_john"));

        hashtable_Dialog_insert(test_dialog, "dialog_act4a_parmy", dialog_load_from_file("../res/data/dialog/act4/dialog_act4a_parmy"));
        hashtable_Dialog_insert(test_dialog, "dialog_act4c_parmy", dialog_load_from_file("../res/data/dialog/act4/dialog_act4c_parmy"));

        Dialog *d = dialog_load_from_file("../res/data/dialog/act4/dialog_act4b_parmy");
        char buf[60];

        for (u8 j = 0; j < ACT4_QUESTION_COUNT; ++j) {
          snprintf(buf, sizeof(buf), "../res/data/question/act4/q%i", _get_act_question_number(GAME_ACT4));
          dialog_append_question_from_file(d, "Parmy", buf);
        }

        hashtable_Dialog_insert(test_dialog, "dialog_act4b_parmy", d);
        break;
      }
  }

  LOG_DEBUG("Game: Dialog loaded");
}

void game_setup_act(enum GameAct act) {
  u32 lock = 0, chr = 0, state = 0;
  global.game_state_flag = 0;

  scene_collider_reset(global.scene);


  switch (act) {
    case GAME_ACT1:
      lock  = (GAME_STATE_LOCK_FISH | GAME_STATE_LOCK_CHURCH | GAME_STATE_LOCK_TUNNEL | GAME_STATE_LOCK_LIBRARY | GAME_STATE_LOCK_LJ_HOME | GAME_STATE_LOCK_OG_HOME | GAME_STATE_LOCK_VC_HOME);
      chr   = (GAME_STATE_SHOW_ROXY | GAME_STATE_SHOW_EMMA);
      state = GAME_STATE_ACT1;
      break;
    case GAME_ACT2:
      lock  = GAME_STATE_LOCK_TUNNEL | GAME_STATE_LOCK_VC_HOME;
      chr   = (GAME_STATE_SHOW_ROXY | GAME_STATE_SHOW_EMMA | GAME_STATE_SHOW_PARMY | GAME_STATE_SHOW_NATHAN);
      state = GAME_STATE_ACT2 | GAME_STATE_GET_MAN_PAGE;
      break;
    case GAME_ACT3:
      lock  = GAME_STATE_LOCK_TUNNEL | GAME_STATE_LOCK_VC_HOME | GAME_STATE_LOCK_FISH;
      chr   = (GAME_STATE_SHOW_ROXY | GAME_STATE_SHOW_EMMA | GAME_STATE_SHOW_PARMY | GAME_STATE_SHOW_NATHAN | GAME_STATE_SHOW_TOM);
      state = GAME_STATE_ACT3 | GAME_STATE_GET_MAN_PAGE;
      break;
    case GAME_ACT4:
      lock  = GAME_STATE_LOCK_TUNNEL;
      chr   = (GAME_STATE_SHOW_ROXY | GAME_STATE_SHOW_EMMA | GAME_STATE_SHOW_PARMY | GAME_STATE_SHOW_NATHAN | GAME_STATE_SHOW_TOM | GAME_STATE_SHOW_VC | GAME_STATE_SHOW_JOEY);
      state = GAME_STATE_ACT4 | GAME_STATE_GET_MAN_PAGE;
      break;
  }

  question_flag = 0;
  global.game_state_flag |= (lock | chr | state);

  _load_dialog(act, player_get_body());

  scene_fade_in(global.scene);
  LOG_DEBUG("Game: Setup Act%d", (act+1));
}

void game_update_dialog_state(char *tag) {
  if (game_state_check(GAME_STATE_ACT1)) {
    if (strcmp("dialog_act1a_roxy", tag) == 0) {
      game_state_on(ACT1_ROXY_TALKED | ACT1_OG_HOME_KEY);
      game_state_on(GAME_STATE_GET_MAN_PAGE);

      game_state_off(GAME_STATE_LOCK_OG_HOME);

      LOG_DEBUG("Game: Finish talking to Roxy at act1");
      LOG_DEBUG("Game: Get the key from Roxy on act1, unlock OG home");
    } 
    else if (strcmp("dialog_act1_image", tag) == 0) {
      game_state_on(ACT1_GET_G_IMAGE);

      scene_fade_out(global.scene);
      change = true;

      LOG_DEBUG("Game: Get G image");
    } 
  }
  else if (game_state_check(GAME_STATE_ACT2)) {
    if (strcmp("dialog_act2a_emma", tag) == 0) {
      game_state_on(ACT2_EMMA_TALKED);

      LOG_DEBUG("Game: Emma talked");
    } 
    else if (strcmp("dialog_act2a_parmy", tag) == 0) {
      game_state_on(ACT2_PARMY_TALKED);

      LOG_DEBUG("Game: Parmy talked");
    } 
    else if (strcmp("dialog_act2a_nathan", tag) == 0) {
      game_state_on(ACT2_NATHAN_TALKED);

      LOG_DEBUG("Game: Nathan talked");
    } 
  }
  else if (game_state_check(GAME_STATE_ACT3)) {
    if (strcmp("dialog_act3_tom", tag) == 0) {
      game_state_on(ACT3_TOM_TALKED);

      scene_fade_out(global.scene);
      change = true;
    } 
  }
  else if (game_state_check(GAME_STATE_ACT4)) {

    if (strcmp("dialog_act4a_john", tag) == 0) {
      game_state_on(ACT4_VC_QUEST);

      LOG_DEBUG("Game: VC Quest");
    }
    else if (strcmp("dialog_act4c_john", tag) == 0) {
      game_state_on(ACT4_TUNNEL_KEY);

      game_state_off(GAME_STATE_LOCK_TUNNEL);

      LOG_DEBUG("Game: Finish Quest and Unlock Tunnel");
    }
    else if (strcmp("dialog_act4b_parmy", tag) == 0) {
      game_state_on(ACT4_FISH_GET);

      LOG_DEBUG("Game: Get the fish");
    }
  }

  update = true;
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
  if (game_get_act() == GAME_ACT4 && global.timer->on_time) {
      scene_change_scene(global.scene, SCENE_ENDGAME);
      return;
  }

  if (change && global.scene->faded) { 
    game_setup_act(game_get_act() + 1); 
    change = false; 
  }

  if (!update) return;

  if (game_state_check(GAME_STATE_ACT2)) {
    if (game_state_check(ACT2_EMMA_TALKED | ACT2_NATHAN_TALKED | ACT2_PARMY_TALKED)) {
      scene_fade_out(global.scene);
      change = true;
    }
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
      if (strcmp(packet->dialog_tag, "parmy") == 0) {
        if (game_state_check(ACT2_PARMY_TALKED)) {
          snprintf(buf, 60, "dialog_act2b_parmy");
        }
        else {
          snprintf(buf, 60, "dialog_act2a_parmy");
        }
      }
      else if (strcmp(packet->dialog_tag, "nathan") == 0) {
        if (game_state_check(ACT2_NATHAN_TALKED)) {
          snprintf(buf, 60, "dialog_act2b_nathan");
        }
        else {
          snprintf(buf, 60, "dialog_act2a_nathan");
        }
      }
      else if (strcmp(packet->dialog_tag, "emma") == 0) {
        if (game_state_check(ACT2_EMMA_TALKED)) {
          snprintf(buf, 60, "dialog_act2b_emma");
        }
        else {
          snprintf(buf, 60, "dialog_act2a_emma");
        }
      }
      else {
        snprintf(buf, 60, "dialog_act2_%s",packet->dialog_tag);
      }
      break;
    case GAME_ACT3:
      snprintf(buf, 60, "dialog_act3_%s",packet->dialog_tag);
      break;
    case GAME_ACT4:
      if (strcmp(packet->dialog_tag, "parmy") == 0) {
        if (game_state_check(ACT4_VC_QUEST) && !game_state_check(ACT4_FISH_GET)) {
          snprintf(buf, 60, "dialog_act4b_parmy");
        }
        else {
          snprintf(buf, 60, "dialog_act4a_parmy");
        }
      }
      else if (strcmp(packet->dialog_tag, "john") == 0) {
        if (game_state_check(ACT4_TUNNEL_KEY)) {
          snprintf(buf, 60, "dialog_act4d_john");
        }
        else if (game_state_check(ACT4_FISH_GET)){
          snprintf(buf, 60, "dialog_act4c_john");
        }
        else if (game_state_check(ACT4_VC_QUEST)) {
          snprintf(buf, 60, "dialog_act4b_john");
        }
        else {
          snprintf(buf, 60, "dialog_act4a_john");
        }
      }
      else {
        snprintf(buf, 60, "dialog_act4_%s",packet->dialog_tag);
      }
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

    {
      if (global.scene->chunk_id == CHUNK_INSIDE_LIBRARY && game_get_act() == GAME_ACT2) {
        quad_renderer_append_quad(renderer[LAYER_STRUCTURE], (vec3s){52,133.8}, (vec2s){15,15}, (vec4s){0.25,0.25,0.25,1.0});
      }
    }

#if defined (DEBUG) && defined (DEBUG_ENABLE_IMGUI)
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
  scene_chunk_change(global.scene, player_body, chunk_id, target_coord);

  switch (chunk_id) {
    case CHUNK_SPAWN            :
      LOG_DEBUG("Game: Change to CHUNK_SPAWN");
      break;
    case CHUNK_VILLAGE_ENTRANCE :
      LOG_DEBUG("Game: Change to CHUNK_VILLAGE_ENTRANCE");
      break;
    case CHUNK_VILLAGE_LEFT     :
      LOG_DEBUG("Game: Change to CHUNK_LEFT");
      break;
    case CHUNK_VILLAGE_RIGHT    :
      LOG_DEBUG("Game: Change to CHUNK_RIGHT");
      break;
    case CHUNK_VILLAGE_TOP      :
      if (game_get_act() != GAME_ACT1) {
        scene_chunk_add_prefab("nathan_down", (vec2s){12.3,11});

        physics_static_body_create(global.physics, (vec2s){198,944} , (vec2s){14,21}, COLLISION_LAYER_PLAYER, COLLISION_LAYER_SOLID, NULL);
        scene_chunk_add_dialog(global.scene, (ChunkDialog){(vec2s){198,940}, (vec2s){14,4}, "nathan", .body_id = -1});
      }
      LOG_DEBUG("Game: Change to CHUNK_TOP");
      break;
    case CHUNK_VILLAGE_TOP_END  :
      timer_start(global.timer, 2.0f);
      LOG_DEBUG("Game: Change to CHUNK_TOP_END");
      break;
    case CHUNK_VILLAGE_TOP_LEFT :
      LOG_DEBUG("Game: Change to CHUNK_TOP_LEFT");
      break;
    case CHUNK_VILLAGE_TOP_RIGHT:
      if (game_get_act() == GAME_ACT4) {
        scene_chunk_add_prefab("john_down", (vec2s){15.5,6.5});

        physics_static_body_create(global.physics, (vec2s){729,872} , (vec2s){14,21}, COLLISION_LAYER_PLAYER, COLLISION_LAYER_SOLID, NULL);
        scene_chunk_add_dialog(global.scene, (ChunkDialog){(vec2s){729,868}, (vec2s){14,4}, "john", .body_id = -1});
      }
      LOG_DEBUG("Game: Change to CHUNK_TOP_RIGHT");
      break;
    case CHUNK_VILLAGE_TUNNEL   :
      LOG_DEBUG("Game: Change to CHUNK_TUNNEL");
      break;
    case CHUNK_INSIDE_LIBRARY   :
      if (game_get_act() == GAME_ACT3 || game_get_act() == GAME_ACT4) {
        scene_chunk_add_dialog(global.scene, (ChunkDialog){(vec2s){53,118}, (vec2s){10,6}, "tom", .body_id = -1});
      }
      LOG_DEBUG("Game: Change to CHUNK_INSIDE_LIBRARY");
      break;
    case CHUNK_INSIDE_RESTAURANT:
      LOG_DEBUG("Game: Change to CHUNK_INSIDE_RESTAURANT");
      break;
    case CHUNK_INSIDE_CHURCH    :
      LOG_DEBUG("Game: Change to CHUNK_INSIDE_CHURCH");
      break;
    case CHUNK_INSIDE_FISH      :
      LOG_DEBUG("Game: Change to CHUNK_INSIDE_FISH");
      break;
    case CHUNK_INSIDE_OG_HOME   :
      if (game_get_act() == GAME_ACT1) {
        scene_chunk_add_dialog(global.scene, (ChunkDialog){(vec2s){73,135}, (vec2s){13,4}, "image", .body_id = -1});

      }
      LOG_DEBUG("Game: Change to CHUNK_INSIDE_OG_HOME");
      break;
    case CHUNK_INSIDE_LJ_HOME   :
      LOG_DEBUG("Game: Change to CHUNK_INSIDE_LJ_HOME");
      break;
    case CHUNK_INSIDE_VC_HOME   : 
      LOG_DEBUG("Game: Change to CHUNK_INSIDE_VC_HOME");
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

  if (item == NULL) {
    LOG_ERROR("Game: Cannot load dialog tag %s", buf);
    return;
  }
  else {
    LOG_DEBUG("Game: Attach dialog tag %s", buf);
  }

  scene_dialog_attach(global.scene, item->value, buf);
}

DialogText* game_get_act_dialog(enum GameAct act) {
  DialogText *dialog = NULL;

  switch (act) {
    case GAME_ACT2:
      {
        static char state = 'a';

        char buf[50];
        snprintf(buf,sizeof(buf),"../res/data/dialog/act2/dialog_act2%c", state);
        dialog = dialog_load_text_from_file(buf);
        
        state++;
        break;
      }
    case GAME_ACT3:
      dialog = dialog_load_text_from_file("../res/data/dialog/act3/dialog_act3");
      break;
    case GAME_ACT4:
      dialog = dialog_load_text_from_file("../res/data/dialog/act4/dialog_act4");
      break;
    default:
      break;
  }

  return dialog;
}
