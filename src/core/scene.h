#ifndef SCNCE_H
#define SCNCE_H
#include "renderer.h"
#include "physics.h"
#include "camera.h"
#include "dialog.h"
#include "chunk.h"

enum FadeState {
    FADE_NONE,
    FADE_IN,
    FADE_OUT
};

enum SceneState {
    SCENE_MENU,
    SCENE_INTRO,
    SCENE_INGAME,
    SCENE_ENDGAME,
};

typedef enum {
  MENU_MAIN,
  MENU_QUEST,
  MENU_MAN_PAGE
} Menu;

typedef enum {
  MENU_CHOICE_QUEST,
  MENU_CHOICE_MAN_PAGE,
  MENU_CHOICE_EXIT 
} MenuChoice;

typedef enum {
  MAN_PAGE1,
  MAN_PAGE2,
  MAN_PAGE3,
  MAN_PAGE4,
  MAN_PAGE5,
  MAN_PAGE6,
} MAN_PAGE;

typedef struct Scene {
    QuadRenderer *quad_renderer;
    TextRenderer *text_renderer;

    Camera *camera;

    enum SceneState scene_state;
    enum FadeState fade_state;
    Menu menu_state;

    Chunks chunk_id; 
    Chunk *chunk;

    // !!temp
    DialogNode *dialog;
    char *dialog_tag;
    f32 fade_alpha;

    u8 selected;
    u8 man_page;

    b8 faded; 
    b8 fading;
    b8 on_menu;
    b8 on_dialog;
}Scene;

Scene* scene_init(void);
void scene_destroy(Scene *self);

// We need player_body to set the camera position to center to the player
void scene_update(Scene *self, Body *player_body);
void scene_render(Scene *self);

// So we've check the collision from callback function and if it is a teleporter will call 
// this function to change to change chunk by geting target chunk_id and target_coord.

void scene_chunk_add_prefab(Scene *self, char *name, vec2s coord);
void scene_chunk_change(Scene *self, Body *player_body, Chunks chunk_id, vec2s target_coord);
void scene_chunk_add_dialog(Scene *self, ChunkDialog dialog);
void scene_change_scene(Scene *self, enum SceneState scene);

void scene_dialog_set(Scene *self, DialogNode *dialog);
void scene_dialog_attach(Scene *self, Dialog *dialog, char *tag);
void scene_dialog_end(Scene *self);
void scene_dialog_next(Scene *self);
DialogNode *scene_get_curr_dialog(Scene *self);

void scene_reset(Scene *scene);

void scene_fade_reset(Scene *self);
void scene_fade_out(Scene *self);
void scene_fade_in(Scene *self);
#endif
