#ifndef SCNCE_H
#define SCNCE_H
#include "chunk.h"
#include "physics.h"
#include "camera.h"

enum FadeState {
    FADE_NONE,
    FADE_IN,
    FADE_OUT
};

enum ScnceState {
    MENU,
    INTRO,
    INGAME,
};

typedef struct Scnce {
    enum ScnceState scnce_state;
    Camera *camera;

    Chunks chunk_id; 
    Chunk *chunk;

    f32 fade_alpha;
    enum FadeState fade_state;
    b8 faded;
}Scnce;

Scnce* scnce_init(void);
void scnce_destroy(Scnce *self);

// We need player_body to set the camera position to center to the player
void scnce_update(Scnce *self, Body *player_body);

// So we've check the collision from callback function and if it is a teleporter will call 
// this function to change to change chunk by geting target chunk_id and target_coord.
void scnce_change_chunk(Scnce *self, Body *player_body, Chunks chunk_id, vec2s target_coord);

void scnce_change_scnce(Scnce *self, enum ScnceState scnce);

void scnce_fade_reset(Scnce *self);
void scnce_fade_out(Scnce *self);
void scnce_fade_in(Scnce *self);
#endif
