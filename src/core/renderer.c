#include "renderer.h"
#include "batch_render.h"
#include "../global.h"
#include "../defs.h"

static struct BatchRender *_render_batch = NULL;

void renderer_init(void) {
    _render_batch = batch_render_init();

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_FRONT_AND_BACK);
    glCullFace(GL_FRONT);
}

void renderer_destroy(void) {
    batch_render_destroy(_render_batch);
}

void renderer_prepare(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0,0.0,0.0,1.0);

    _render_batch->quad_count = 0;
}

void renderer_render(void) {

    // TODO: Make batch for line segment in this renderer and then add all body to it and then render after player
    // TODO: Render structure object in its own batch
    // TODO: Remove dialog_render and implement dialog rendering in this project

    { // get player information and then add to _render_batch to render at the end
        struct Spritesheet *player_spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_PLAYER);
        f32 tex_coord[4];
        player_get_tex_coord(tex_coord);
        batch_render_append_quad_texture(_render_batch, global.PlayerState.position, PLAYER_SIZE, WHITE, player_spritesheet->texture, tex_coord);
    }

    // render tilemap
    if (global.ChunkState.chunk) {
        chunk_render();
    }

    // render player & ...
    batch_render_render(_render_batch);
}
