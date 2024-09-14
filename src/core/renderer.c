#include "renderer.h"
#include "batch_render.h"
#include "animation.h"
#include "../global.h"
#include "../defs.h"
#include "chunk.h"
#include "physics.h"

static struct BatchRender *_render_batch   = NULL;
static struct LineBatchRender *_line_batch = NULL;

// Store ChunkIds and get chunk when need to render
static u64 *CHUNKS;

// MARK: All line render is for Debugging 
static void _set_chunk(Chunks chunkId) {
    global.ChunkState.chunk_id = CHUNKS[chunkId];
    Body *player_body = physics_body_get(global.PlayerState.body_id);
    Chunk *chunk      = chunk_get(global.ChunkState.chunk_id);

    global.ChunkState.chunk = chunk;
    player_body->position   = (vec2s){chunk->position.x + chunk->spawn.x * TILE_SIZE, chunk->position.y + chunk->spawn.y * TILE_SIZE};
}

void renderer_append_line_segment(vec2s a, vec2s b, vec4s color) {
    line_batch_render_append_line(_line_batch, a, b, color);
}

void renderer_append_quad_line(vec2s position, vec2s size, vec4s color) {
    vec2s points[] = {
        {position.x - size.x, position.y - size.y},   
        {position.x + size.x, position.y - size.y},   
        {position.x + size.x, position.y + size.y},   
        {position.x - size.x, position.y + size.y},   
    };

    renderer_append_line_segment(points[0], points[1], color);
    renderer_append_line_segment(points[1], points[2], color);
    renderer_append_line_segment(points[2], points[3], color);
    renderer_append_line_segment(points[3], points[0], color);
}

void renderer_init(void) {
    // load resources
    { 
        asset_manager_push_shader(global.asset_manager, "default_shader", "res/shaders/default.vert", "res/shaders/default.frag");
        asset_manager_push_shader(global.asset_manager, "line_shader", "res/shaders/line.vert", "res/shaders/line.frag");

        asset_manager_push_spritesheet(global.asset_manager, TEXTURE_TESTING,  9, 3, 3, 16);
        asset_manager_push_spritesheet(global.asset_manager, TEXTURE_BASIC,   49, 7, 7, 16);
        asset_manager_push_spritesheet(global.asset_manager, TEXTURE_NPC,     32, 4, 8, 16);
        asset_manager_push_spritesheet(global.asset_manager, TEXTURE_PLAYER,  32, 4, 8, 16);
    }

    camera_init(&global.camera, (vec2s){0,0});
    physics_init();
    animation_init();
    player_init();
    chunk_init();

    _render_batch = batch_render_init();
    _line_batch   = line_batch_render_init();

    CHUNKS = malloc(sizeof(CHUNKS) * CHUNK_LAST);

    // load from file
    CHUNKS[CHUNK_SPAWN]   = chunk_load_from_file("res/data/chunk_home");
    CHUNKS[CHUNK_VILLAGE] = chunk_load_from_file("res/data/chunk_village");

    _set_chunk(CHUNK_SPAWN);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_FRONT_AND_BACK);
    glCullFace(GL_FRONT);
}

void renderer_destroy(void) {
    camera_destroy(global.camera);
    physics_destroy();
    animation_destroy();
    chunk_destroy();

    batch_render_destroy(_render_batch);
    line_batch_render_destroy(_line_batch);

    free(CHUNKS);
}

void renderer_prepare(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0,0.0,0.0,1.0);

    _render_batch->quad_count = 0;
    _line_batch->line_count   = 0;
}

void renderer_render(void) {
    // TODO: Render structure object in its own batch
    // TODO: Remove dialog_render and implement dialog rendering in this project
    // TODO: We need to make a collider box from the center or the render object, so we need to fix the spritesheet to fit this 
    
    { // get player information and then add to _render_batch to render at the end
        Body *player_body = physics_body_get(global.PlayerState.body_id);
        struct Spritesheet *player_spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_PLAYER);
        f32 tex_coord[4];

        player_get_tex_coord(tex_coord);
        batch_render_append_quad_texture(_render_batch, (vec3s){player_body->position.x,player_body->position.y, 0.0f}, PLAYER_SIZE, WHITE, player_spritesheet->texture, tex_coord);
    }

    if (global.ChunkState.chunk) { chunk_render(); }

    physics_render_collider();
    batch_render_render(_render_batch);
    line_batch_render_render(_line_batch);
}

void renderer_append_aabb(AABB aabb, vec4s color) {
        renderer_append_quad_line(aabb.center, aabb.half_size, color);
}
