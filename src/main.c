#include "core/asset_manager.h"
#include "core/camera.h"
#include "core/player.h"
#include "core/renderer.h"
#include "core/animation.h"
#include "core/chunk.h"
#include "core/physics.h"
#include "core/chunk_internal.h"

#include "util/debug.h"

#include "global.h"
#include "defs.h"

struct Global global;

static void border_collision(vec3s *a, vec2s size, vec2s start, vec2s end) {
    if (a->y < start.y) a->y = start.y;
    if (a->x < start.x) a->x = start.x;

    if (a->x + size.x > end.x) a->x = end.x - size.x;
    if (a->y + size.y > end.y) a->y = end.y - size.y;
}

static void input_handling(void) {
    player_input();

    if (glfwGetKey(global.window->handle, GLFW_KEY_X) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else if (glfwGetKey(global.window->handle, GLFW_KEY_Z) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void setup(void) {
    asset_manager_init(&global.asset_manager);

    asset_manager_push_shader(global.asset_manager, "default_shader", "res/shaders/default.vert", "res/shaders/default.frag");

    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_TESTING,  9, 3, 3, 16);
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_BASIC,   49, 7, 7, 16);
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_PLAYER,  32, 4, 8, 16);
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_NPC,     32, 4, 8, 16);

    camera_init(&global.camera, (vec3s){0,0,0}, (vec2s){100,100});
    physics_init();
    animation_init();
    player_init();
    renderer_init();
    chunk_init();

    { // create chunk suppose we load the chunk info from file
        Tile *tilemap = malloc(sizeof(*tilemap) * CHUNK_SIZE_X * CHUNK_SIZE_Y);

        for (u32 y = 0; y < CHUNK_SIZE_Y; y++) {
            for (u32 x = 0; x < CHUNK_SIZE_X; x++) {
                tilemap[y * CHUNK_SIZE_X + x].uvs       = map   [y * CHUNK_SIZE_X + x];
                tilemap[y * CHUNK_SIZE_X + x].action_id = action[y * CHUNK_SIZE_X + x];

                if (body[y * CHUNK_SIZE_X + x] == 1) {
                    tilemap[y * CHUNK_SIZE_X + x].body_id   = physics_body_create((vec2s){x*TILE_SIZE, y*TILE_SIZE}, DEFAULT_SCALE);
                }
                else {
                    tilemap[y * CHUNK_SIZE_X + x].body_id   = -1;
                }
            }
        }

        // create chunk of size 30x30 locate at pos 0x0
        Chunk *chunk = chunk_create((ivec2s){CHUNK_SIZE_X, CHUNK_SIZE_Y}, (vec2s){0,0}, tilemap);
        global.ChunkState.chunk = chunk;
        INFO_CHUNK(chunk);
        free(tilemap);
    }
}

void update(void) {
    input_handling();

    { // Camera
        camera_center_to_obj(global.camera, global.PlayerState.position, PLAYER_SIZE);
        border_collision(&global.camera->position, (vec2s){PROJECTION_WIDTH, PROJECTION_HEIGHT}, global.ChunkState.chunk->start_position, global.ChunkState.chunk->end_position);
        border_collision(&global.PlayerState.position, PLAYER_SIZE, global.ChunkState.chunk->start_position, global.ChunkState.chunk->end_position);

        camera_update(global.camera); 
    }

    physics_update(global.dt);
    animation_update(global.dt);

    renderer_prepare();
    renderer_render();
}

void cleanup(void) {
    camera_destroy(global.camera);
    asset_manager_destroy(global.asset_manager);
    animation_destroy();

    renderer_destroy();
    chunk_destroy();
}

int main(void) {
    struct Window window;

    if (window_init(&window, setup, update, cleanup)) {
        global.window = &window;
        window_loop(&window);
    }
    window_destroy(&window);
    return 0;
}
