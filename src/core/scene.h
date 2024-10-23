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

typedef struct Scene {
    enum SceneState scene_state;
    Camera         *camera;

    Chunks      chunk_id; 
    array_list *chunk_colliders;
    array_list *chunk_teleporters;
    array_list *chunk_dialogs;

    enum FadeState fade_state;
    f32            fade_alpha;
    b8             faded; 
    b8             fading;

    // !!temp
    DialogNode *dialog;
    char       *dialog_tag;
    b8          on_dialog;
    u8          selected_answer;

    // for render text, dialog, and other things in the corresponding scene
    //
    // So we could have quad renderer to render everything, but it's fine 
    // to have quad renderer and text renderer separately I think?
    QuadRenderer *quad_renderer;
    TextRenderer *text_renderer;
}Scene;

Scene* scene_init(void);
void scene_destroy(Scene *self);

// We need player_body to set the camera position to center to the player
void scene_update(Scene *self, Body *player_body);
void scene_render(Scene *self);

// So we've check the collision from callback function and if it is a teleporter will call 
// this function to change to change chunk by geting target chunk_id and target_coord.

void scene_chunk_add_prefab(char *name, vec2s coord);
void scene_chunk_change(Scene *self, Body *player_body, Chunks chunk_id, vec2s target_coord);
void scene_chunk_add_dialog(Scene *self, ChunkDialog dialog);
void scene_change_scene(Scene *self, enum SceneState scene);

void scene_dialog_set(Scene *self, DialogNode *dialog);
void scene_dialog_attach(Scene *self, Dialog *dialog, char *tag);
void scene_dialog_end(Scene *self);
void scene_dialog_next(Scene *self);
DialogNode *scene_get_curr_dialog(Scene *self);

void scene_collider_reset(Scene *scene);

void scene_fade_reset(Scene *self);
void scene_fade_out(Scene *self);
void scene_fade_in(Scene *self);

ChunkRenderInfo* scene_get_chunk_render_info(void);
Chunk* scene_get_curr_chunk(void);
#endif
