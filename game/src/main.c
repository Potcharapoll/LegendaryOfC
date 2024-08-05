#include "cglm/io.h"
#include "gfx/window.h"
#include "core/renderer.h"
#include "core/camera.h"
#include "core/asset_manager.h"
#include "defs.h"

AssetManager *asset_manager = NULL;
Camera *camera = NULL;
struct Window window;

u32 rows = HEIGHT/32;
u32 cols = WIDTH/32;

typedef struct {
    vec2s size;
    vec3s pos;
    vec4s color;
} Entity;

typedef struct {
    u64 uid;
    u32 rows;
    u32 cols;
    vec2s start_position;
    vec2s end_position;
    Entity *entities;
} Chunk;

Entity player;

#define CHUNK_SIZE 60
Chunk *c1, *c2;
Chunk *chunk_list[2];
u64 curr_chunk_idx = 0;
Chunk *curr_chunk = NULL;
b8 next_chunk = false;

f32 rand_float(void) {
    return (f32)rand() / RAND_MAX;
}

static void border_collision(vec3s *a, vec2s size, vec2s start, vec2s end) {
    if (a->y < start.y) a->y = start.y;
    if (a->x < start.x) a->x = start.x;

    if (a->x + size.x > end.x) a->x = end.x - size.x;
    if (a->y + size.y > end.y) a->y = end.y - size.y;
}


static void camera_center_to_entity(vec3s *a, vec3s entity_pos, vec2s entity_size) {
    a->x = entity_pos.x - PROJECTION_WIDTH /2.0f - entity_size.x / 2.0f;
    a->y = entity_pos.y - PROJECTION_HEIGHT/2.0f - entity_size.y / 2.0f;
}

static void chunk_init(Chunk *chunk, vec4s color) {
    static u64 _id = 0;

    chunk->uid = _id++;
    f32 _x = chunk->start_position.x;
    f32 _y = chunk->start_position.y;
    for (int y = 0; y < CHUNK_SIZE; y++) {
        for (int x = 0; x < CHUNK_SIZE; x++) {
            if ((int)_x % (32*CHUNK_SIZE) == 0) { _x = chunk->start_position.x; }

            chunk->entities[y * 60 + x].color = color;
            /* chunk->entities[y * 60 + x].color = (vec4s){rand_float(), rand_float(), rand_float(), 1.0}; */
            chunk->entities[y * 60 + x].size  = DEFAULT_SIZE;
            chunk->entities[y * 60 + x].pos   = (vec3s){_x, _y, 0};
            _x += 32;
        }
        _y += 32;
    }
    chunk->end_position.x = _x;
    chunk->end_position.y = _y;
}

void setup(void) {
    asset_manager_init(&asset_manager);

    camera_init(&camera, (vec3s){0,0,0}, (vec2s){100,100});

    renderer_init(camera, asset_manager);

    player = (Entity){DEFAULT_SIZE, (vec3s){32*60/2.0f,32*60/2.0f,0.0f}, BLACK};

    c1 = malloc(sizeof(*c1));
    c1->cols = 60;
    c1->rows = 60;
    c1->start_position = (vec2s){0,0};
    c1->entities = malloc((c1->rows * c1->cols) * sizeof(Entity));
    chunk_init(c1, GREEN);

    c2 = malloc(sizeof(*c2));
    c2->cols = 60;
    c2->rows = 60;
    c2->start_position = (vec2s){c1->end_position.x, 0};
    c2->entities = malloc((c2->rows * c2->cols) * sizeof(Entity));
    chunk_init(c2, RED);

    chunk_list[0] = c1;
    chunk_list[1] = c2;
    curr_chunk = chunk_list[0];
}

void update(void) {
    printf("FPS: %f\tCAMPOS: %f,%f\tPLAYERPOS: %f,%f\tChunkId: %lu\n",1 / window.delta_time,
            camera->position.x, camera->position.y,
            player.pos.x, player.pos.y, curr_chunk->uid);

    if (glfwGetKey(window.handle, GLFW_KEY_W) == GLFW_PRESS) {
        player.pos.y += 200 * window.delta_time;
    }
    else if (glfwGetKey(window.handle, GLFW_KEY_S) == GLFW_PRESS) {
        player.pos.y -= 200 * window.delta_time;
    }

    if (glfwGetKey(window.handle, GLFW_KEY_D) == GLFW_PRESS) {
        player.pos.x += 200 * window.delta_time;
    }
    else if (glfwGetKey(window.handle, GLFW_KEY_A) == GLFW_PRESS) {
        player.pos.x -= 200 * window.delta_time;
    }

    // c1 to c2
    if ((player.pos.x + player.size.x > c1->end_position.x || player.pos.y + player.size.y > c1->end_position.y) && curr_chunk == c1) {
        next_chunk = true;
    }

    // c2 back to c1
    if ((player.pos.x + player.size.x > c2->end_position.x || player.pos.y + player.size.y > c2->end_position.y) && curr_chunk == c2) {
        next_chunk = true;
    }


    if (next_chunk) {
        curr_chunk_idx = (curr_chunk_idx + 1) % 2;
        curr_chunk = chunk_list[curr_chunk_idx];
        player.pos.x = curr_chunk->start_position.x;
        next_chunk = false;
    }

    camera_center_to_entity(&camera->position, player.pos, player.size);
    border_collision(&camera->position, (vec2s){PROJECTION_WIDTH, PROJECTION_HEIGHT}, curr_chunk->start_position, curr_chunk->end_position);
    border_collision(&player.pos, player.size, curr_chunk->start_position, curr_chunk->end_position);

    camera_update(camera); 

    renderer_clean();
    renderer_prepare();

    renderer_append_quad(PLAYER_LAYER, player.size, player.pos, player.color);

    for (int i = 0; i < (60*60); i++) {
        renderer_append_quad(TERRAIN_LAYER, curr_chunk->entities[i].size, curr_chunk->entities[i].pos, curr_chunk->entities[i].color);
    }

    renderer_render();
}

void cleanup(void) {
    renderer_destroy();
    asset_manager_destroy(asset_manager);
    camera_destroy(camera);
}

int main(void) {
    if (window_init(&window, setup, update, cleanup)) {
        window_loop(&window);
    }
    window_destroy(&window);
    return 0;
}

/*
 * 24/08/05 Task
 *  - Chunk render (COMPLETED)
 *  - Dump Player (COMPLETED)
 *  - Move between Chunk (COMPLETED)
 * */
