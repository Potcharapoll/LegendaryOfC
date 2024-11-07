#include "engine/logger.h"

#include "core/asset_manager.h"
#include "core/prefab.h"
#include "core/scene.h"
#include "core/game.h"

#include "gfx/window.h"

#include "global.h"
#include "defs.h"

#include <string.h>
#include <assert.h>

static b8           collide_dialog = false;
static DialogPacket *dialog_packet = NULL;

#ifdef DEBUG
LineRenderer *line_renderer;

static void _append_collider(void) {
  Body *body; 
  for (u32 i = 0; i < global.physics->body_list->len; i++) {
    body = physics_body_get(global.physics, i);
    line_renderer_append_aabb(line_renderer, body->aabb, GREEN);
  }

  Static_Body *static_body; 
  for (u32 i = 0; i < global.physics->static_body_list->len; i++) {
    static_body = physics_static_body_get(global.physics, i);

    if (static_body->collision_flag & COLLISION_LAYER_SOLID) {
      line_renderer_append_aabb(line_renderer, static_body->aabb, WHITE);
    }
    else if (static_body->collision_flag & COLLISION_LAYER_TELEPORTER) {
      line_renderer_append_aabb(line_renderer, static_body->aabb, BLACK);
    }
    else if (static_body->collision_flag & COLLISION_LAYER_DIALOG) {
      line_renderer_append_aabb(line_renderer, static_body->aabb, BLUE);
    }
  }
}
#endif


static ivec2s _get_char_coord(char c) {
  static char text_index[4][26] = {
    "/?0123456789              ",
    "!@#$%^&*()_+-={}[]:\";'<>,.",
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
    "abcdefghijklmnopqrstuvwxyz",
  };

  ivec2s result = {0};

  for (u8 y = 0; y < 4; ++y) {
    for (u8 x = 0; x < 26; ++x) {
      if (c == text_index[y][x]) {
        result.x = x;
        result.y = y;
        break;
      }
    }
  }

  return result;
}

static b8 _teleporter_check(char tag) {
  b8 status = false;

  switch (tag) {
    case TAG_FISH:
      status = game_state_check(GAME_STATE_LOCK_FISH);
      break;
    case TAG_CHURCH:
      status = game_state_check(GAME_STATE_LOCK_CHURCH);
      break;
    case TAG_TUNNEL:
      status = game_state_check(GAME_STATE_LOCK_TUNNEL);
      break;
    case TAG_LIBRARY:
      status = game_state_check(GAME_STATE_LOCK_LIBRARY);
      break;
    case TAG_LJ_HOME:
      status = game_state_check(GAME_STATE_LOCK_LJ_HOME);
      break;
    case TAG_VC_HOME:
      status = game_state_check(GAME_STATE_LOCK_VC_HOME);
      break;
    case TAG_OG_HOME:
      status = game_state_check(GAME_STATE_LOCK_OG_HOME);
      break;
    case TAG_RESTAURANT:
      status = game_state_check(GAME_STATE_LOCK_RESTAURANT);
    case TAG_NORMAL:
      break;
  }

  return (!status);
}

static void _dialog_callback(Static_Body *body, Body *other) {
  if (global.scene->chunk_dialogs == NULL) {
    LOG_ERROR("Cannot run _dialog_callback due to chunk_dialogs is NULL");
    return;
  }

  collide_dialog = true;
  array_list *dialog_list = global.scene->chunk_dialogs;

  for (u8 i = 0; i < dialog_list->len; ++i) {

    ChunkDialog *dialog = array_list_get(dialog_list, i);
    assert(dialog != NULL);

    Static_Body *dialog_body = physics_static_body_get(global.physics, dialog->body_id);
    assert(dialog_body != NULL);

    if (body == dialog_body && dialog_packet == NULL) {
      dialog_packet = dialog_packet_create(dialog->tag, true); 
      break;
    }
  }

  game_state_on(GAME_STATE_SHOW_INTERACT);
}

static void _teleporter_callback(Static_Body *body, Body *other) {

  if (global.scene->chunk_teleporters == NULL) {
    LOG_ERROR("Cannot run _teleporter_callback due to chunk_teleporters is NULL");
    return;
  }

  array_list *teleporter_list = global.scene->chunk_teleporters;

  for (u32 i = 0; i < teleporter_list->len; ++i) {

    ChunkTeleporter *teleporter = array_list_get(teleporter_list, i);
    assert(teleporter != NULL);

    Static_Body *teleporter_body = physics_static_body_get(global.physics, teleporter->body_id);
    assert(teleporter_body != NULL);

    if (body == teleporter_body) {
      other->velocity = glms_vec2_zero();
      player_set_animation(IDLE, player_get_direction());


      if (_teleporter_check(teleporter->tag)) { 
        if (global.scene->fade_state == FADE_NONE) scene_fade_out(global.scene);

        if (global.scene->faded) {
          game_change_chunk(other, teleporter->chunkId, teleporter->target_coord);
          break;
        }
      }
      else {
        vec2s pv;
        AABB minkowski = aabb_minkowski_diff(body->aabb, other->aabb);
        aabb_penetration_vector(&pv, minkowski);

        other->position.x += pv.x;
        other->position.y += pv.y;

        if (!dialog_packet) { dialog_packet = dialog_packet_create("dialog_locked", false); }
      }
    }
  }
}

static void _load_prefab(void) {
  struct Spritesheet *structures_spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_STRUCTURES);
  prefab_create("bus_station", structures_spritesheet, WHITE, (vec2s){48,48},  (vec4s){12,12,15,15});
  prefab_create("bus_stop",    structures_spritesheet, WHITE, (vec2s){16,32},  (vec4s){15,12,16,14});
  prefab_create("restaurant",  structures_spritesheet, WHITE, (vec2s){112,96}, (vec4s){ 6, 0,13, 6});
  prefab_create("library",     structures_spritesheet, WHITE, (vec2s){160,80}, (vec4s){ 0,18,10,23});
  prefab_create("church",      structures_spritesheet, WHITE, (vec2s){80,96},  (vec4s){12,15,17,21});
  prefab_create("fish_shop",   structures_spritesheet, WHITE, (vec2s){80,64},  (vec4s){13, 0,18, 4});
  prefab_create("slider",      structures_spritesheet, WHITE, (vec2s){80,64},  (vec4s){13, 4,18, 8});
  prefab_create("horse1",      structures_spritesheet, WHITE, (vec2s){32,32},  (vec4s){14, 8,16,10});
  prefab_create("horse2",      structures_spritesheet, WHITE, (vec2s){32,32},  (vec4s){16, 8,18,10});
  prefab_create("nathan_home", structures_spritesheet, WHITE, (vec2s){96,96},  (vec4s){ 6,12,12,18});
  prefab_create("orange_home", structures_spritesheet, WHITE, (vec2s){96,96},  (vec4s){ 0, 0, 6, 6});
  prefab_create("old_g_home",  structures_spritesheet, WHITE, (vec2s){96,96},  (vec4s){ 0, 6, 6,12});
  prefab_create("g_home",      structures_spritesheet, WHITE, (vec2s){96,96},  (vec4s){ 0,12, 6,18});
  prefab_create("vc_home",     structures_spritesheet, WHITE, (vec2s){96,96},  (vec4s){ 6, 6,12,12});
  prefab_create("sign_down",   structures_spritesheet, WHITE, (vec2s){48,32},  (vec4s){12,10,15,12});
  prefab_create("sign_left",   structures_spritesheet, WHITE, (vec2s){16,48},  (vec4s){17,12,18,15});
  prefab_create("plant_pot",   structures_spritesheet, WHITE, (vec2s){32,32},  (vec4s){12, 8,14,10});
  prefab_create("image",       structures_spritesheet, WHITE, (vec2s){16,16},  (vec4s){16,12,17,13});

  struct Spritesheet *inside_spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_INSIDE);
  prefab_create("inside_library",    inside_spritesheet, WHITE, (vec2s){352,192}, (vec4s){0,0,23,13});
  prefab_create("inside_restaurant", inside_spritesheet, WHITE, (vec2s){352,176}, (vec4s){0,13,23,24});
  prefab_create("inside_church",     inside_spritesheet, WHITE, (vec2s){192,336}, (vec4s){23,0,36,22});
  prefab_create("inside_fish",       inside_spritesheet, WHITE, (vec2s){160,144}, (vec4s){36,0,47,10});
  prefab_create("inside_lj_home",    inside_spritesheet, WHITE, (vec2s){224,192}, (vec4s){0,24,14,36});
  prefab_create("inside_vc_home",    inside_spritesheet, WHITE, (vec2s){224,192}, (vec4s){14,24,28,36});
  prefab_create("inside_og_home",    inside_spritesheet, WHITE, (vec2s){224,192}, (vec4s){28,24,42,36});

  struct Spritesheet *npcs = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_NPC);
  prefab_create("nathan_down", npcs, WHITE, (vec2s){16,23}, (vec4s){3,2,4,3});
  prefab_create("john_down",   npcs, WHITE, (vec2s){16,23}, (vec4s){5,2,6,3});
}

static void input_handling(Body *player_body) {
  global.input_delay += global.dt;

  if (global.input_delay >= INPUT_DELAY) {

    switch (global.scene->scene_state) {
      case SCENE_MENU:
      case SCENE_INTRO:
        if (window_get_key(global.window, GLFW_KEY_SPACE) && !global.scene->fading) {
            scene_fade_out(global.scene);

            global.input_delay = 0.0f;
        }
        break;
      case SCENE_INGAME:
        if (global.scene->fade_state == FADE_NONE && !global.scene->on_dialog && !global.scene->on_menu) {
          player_input();

          if (window_get_key(global.window, GLFW_KEY_ESCAPE)) {
            player_body->velocity = glms_vec2_zero();
            player_set_animation(IDLE, player_get_direction());

            global.scene->selected = MENU_CHOICE_QUEST;
            global.scene->on_menu  = true;

            LOG_DEBUG("Q Pressed");
            global.input_delay = 0.0f;
          }
          else if (window_get_key(global.window, GLFW_KEY_E)) {
            if (dialog_packet) {
              player_body->velocity = glms_vec2_zero();
              player_set_animation(IDLE, player_get_direction());

              game_attach_dialog(dialog_packet);
            }

            LOG_DEBUG("E Pressed");
            global.input_delay = 0.0f;
          }

#ifdef DEBUG
          if (window_get_key(global.window, GLFW_KEY_X)) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            global.input_delay = 0.0f;
          }
          else if (window_get_key(global.window, GLFW_KEY_Z)) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            global.input_delay = 0.0f;
          }

          if (window_get_key(global.window, GLFW_KEY_I)) {
            global.toggle_collision = !global.toggle_collision;
            global.input_delay = 0.0f;
          }
          if (window_get_key(global.window, GLFW_KEY_U)) {
            global.toggle_show_collider = !global.toggle_show_collider;
            global.input_delay = 0.0f;
          }
          if (window_get_key(global.window, GLFW_KEY_Y)) {
            global.toggle_editor = !global.toggle_editor;
            global.input_delay = 0.0f;
          }

          if (window_get_mouse_button(global.window, GLFW_MOUSE_BUTTON_LEFT)) {
            switch (global.cursor_mode) {
              case CURSOR_MODE_START_POINT:
                global.start_point[0] = global.window->mouse.orthox;
                global.start_point[1] = global.window->mouse.orthoy;
                break;
              case CURSOR_MODE_END_POINT:
                global.end_point[0] = global.window->mouse.orthox;
                global.end_point[1] = global.window->mouse.orthoy;
                break;
              default:
                break;
            }
            global.input_delay = 0.0f;
          }
          if (window_get_mouse_button(global.window, GLFW_MOUSE_BUTTON_RIGHT)) {
            global.cursor_mode = (global.cursor_mode + 1) % CURSOR_MODE_LAST;
            global.input_delay = 0.0f;
          }
#endif
        } else if (global.scene->on_dialog) {
          dialog_input();
        }
        else if (global.scene->on_menu) {

          if (window_get_key(global.window, GLFW_KEY_ESCAPE)) {
            global.scene->menu_state = MENU_MAIN;
            global.scene->on_menu    = false;
            global.scene->selected   = 0;

            global.input_delay = 0.0f;
          }
          if (window_get_key(global.window, GLFW_KEY_SPACE)) {
            switch (global.scene->selected) {
              case MENU_CHOICE_QUEST:
                global.scene->menu_state = MENU_QUEST;
                break;
              case MENU_CHOICE_MAN_PAGE:
                global.scene->menu_state = MENU_MAN_PAGE;
                break;
              case MENU_CHOICE_EXIT:
                window_trigger_close();
                break;
            }
          }

          switch (global.scene->menu_state) {
            case MENU_QUEST:
              break;
            case MENU_MAIN:
              {
                if (window_get_key(global.window, GLFW_KEY_S)) {
                  global.scene->selected = (global.scene->selected < MENU_CHOICE_EXIT) ? global.scene->selected + 1 : global.scene->selected;
                  if (global.scene->selected == MENU_CHOICE_MAN_PAGE && !game_state_check(GAME_STATE_GET_MAN_PAGE)) global.scene->selected++;

                  global.input_delay = 0.0f;
                }
                else if (window_get_key(global.window, GLFW_KEY_W)) {
                  global.scene->selected = (global.scene->selected > MENU_CHOICE_QUEST) ? global.scene->selected - 1 : global.scene->selected;
                  if (global.scene->selected == MENU_CHOICE_MAN_PAGE && !game_state_check(GAME_STATE_GET_MAN_PAGE)) global.scene->selected--;
                  global.input_delay = 0.0f;
                }
                break;
              }
            case MENU_MAN_PAGE:
              {
                if (window_get_key(global.window, GLFW_KEY_D)) {
                  global.scene->man_page = (global.scene->man_page < MAN_PAGE6) ? global.scene->man_page + 1 : global.scene->man_page;
                  global.input_delay = 0.0f;
                }
                else if (window_get_key(global.window, GLFW_KEY_A)) {
                  global.scene->man_page = (global.scene->man_page > MAN_PAGE1) ? global.scene->man_page - 1 : global.scene->man_page;
                  global.input_delay = 0.0f;
                }
                break;
              }
          }
        }
        break;
      case SCENE_ENDGAME:
        if (window_get_key(global.window, GLFW_KEY_SPACE)) {
          window_trigger_close();          
        }
        break;
      default:
        break;
    }
  }
}

void setup(void) {
  pthread_mutex_init(&global.lock, NULL);
  global.asset_manager = asset_manager_init();

  asset_manager_push_shader(global.asset_manager, "default_shader", "../res/shaders/default.vert", "../res/shaders/default.frag");
  asset_manager_push_shader(global.asset_manager, "texture_shader", "../res/shaders/texture.vert", "../res/shaders/texture.frag");
  asset_manager_push_texture(global.asset_manager, TEXTURE_INTERACT, TEXTURE_INTERACT);
  asset_manager_push_spritesheet(global.asset_manager, TEXTURE_TEXT,         81, (ivec2s){26, 4}, (ivec2s){32,32});
  asset_manager_push_spritesheet(global.asset_manager, TEXTURE_PLAYER,       32, (ivec2s){ 8, 4}, (ivec2s){16,22});
  asset_manager_push_spritesheet(global.asset_manager, TEXTURE_NPC,          21, (ivec2s){ 7, 3}, (ivec2s){16,23});
  asset_manager_push_spritesheet(global.asset_manager, TEXTURE_TILE,         56, (ivec2s){ 8, 7}, (ivec2s){16,16});
  asset_manager_push_spritesheet(global.asset_manager, TEXTURE_INSIDE,     1692, (ivec2s){47,36}, (ivec2s){16,16});
  asset_manager_push_spritesheet(global.asset_manager, TEXTURE_STRUCTURES,  368, (ivec2s){18,23}, (ivec2s){16,16});

  global.teleporter_callback = _teleporter_callback;
  global.dialog_callback     = _dialog_callback;
  global.get_char_coord      = _get_char_coord;
  global.timer               = timer_init();
  global.physics             = physics_init(10);
  global.animations          = animation_init();
  global.scene               = scene_init();


  prefab_init();
  _load_prefab();

  player_init();

  game_init();

#ifdef DEBUG 
  global.cursor_mode = CURSOR_MODE_NORMAL;
  glm_vec2_zero(global.start_point);
  glm_vec2_zero(global.end_point);

  asset_manager_push_shader(global.asset_manager, "line_shader", "../res/shaders/line.vert", "../res/shaders/line.frag");
  line_renderer = line_renderer_init();
  editor_init();
#endif

  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  scene_change_scene(global.scene, SCENE_MENU);
}

void update(void) {
  glClear(GL_COLOR_BUFFER_BIT);
  glClearColor(0.0,0.0,0.0,1.0);

  Body *player_body = player_get_body();

  input_handling(player_body);

  scene_update(global.scene, player_body);
  timer_update(global.timer);

  switch (global.scene->scene_state) {
    case SCENE_MENU:
      LOG_DEBUG("IN MENU");
      if (global.scene->faded) { 
        scene_change_scene(global.scene, SCENE_INTRO); 
      }
      break;
    case SCENE_INTRO:
      if (!global.scene->faded && !global.timer->busy) {
        timer_start(global.timer, 5.0f);
      } 

      if (global.timer->on_time) {
        scene_fade_out(global.scene);
      }

      if (global.scene->faded && global.timer->busy) {
        timer_reset(global.timer);
        scene_change_scene(global.scene, SCENE_INGAME);

        game_setup_act(GAME_ACT1);
      }
      break;
    case SCENE_INGAME:
      game_update();

      if (!collide_dialog && dialog_packet) { dialog_packet_free(&dialog_packet); }
      collide_dialog = false;

      animation_update(global.animations, global.dt);
      physics_update(global.physics, global.dt);
      game_render(player_body);
      break;
    default:
      break;
  }

  scene_render(global.scene);

#ifdef DEBUG
  if (global.toggle_show_collider) _append_collider();
  line_renderer_render(line_renderer);

  if (global.toggle_editor) editor_render();
#endif
}

void cleanup(void) {
  pthread_mutex_destroy(&global.lock);
  timer_destroy(global.timer);
  physics_destroy(global.physics);
  animation_destroy(global.animations);
  scene_destroy(global.scene);
  prefab_destroy();
  game_destroy();

  asset_manager_destroy(global.asset_manager);

#ifdef DEBUG
  line_renderer_destroy(line_renderer);
  editor_destroy();
#endif

  LOG_TRACE("Window: Cleaning up");
}

int main(void) {
#ifdef DEBUG
  LOG_INFO("%s Version %.1f DEBUG Mode", TITLE, VERSION);
#else
  LOG_INFO("%s Version %.1f Release Mode", TITLE, VERSION);
#endif

  struct Window window = {0};

  window_init(&window, setup, update, cleanup);

  global.window = &window;
  window_loop(&window);

  window_destroy(&window);
  return 0;
}
