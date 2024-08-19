#include "global.h"
#include "defs.h"

#include "gfx/animation.h"

#include "core/entity.h"
#include "core/renderer.h"
#include "core/chunk_internal.h"


struct Global global;

enum Direction { 
    UP,
    DOWN,
    LEFT,
    RIGHT, 

    DIRECTION_LAST 
};

static u32 adef_idle[DIRECTION_LAST];
static u32 adef_walk[DIRECTION_LAST];
static u32 animation_idle[DIRECTION_LAST];
static u32 animation_walk[DIRECTION_LAST];
static enum Direction player_direction;
static Entity player  = {0};

static Chunk *chunk_list[2];

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

static void input_handling(void) {
    s32 up    = glfwGetKey(global.window->handle, GLFW_KEY_W);
    s32 down  = glfwGetKey(global.window->handle, GLFW_KEY_S);
    s32 right = glfwGetKey(global.window->handle, GLFW_KEY_D);
    s32 left  = glfwGetKey(global.window->handle, GLFW_KEY_A);

    if (up == GLFW_PRESS) {
        player.position.y += 200 * global.dt;
        player.animation_id = animation_walk[UP];
        player_direction    = UP;
    }
    else if (down == GLFW_PRESS) {
        player.position.y -= 200 * global.dt;
        player.animation_id = animation_walk[DOWN];
        player_direction    = DOWN;
    }

    if (right == GLFW_PRESS) {
        player.position.x += 200 * global.dt;
        player.animation_id = animation_walk[RIGHT];
        player_direction    = RIGHT;
    }
    else if (left == GLFW_PRESS) {
        player.position.x -= 200 * global.dt;
        player.animation_id = animation_walk[LEFT];
        player_direction    = LEFT;
    }

    if (!up && !down && !right && !left) {
        player.animation_id = animation_idle[player_direction];
    }

    if (glfwGetKey(global.window->handle, GLFW_KEY_X) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else if (glfwGetKey(global.window->handle, GLFW_KEY_Z) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void setup(void) {
    asset_manager_init(&global.asset_manager);

    camera_init(&global.camera, (vec3s){0,0,0}, (vec2s){100,100});

    renderer_init();

    animation_init();

    // add needed resources
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_CHUNK2, 12, 3, 4, 16);
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_CHUNK,  49, 7, 7, 16);
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_PLAYER, 32, 4, 8, 16);
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_NPC,    32, 4, 8, 16);
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_NPC2,   12, 3, 4, 16);

    // initialize player
    player = (Entity){HUMAN_SCALE, (vec3s){CHUNK1_SPAWN_X, CHUNK1_SPAWN_Y}, WHITE, .animation_id = -1};

    // initialize chunks
    chunk_init(&chunk_list[0], 60, 60, (vec2s){0,0}, chunk1_uv);
    chunk_init(&chunk_list[1], 60, 60, (vec2s){chunk_list[0]->end_position.x, 0}, chunk2_uv);

    global.ChunkState.chunk     = chunk_list[0];
    global.ChunkState.chunk_idx = 0;

    // chunk0 objects
    {
        // bus stop station
        chunk_push_structure(chunk_list[0], (vec2s){TILE_SIZE*3,TILE_SIZE*6}, (vec2s){15,10}, 4, 0, 6, 3);

        // bus stop sign
        chunk_push_structure(chunk_list[0], (vec2s){TILE_SIZE,TILE_SIZE*2}, (vec2s){20, 10}, 4, 3, 2, 1);

        // npc
        chunk_push_npc(chunk_list[0], HUMAN_SCALE, (vec2s){20, 20}, 3, 0);
    }

    { // player animation
        struct Spritesheet *player_spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_PLAYER);

        adef_idle[LEFT]  = animation_definition_create(player_spritesheet, (f32[]){0},(u8[]){0},(u8[]){0},1);
        adef_idle[RIGHT] = animation_definition_create(player_spritesheet, (f32[]){0},(u8[]){1},(u8[]){0},1);
        adef_idle[UP]    = animation_definition_create(player_spritesheet, (f32[]){0},(u8[]){2},(u8[]){0},1);
        adef_idle[DOWN]  = animation_definition_create(player_spritesheet, (f32[]){0},(u8[]){3},(u8[]){0},1);

        adef_walk[LEFT]  = animation_definition_create(player_spritesheet, (f32[]){0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1}, (u8[]){0,0,0,0,0,0,0,0},(u8[]){0,1,2,3,4,5,6,7}, 8);
        adef_walk[RIGHT] = animation_definition_create(player_spritesheet, (f32[]){0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1}, (u8[]){1,1,1,1,1,1,1,1},(u8[]){0,1,2,3,4,5,6,7}, 8);
        adef_walk[UP]    = animation_definition_create(player_spritesheet, (f32[]){0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1}, (u8[]){2,2,2,2,2,2,2,2},(u8[]){0,1,2,3,4,5,6,7}, 8);
        adef_walk[DOWN]  = animation_definition_create(player_spritesheet, (f32[]){0.1,0.1,0.1,0.1,0.1,0.1,0.1,0.1}, (u8[]){3,3,3,3,3,3,3,3},(u8[]){0,1,2,3,4,5,6,7}, 8);

        animation_walk[LEFT]  = animation_create(adef_walk[LEFT], true);
        animation_walk[RIGHT] = animation_create(adef_walk[RIGHT], true);
        animation_walk[UP]    = animation_create(adef_walk[UP], true);
        animation_walk[DOWN]  = animation_create(adef_walk[DOWN], true);

        animation_idle[LEFT]  = animation_create(adef_idle[LEFT], false);
        animation_idle[RIGHT] = animation_create(adef_idle[RIGHT], false);
        animation_idle[UP]    = animation_create(adef_idle[UP], false);
        animation_idle[DOWN]  = animation_create(adef_idle[DOWN], false);

        player.animation_id = animation_idle[DOWN];
        player_direction    = DOWN;
    }
}

void update(void) {
   /* ivec2s chunk_rc_pos = chunk_get_row_col_position(curr_chunk, player.position); */
   /*  LOG_DEBUG("FPS: %f\tCAMPOS: %f,%f\tPLAYERPOS: %f,%f\tChunkId: %lu\tChunk Position RC: %u,%u\n", */
   /*          1 / window.delta_time, */
   /*          camera->position.x, camera->position.y, */
   /*          player.position.x, player.position.y, */
   /*          curr_chunk->uid, chunk_rc_pos.x, chunk_rc_pos.y); */

    input_handling();

    { // go between two chunk
        if (player.position.x + player.size.x > chunk_list[0]->end_position.x && global.ChunkState.chunk_idx == 0) {
            global.ChunkState.next_chunk = true;
        }

        if (player.position.x + player.size.x > chunk_list[1]->end_position.x && global.ChunkState.chunk_idx == 1) {
            global.ChunkState.next_chunk = true;
        }
    }

    { // Camera
        camera_center_to_entity(&global.camera->position, player.position, player.size);
        border_collision(&global.camera->position, (vec2s){PROJECTION_WIDTH, PROJECTION_HEIGHT}, global.ChunkState.chunk->start_position, global.ChunkState.chunk->end_position);
        border_collision(&player.position, player.size, global.ChunkState.chunk->start_position, global.ChunkState.chunk->end_position);
        camera_update(global.camera); 
    }

    animation_update(global.dt);
    /* physics_update(global.dt); */

    chunk_update();

    renderer_prepare();
    renderer_render_chunk(global.ChunkState.chunk);
    renderer_render();
}

void cleanup(void) {
    chunk_destroy(chunk_list[0]);
    chunk_destroy(chunk_list[1]);

    camera_destroy(global.camera);

    renderer_destroy();

    animation_destroy();

    /* physics_destroy(); */
    /* dialog_destroy(); */

    asset_manager_destroy(global.asset_manager);
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
