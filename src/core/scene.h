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
    MENU,
    INTRO,
    INGAME,
    ENDGAME,
};

typedef struct Scene {
    enum SceneState scene_state;
    Camera *camera;

    Chunks chunk_id; 
    Chunk *chunk;

    f32 fade_alpha;
    enum FadeState fade_state;
    b8 faded;

    // plan to use for filter for the night time 
    vec4s gradient;

    Dialog *dialog;
    b8 on_dialog;

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
void scene_change_chunk(Scene *self, Body *player_body, Chunks chunk_id, vec2s target_coord);

void scene_change_scene(Scene *self, enum SceneState scene);

void scene_attach_dialog(Scene *self, Dialog *dialog);

void scene_fade_reset(Scene *self);
void scene_fade_out(Scene *self);
void scene_fade_in(Scene *self);
#endif
