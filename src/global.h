#ifndef GLOBAL_H
#define GLOBAL_H

#pragma GCC diagnostic ignored "-Wmissing-braces"

#if defined (DEBUG) && defined (DEBUG_ENABLE_IMGUI)
#include "engine/editor.h"
#endif

#include "gfx/window.h"

#include "util/types.h"

#include "core/asset_manager.h"
#include "core/animation.h"
#include "core/physics.h"
#include "core/scene.h"
#include "core/timer.h"

#include <pthread.h>
#include <AL/alc.h>

enum CursorMode {
  CURSOR_MODE_START_POINT,
  CURSOR_MODE_END_POINT,
  CURSOR_MODE_NORMAL,

  CURSOR_MODE_LAST
};

enum Sound {
  SOUND_MAIN_MENU,
  SOUND_INGAME
};

struct Global {
  struct Window       *window;
  struct AssetManager *asset_manager;
  Scene               *scene;
  Physics             *physics;
  Animation           *animations;
  Timer               *timer;

  f32 dt;
  f32 input_delay;
  u32 game_state_flag;
  pthread_mutex_t lock;

  void   (*dialog_callback)(Static_Body* body, Body *other);
  void   (*teleporter_callback)(Static_Body* body, Body *other);
  ivec2s (*get_char_coord)(char c);

  struct {
    ALCdevice  *device;
    ALCcontext *context;
  } sound_ctx;
  enum Sound current_sound;

#ifdef DEBUG
  struct {
#ifdef DEBUG_ENABLE_IMGUI
    struct ImGui *editor;
    b8 toggle_editor;
    vec2 start_point, end_point;
    enum CursorMode cursor_mode;
#endif

    b8 toggle_collision;
    b8 toggle_show_collider;
  };
#endif
};

extern struct Global global;
#endif
