#include "../util/debug.h"
#include "../global.h"
#include "../defs.h"

#include "renderer.h"
#include "animation.h"
#include "cglm/struct/vec2.h"
#include "chunk.h"
#include "dialog.h"
#include "physics.h"
#include "player.h"
#include "prefab.h"

#include <pthread.h>
#include <glad/glad.h>
#include <string.h>

static int                 texture_slot[8] = {0,1,2,3,4,5,6,7};
static struct LineBatchRender *_line_batch = NULL;
static struct BatchRender       **_batches = NULL; 
static Chunk                     **_chunks = NULL;
static b8                          prepare = false;

static pthread_mutex_t lock;

// SUGGEST: fading maybe should be in scnce module
void fade(void) {
    f32 time = global.dt * 2;

    switch (global.FadeState.state) {
        case FADE_IN:
            if (global.FadeState.alpha > 0.0) {
                global.FadeState.alpha -= time;
            } 

            if (global.FadeState.alpha < 0.0) {
                global.FadeState.alpha  = 0.0f;
                global.FadeState.state = FADE_NONE;
            }
            break;
        case FADE_OUT:
            if (global.FadeState.alpha < 1.0) {
                global.FadeState.alpha += time;
            } 

            if (global.FadeState.alpha > 1.0) {
                global.FadeState.alpha = 1.0f;
                prepare = true;
            }
            break;
        default:
            break;
    }
}

void collision_callback(Static_Body *body, Body *other) {
    Chunk *chunk = global.ChunkState.chunk;

    if ((body->collision_flag & COLLISION_LAYER_TELEPORTER) == COLLISION_LAYER_TELEPORTER) {
        for (u8 i = 0; i < chunk->teleporter_count; ++i) {
            Static_Body *teleporter_body = physics_static_body_get(chunk->teleporter[i].body_id);

            if (body == teleporter_body) {
                other->velocity = glms_vec2_zero();
                player_set_animation(IDLE, global.PlayerState.direction);

                if (global.FadeState.state == FADE_NONE) global.FadeState.state = FADE_OUT;
                if (prepare) renderer_set_chunk(chunk->teleporter[i].chunkId, chunk->teleporter[i].target_coord);
            }
        }
        return;
    }
    if (body->collision_flag == COLLISION_LAYER_DIALOG) {
        puts("You hit dialog");
    }
}

void renderer_reload_chunk(void) {
    pthread_mutex_lock(&lock);

    for (u32 i = 0; i < CHUNK_LAST; ++i) {
        free(_chunks[i]->uv);
        free(_chunks[i]->prefab);
        free(_chunks[i]->dialog);
        free(_chunks[i]->collider);
        free(_chunks[i]->teleporter);
        free(_chunks[i]);
    }              

    _chunks[CHUNK_SPAWN]             = chunk_load_from_file("res/data/chunk_spawn");
    _chunks[CHUNK_VILLAGE_ENTRANCE]  = chunk_load_from_file("res/data/chunk_village_entrance");
    _chunks[CHUNK_VILLAGE_LEFT]      = chunk_load_from_file("res/data/chunk_village_left");
    _chunks[CHUNK_VILLAGE_RIGHT]     = chunk_load_from_file("res/data/chunk_village_right");
    _chunks[CHUNK_VILLAGE_TOP]       = chunk_load_from_file("res/data/chunk_village_top");
    _chunks[CHUNK_VILLAGE_TOP_END]   = chunk_load_from_file("res/data/chunk_village_top_end");
    _chunks[CHUNK_VILLAGE_TOP_LEFT]  = chunk_load_from_file("res/data/chunk_village_top_left");
    _chunks[CHUNK_VILLAGE_TOP_RIGHT] = chunk_load_from_file("res/data/chunk_village_top_right");
    _chunks[CHUNK_VILLAGE_TUNNEL]    = chunk_load_from_file("res/data/chunk_village_tunnel");
    _chunks[CHUNK_INSIDE_LIBRARY]    = chunk_load_from_file("res/data/chunk_inside_library");
    _chunks[CHUNK_INSIDE_RESTAURANT] = chunk_load_from_file("res/data/chunk_inside_restaurant");
    _chunks[CHUNK_INSIDE_CHURCH]     = chunk_load_from_file("res/data/chunk_inside_church");
    _chunks[CHUNK_INSIDE_FISH]       = chunk_load_from_file("res/data/chunk_inside_fish");
    _chunks[CHUNK_INSIDE_LJ_HOME]    = chunk_load_from_file("res/data/chunk_inside_lj_home");
    _chunks[CHUNK_INSIDE_OG_HOME]    = chunk_load_from_file("res/data/chunk_inside_og_home");
    _chunks[CHUNK_INSIDE_VC_HOME]    = chunk_load_from_file("res/data/chunk_inside_vc_home");
    global.ChunkState.chunk = _chunks[global.ChunkState.chunk_id];

    pthread_mutex_unlock(&lock);
    renderer_reset_chunk();
}

void renderer_set_chunk(Chunks chunkId, vec2s target_pos) {
    physics_static_body_reset();

    Body *player_body = physics_body_get(global.PlayerState.body_id);
    Chunk *chunk      = _chunks[chunkId];

    if (target_pos.x == -1) {
        player_body->position = (vec2s){player_body->position.x, chunk->position.y + target_pos.y * TILE_SIZE};
    }
    else if (target_pos.y == -1) {
        player_body->position = (vec2s){chunk->position.x + target_pos.x * TILE_SIZE, player_body->position.y};
    }
    else {
        player_body->position = (vec2s){chunk->position.x + target_pos.x * TILE_SIZE, chunk->position.y + target_pos.y * TILE_SIZE};
    }

    global.ChunkState.chunk    = chunk;
    global.ChunkState.chunk_id = chunkId;

    vec2s pos = (vec2s){chunk->position.x, chunk->position.y};
    for (u32 i = 0; i < chunk->collider_count; ++i) {
        physics_static_body_create(
                (vec2s){chunk->collider[i].pos.x, chunk->collider[i].pos.y}, 
                (vec2s){chunk->collider[i].size.x, chunk->collider[i].size.y}, 
                COLLISION_LAYER_PLAYER, COLLISION_LAYER_SOLID, collision_callback); 
    } 

    for (u32 i = 0; i < chunk->teleporter_count; ++i) {
        chunk->teleporter[i].body_id = physics_static_body_create(
                (vec2s){chunk->teleporter[i].pos.x, chunk->teleporter[i].pos.y}, 
                (vec2s){chunk->teleporter[i].size.x, chunk->teleporter[i].size.y}, 
                COLLISION_LAYER_PLAYER, COLLISION_LAYER_TELEPORTER, collision_callback); 
    } 

    for (u32 i = 0; i < chunk->dialog_count; ++i) {
        chunk->dialog[i].body_id = physics_static_body_create(
                (vec2s){pos.x + TILE_SIZE * chunk->dialog[i].coord.x, pos.y + TILE_SIZE * chunk->dialog[i].coord.y}, 
                DEFAULT_SCALE, COLLISION_LAYER_PLAYER, COLLISION_LAYER_DIALOG, collision_callback); 
    } 

    prepare = false;
    if (global.FadeState.state == FADE_OUT) global.FadeState.state = FADE_IN;
}

void renderer_reset_chunk(void) {
    fprintf(stdout, "Chunk Reset\n");

    physics_static_body_reset();

    Chunk *chunk = global.ChunkState.chunk;

    vec2s pos = (vec2s){chunk->position.x, chunk->position.y};
    for (u32 i = 0; i < chunk->collider_count; ++i) {
        physics_static_body_create(
                (vec2s){chunk->collider[i].pos.x, chunk->collider[i].pos.y}, 
                (vec2s){chunk->collider[i].size.x, chunk->collider[i].size.y}, 
                COLLISION_LAYER_PLAYER, COLLISION_LAYER_SOLID, collision_callback); 
    } 

    for (u32 i = 0; i < chunk->teleporter_count; ++i) {
        chunk->teleporter[i].body_id = physics_static_body_create(
                (vec2s){chunk->teleporter[i].pos.x, chunk->teleporter[i].pos.y}, 
                (vec2s){chunk->teleporter[i].size.x, chunk->teleporter[i].size.y}, 
                COLLISION_LAYER_PLAYER, COLLISION_LAYER_TELEPORTER, collision_callback); 
    } 

    for (u32 i = 0; i < chunk->dialog_count; ++i) {
        chunk->dialog[i].body_id = physics_static_body_create(
                (vec2s){pos.x + TILE_SIZE * chunk->dialog[i].coord.x, pos.y + TILE_SIZE * chunk->dialog[i].coord.y}, 
                DEFAULT_SCALE, COLLISION_LAYER_PLAYER, COLLISION_LAYER_DIALOG, collision_callback); 
    } 
}

void renderer_append_line_segment(vec2s start, vec2s end, vec4s color) {
    u32 idx = _line_batch->line_count++;

    _line_batch->vertices[idx * 2 + 0] = (struct LineVertex){ 
        .position  = start,
        .color     = color, 
    };
    _line_batch->vertices[idx * 2 + 1] = (struct LineVertex){ 
        .position  = end,
        .color     = color, 
    };
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
    global.collision_callback = collision_callback;
    
    pthread_mutex_init(&lock, NULL);

    asset_manager_push_shader(global.asset_manager, "default_shader", "res/shaders/default.vert", "res/shaders/default.frag");
    asset_manager_push_shader(global.asset_manager, "line_shader",    "res/shaders/line.vert",    "res/shaders/line.frag");

    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_TEXT,         81,  3, 27, 32);
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_PLAYER,       32,  4,  8, 16);
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_TILE,         56,  7,  8, 16);
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_INSIDE,     1692, 36, 47, 16);
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_STRUCTURES,  368, 23, 18, 16);
    
    camera_init(&global.camera, (vec2s){0,0});
    physics_init();
    animation_init();
    prefab_init();
    player_init();

    // for debugging
    editor_init();

    _line_batch = malloc(sizeof(*_line_batch));
    assert(_line_batch);

    _line_batch->shader        = *(struct Shader*)asset_manager_get_shader(global.asset_manager, "line_shader"); 
    _line_batch->vertices      = malloc(MAX_VERTICES_PER_BATCH * sizeof(*_line_batch->vertices));
    _line_batch->line_count    = 0;

    GL_TRY(glGenVertexArrays(1, &_line_batch->vao));
    GL_TRY(glGenBuffers(1, &_line_batch->vbo));

    GL_TRY(glBindVertexArray(_line_batch->vao));
    GL_TRY(glBindBuffer(GL_ARRAY_BUFFER, _line_batch->vbo));
    GL_TRY(glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES_PER_BATCH * sizeof(struct LineVertex), NULL, GL_DYNAMIC_DRAW));

    GL_TRY(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(struct LineVertex), (void*)offsetof(struct LineVertex, position)));
    GL_TRY(glEnableVertexAttribArray(0));

    GL_TRY(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(struct LineVertex), (void*)offsetof(struct LineVertex, color)));
    GL_TRY(glEnableVertexAttribArray(1));
    GL_TRY(glBindVertexArray(0));

    u32 *indices = malloc(MAX_INDICES_PER_BATCH * sizeof(*indices));
    for (int i = 0; i < MAX_QUAD_PER_BATCH; i++) {
        u32 offset = i * 4;
        u32 idx    = i * 6;

        indices[idx + 0] = offset + 0;
        indices[idx + 1] = offset + 1;
        indices[idx + 2] = offset + 2;

        indices[idx + 3] = offset + 2;
        indices[idx + 4] = offset + 3;
        indices[idx + 5] = offset + 0;
    }

    _batches = malloc(LAYER_LAST * sizeof(_batches));
    assert(_batches);

    for (int i = 0; i < LAYER_LAST; ++i) {
        _batches[i] = malloc(sizeof(*_batches[i]));
        assert(_batches[i]);

        _batches[i]->vertices      = malloc(MAX_VERTICES_PER_BATCH * sizeof(*_batches[i]->vertices));
        assert(_batches[i]->vertices);

        _batches[i]->shader        = *(struct Shader*)asset_manager_get_shader(global.asset_manager, "default_shader"); 
        _batches[i]->quad_count    = 0;
        _batches[i]->texture_count = 0;

        GL_TRY(glGenVertexArrays(1, &_batches[i]->vao));
        GL_TRY(glGenBuffers(1, &_batches[i]->vbo));
        GL_TRY(glGenBuffers(1, &_batches[i]->ebo));

        GL_TRY(glBindVertexArray(_batches[i]->vao));

        GL_TRY(glBindBuffer(GL_ARRAY_BUFFER, _batches[i]->vbo));
        GL_TRY(glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES_PER_BATCH * sizeof(struct Vertex), NULL, GL_DYNAMIC_DRAW));

        GL_TRY(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _batches[i]->ebo));
        GL_TRY(glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES_PER_BATCH * sizeof(*indices), indices, GL_STATIC_DRAW));

        GL_TRY(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, position)));
        GL_TRY(glEnableVertexAttribArray(0));

        GL_TRY(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, color)));
        GL_TRY(glEnableVertexAttribArray(1));
        
        GL_TRY(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, tex_coord)));
        GL_TRY(glEnableVertexAttribArray(2));

        GL_TRY(glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(struct Vertex), (void*)offsetof(struct Vertex, tex_slot)));
        GL_TRY(glEnableVertexAttribArray(3));
        GL_TRY(glBindVertexArray(0));
    }
    free(indices);


    { // create prefabs
        struct Spritesheet *structures_spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_STRUCTURES);
        prefab_create("bus_station", structures_spritesheet, WHITE, (vec2s){48,48},  (vec4s){12,12,15,15});
        prefab_create("bus_stop",    structures_spritesheet, WHITE, (vec2s){16,32},  (vec4s){15,12,16,14});
        prefab_create("restaurant",  structures_spritesheet, WHITE, (vec2s){112,96}, (vec4s){ 6, 0,13, 6});
        prefab_create("library",     structures_spritesheet, WHITE, (vec2s){160,80}, (vec4s){ 0,18,10,23});
        prefab_create("church",      structures_spritesheet, WHITE, (vec2s){80,96},  (vec4s){12,15,17,21});
        prefab_create("fish_shop",   structures_spritesheet, WHITE, (vec2s){80,64},  (vec4s){13, 0,18, 4});
        prefab_create("slider",      structures_spritesheet, WHITE, (vec2s){80,64},  (vec4s){13, 4,18, 8});
        prefab_create("horse1",      structures_spritesheet, WHITE, (vec2s){32,32},  (vec4s){14, 8,16,10});
        prefab_create("horse2",      structures_spritesheet, WHITE, (vec2s){32,32},  (vec4s){16, 8,18,10});
        prefab_create("nathan_home", structures_spritesheet, WHITE, (vec2s){96,96},  (vec4s){ 6,12,12,18});
        prefab_create("orange_home", structures_spritesheet, WHITE, (vec2s){96,96},  (vec4s){ 0, 0, 6, 6});
        prefab_create("old_g_home",  structures_spritesheet, WHITE, (vec2s){96,96},  (vec4s){ 0, 6, 6,12});
        prefab_create("g_home",      structures_spritesheet, WHITE, (vec2s){96,96},  (vec4s){ 0,12, 6,18});
        prefab_create("vc_home",     structures_spritesheet, WHITE, (vec2s){96,96},  (vec4s){ 6, 6,12,12});
        prefab_create("sign_down",   structures_spritesheet, WHITE, (vec2s){48,32},  (vec4s){12,10,15,12});
        prefab_create("sign_left",   structures_spritesheet, WHITE, (vec2s){16,48},  (vec4s){17,12,18,15});
        prefab_create("plant_pot",   structures_spritesheet, WHITE, (vec2s){32,32},  (vec4s){12, 8,14,10});

        struct Spritesheet *inside_spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_INSIDE);
        prefab_create("inside_library",    inside_spritesheet, WHITE, (vec2s){352,192}, (vec4s){0,0,23,13});
        prefab_create("inside_restaurant", inside_spritesheet, WHITE, (vec2s){352,176}, (vec4s){0,13,23,24});
        prefab_create("inside_church",     inside_spritesheet, WHITE, (vec2s){192,336}, (vec4s){23,0,36,22});
        prefab_create("inside_fish",       inside_spritesheet, WHITE, (vec2s){160,144}, (vec4s){36,0,47,10});
        prefab_create("inside_lj_home",    inside_spritesheet, WHITE, (vec2s){224,192}, (vec4s){0,24,14,36});
        prefab_create("inside_vc_home",    inside_spritesheet, WHITE, (vec2s){224,192}, (vec4s){14,24,28,36});
        prefab_create("inside_og_home",    inside_spritesheet, WHITE, (vec2s){224,192}, (vec4s){28,24,42,36});
    }

    { // load chunks
        _chunks = malloc(sizeof(_chunks) * CHUNK_LAST);
        _chunks[CHUNK_SPAWN]             = chunk_load_from_file("res/data/chunk_spawn");
        _chunks[CHUNK_VILLAGE_ENTRANCE]  = chunk_load_from_file("res/data/chunk_village_entrance");
        _chunks[CHUNK_VILLAGE_LEFT]      = chunk_load_from_file("res/data/chunk_village_left");
        _chunks[CHUNK_VILLAGE_RIGHT]     = chunk_load_from_file("res/data/chunk_village_right");
        _chunks[CHUNK_VILLAGE_TOP]       = chunk_load_from_file("res/data/chunk_village_top");
        _chunks[CHUNK_VILLAGE_TOP_END]   = chunk_load_from_file("res/data/chunk_village_top_end");
        _chunks[CHUNK_VILLAGE_TOP_LEFT]  = chunk_load_from_file("res/data/chunk_village_top_left");
        _chunks[CHUNK_VILLAGE_TOP_RIGHT] = chunk_load_from_file("res/data/chunk_village_top_right");
        _chunks[CHUNK_VILLAGE_TUNNEL]    = chunk_load_from_file("res/data/chunk_village_tunnel");

        _chunks[CHUNK_INSIDE_LIBRARY]    = chunk_load_from_file("res/data/chunk_inside_library");
        _chunks[CHUNK_INSIDE_RESTAURANT] = chunk_load_from_file("res/data/chunk_inside_restaurant");
        _chunks[CHUNK_INSIDE_CHURCH]     = chunk_load_from_file("res/data/chunk_inside_church");
        _chunks[CHUNK_INSIDE_FISH]       = chunk_load_from_file("res/data/chunk_inside_fish");
        _chunks[CHUNK_INSIDE_LJ_HOME]    = chunk_load_from_file("res/data/chunk_inside_lj_home");
        _chunks[CHUNK_INSIDE_OG_HOME]    = chunk_load_from_file("res/data/chunk_inside_og_home");
        _chunks[CHUNK_INSIDE_VC_HOME]    = chunk_load_from_file("res/data/chunk_inside_vc_home");
    }

    renderer_set_chunk(CHUNK_SPAWN, SPAWN_COORD);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
}

void renderer_destroy(void) {
    pthread_mutex_destroy(&lock);

    camera_destroy(global.camera);
    physics_destroy();
    animation_destroy();
    prefab_destroy();

    // debugging
    editor_destroy();

    glDeleteVertexArrays(1, &_line_batch->vao);
    glDeleteBuffers(1, &_line_batch->vbo);
    free(_line_batch->vertices);
    free(_line_batch);

    for (int i = 0; i < LAYER_LAST; ++i) {
        glDeleteVertexArrays(1, &_batches[i]->vao);
        glDeleteBuffers(1, &_batches[i]->vbo);
        glDeleteBuffers(1, &_batches[i]->ebo);

        free(_batches[i]->vertices);
        free(_batches[i]);
    }
    free(_batches);

    for (u32 i = 0; i < CHUNK_LAST; ++i) {
        free(_chunks[i]->uv);
        free(_chunks[i]->prefab);
        free(_chunks[i]->dialog);
        free(_chunks[i]->collider);
        free(_chunks[i]->teleporter);
        free(_chunks[i]);
    }
    free(_chunks);
}

void renderer_prepare(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0,0.0,0.0,1.0);

    fade();

    _line_batch->line_count   = 0;
    for (int i = 0; i < LAYER_LAST; ++i) {
        _batches[i]->quad_count = 0;
    }
}

void renderer_render(void) {

    { 
        Body *player_body = physics_body_get(global.PlayerState.body_id);
        struct Spritesheet *player_spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_PLAYER);
        f32 tex_coord[4];

        player_get_tex_coord(tex_coord);
        renderer_append_quad_texture(LAYER_PLAYER, 
                (vec3s){player_body->position.x,player_body->position.y, 0.0f}, 
                PLAYER_SIZE, WHITE, player_spritesheet->texture, tex_coord);
    }

    if (global.start_point[0] != 0 && global.start_point[1] != 0) {
        renderer_append_quad(LAYER_PLAYER, (vec3s){global.start_point[0], global.start_point[1], 0.0f}, (vec2s){1,1}, GREEN);
    }
    if (global.end_point[0] != 0 && global.end_point[1] != 0) {
        renderer_append_quad(LAYER_PLAYER, (vec3s){global.end_point[0], global.end_point[1], 0.0f}, (vec2s){1,1}, BLUE);
    }

    // render cursor
    renderer_append_quad(LAYER_PLAYER, (vec3s){global.window->mouse.orthox, global.window->mouse.orthoy, 0.0f}, (vec2s){1,1}, WHITE);

    // fade rect
    renderer_append_quad(LAYER_PLAYER, (vec3s){global.camera->position.x,global.camera->position.y,0.0f}, (vec2s){WIDTH, HEIGHT}, (vec4s){0,0,0,global.FadeState.alpha});

    if (global.ChunkState.chunk)             chunk_render();
    if (global.DialogState.curr_dialog_node) dialog_render();
    if (global.toggle_show_collider)         physics_render_collider();

    {
        for (u8 layer = 0; layer < LAYER_LAST; ++layer) {
            GL_TRY(shader_bind(_batches[layer]->shader));
            GL_TRY(shader_uniform_viewproj(_batches[layer]->shader, get_view_proj(global.camera)));
            GL_TRY(shader_uniform_int_array(_batches[layer]->shader, "tex", 8, texture_slot));

            GL_TRY(glBindBuffer(GL_ARRAY_BUFFER, _batches[layer]->vbo)); 
            GL_TRY(glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_VERTICES_PER_BATCH * sizeof(struct Vertex), _batches[layer]->vertices));

            for (u32 i = 0; i < _batches[layer]->texture_count; ++i) {
                texture_bind(_batches[layer]->textures[i], i);
            }

            GL_TRY(glBindVertexArray(_batches[layer]->vao));
            GL_TRY(glDrawElements(GL_TRIANGLES, (6 * _batches[layer]->quad_count), GL_UNSIGNED_INT, 0));

            GL_TRY(glBindTexture(GL_TEXTURE_2D, 0));
            GL_TRY(glBindVertexArray(0));
            shader_unbind();
        }
    }

    {
        GL_TRY(shader_bind(_line_batch->shader));
        GL_TRY(shader_uniform_viewproj(_line_batch->shader, get_view_proj(global.camera)));

        GL_TRY(glBindVertexArray(_line_batch->vao));
        GL_TRY(glDrawArrays(GL_LINES, 0, (_line_batch->line_count * 2)));
        GL_TRY(glBindBuffer(GL_ARRAY_BUFFER, _line_batch->vbo));
        GL_TRY(glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_VERTICES_PER_BATCH * sizeof(struct LineVertex), _line_batch->vertices));
        GL_TRY(glBindVertexArray(0));
        GL_TRY(shader_unbind());

    }
    GL_TRY();

    if (global.toggle_editor){ editor_render(); }

}

void renderer_append_aabb(AABB aabb, vec4s color) {
        renderer_append_quad_line(aabb.center, aabb.half_size, color);
}

void renderer_append_prefab(RenderLayer layer, vec2s coord, char *prefab_name) {
    Prefab *prefab = prefab_get(prefab_name);

    vec3s position = { 
        global.ChunkState.chunk->position.x + TILE_SIZE * coord.x, 
        global.ChunkState.chunk->position.y + TILE_SIZE * coord.y, 
        0.0f
    };

    renderer_append_quad_texture(layer, position, prefab->size, prefab->color, prefab->spritesheet->texture, prefab->tex_coord);
}

void renderer_append_quad(RenderLayer layer, vec3s position, vec2s size, vec4s color) {
    f32 _tex_coord[4] = {0,1,0,1};

    u32 idx = _batches[layer]->quad_count++;
    _batches[layer]->vertices[idx * 4 + 0] = (struct Vertex){ 
        .position  = { .x = position.x, .y = position.y, .z = position.z}, 
        .color     = color, 
        .tex_coord = {_tex_coord[0],_tex_coord[2]}, 
        .tex_slot  = -1
    };
    _batches[layer]->vertices[idx * 4 + 1] = (struct Vertex){ 
        .position  = { .x = position.x + size.x, .y = position.y, .z = position.z}, 
        .color     = color, 
        .tex_coord = {_tex_coord[1], _tex_coord[2]}, 
        .tex_slot  = -1
    };
    _batches[layer]->vertices[idx * 4 + 2] = (struct Vertex){ 
        .position  = { .x = position.x + size.x, .y = position.y + size.y, .z = position.z}, 
        .color     = color, 
        .tex_coord = {_tex_coord[1],_tex_coord[3]}, 
        .tex_slot  = -1
    };
    _batches[layer]->vertices[idx * 4 + 3] = (struct Vertex){ 
        .position  = { .x = position.x, .y = position.y + size.y, .z = position.z}, 
        .color     = color, 
        .tex_coord = {_tex_coord[0],_tex_coord[3]}, 
        .tex_slot  = -1
    };
}

void renderer_append_quad_texture(RenderLayer layer, vec3s position, vec2s size, vec4s color, struct Texture texture, f32 *tex_coord) {
    f32 _tex_coord[4] = {0,1,0,1};
    u32 tex_slot      = renderer_batch_append_texture(layer, texture);

    if (tex_coord != NULL){
        memcpy(_tex_coord, tex_coord, sizeof(f32)*4);
    }

    u32 idx = _batches[layer]->quad_count++;
    _batches[layer]->vertices[idx * 4 + 0] = (struct Vertex){ 
        .position  = { .x = position.x, .y = position.y, .z = position.z}, 
        .color     = color, 
        .tex_coord = {_tex_coord[0],_tex_coord[2]}, 
        .tex_slot  = tex_slot 
    };
    _batches[layer]->vertices[idx * 4 + 1] = (struct Vertex){ 
        .position  = { .x = position.x + size.x, .y = position.y, .z = position.z}, 
        .color     = color, 
        .tex_coord = {_tex_coord[1], _tex_coord[2]}, 
        .tex_slot  = tex_slot 
    };
    _batches[layer]->vertices[idx * 4 + 2] = (struct Vertex){ 
        .position  = { .x = position.x + size.x, .y = position.y + size.y, .z = position.z}, 
        .color     = color, 
        .tex_coord = {_tex_coord[1],_tex_coord[3]}, 
        .tex_slot  = tex_slot 
    };
    _batches[layer]->vertices[idx * 4 + 3] = (struct Vertex){ 
        .position  = { .x = position.x, .y = position.y + size.y, .z = position.z}, 
        .color     = color, 
        .tex_coord = {_tex_coord[0],_tex_coord[3]}, 
        .tex_slot  = tex_slot 
    };
}

u32 renderer_batch_append_texture(RenderLayer layer, struct Texture texture) {
    b8 found     = false;
    u32 tex_slot = -1;

    for (u32 i = 0; i < _batches[layer]->texture_count; i++) {
        if (_batches[layer]->textures[i].handle == texture.handle) {
            tex_slot = i;
            found = true;
            break;
        }
    }

    if (!found) {
        tex_slot = _batches[layer]->texture_count++;
        _batches[layer]->textures[tex_slot] = texture;
    } 

    return tex_slot;
}
