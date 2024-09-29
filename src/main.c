#include "core/timer.h"
#include "engine/logger.h"

#include "core/asset_manager.h"
#include "core/animation.h"
#include "core/renderer.h"
#include "core/player.h"
#include "core/prefab.h"
#include "core/scnce.h"

#include "gfx/window.h"

#define DEBUG_INPUT

#include "global.h"
#include "defs.h"

//   ~85%
//   Progression system       -- ON GOING --
//   Finish Editor            -- PLANNED  --
//   Sounds system            -- PLANNED  --
//
//   PLAN -- Fix prefab position of inside chunks.(DONE) 
//           Gradient color (Day/Night)           (DONE)
//           Logger                               (DONE)
//           Finish inside art                    (DONE)
//           place collider                       (DONE)
//           camera                               (CAN SKIP)
//           progression                          (5%)
//
//           Night gradient -> (64,25,71,140) or (0,0,0,174)

// We will make a function to handle setup for all acts of the game

// SUGGEST: Maybe attach the renderer to scnce to make it can render text and fade when we want, and also the fade layer
// SUGGEST: Maybe remove the act_trigger
// SUGGEST: Change from physics (Static_Body, Body) to ECS

b8 act_trigger = false;

typedef enum {
    LAYER_BASE,
    LAYER_BASE_UPPER,
    LAYER_STRUCTURE,
    LAYER_TOP,

    LAYER_LAST
} RenderLayer;

LineRenderer *line_renderer;
TextRenderer *text_renderer;
QuadRenderer **quad_renderer;

static void _append_collider(void) {
    Body *body; 
    for (u32 i = 0; i < global.physics->body_list->len; i++) {
        body = physics_body_get(global.physics, i);
        line_renderer_append_aabb(line_renderer, body->aabb, (global.PlayerState.on_collision) ? RED : GREEN);
    }

    Static_Body *static_body; 
    for (u32 i = 0; i < global.physics->static_body_list->len; i++) {
        static_body = physics_static_body_get(global.physics, i);

        if (static_body->collision_flag & COLLISION_LAYER_SOLID) {
            line_renderer_append_aabb(line_renderer, static_body->aabb, WHITE);
        }
        else if (static_body->collision_flag & COLLISION_LAYER_TELEPORTER) {
            line_renderer_append_aabb(line_renderer, static_body->aabb, BLACK);
        }
        else if (static_body->collision_flag & COLLISION_LAYER_DIALOG) {
            line_renderer_append_aabb(line_renderer, static_body->aabb, BLUE);
        }
    }
}

static void _setup_act(void) {
    if (act_trigger) {
        switch (global.act) {
            case ACT0:
                LOG_DEBUG("Act 0 Introduction");
                break;
            case ACT1:
                LOG_DEBUG("Act 1 In the Village");
                break;
            case ACT2:
                LOG_DEBUG("Act 2");
                break;
            case ACT3:
                LOG_DEBUG("Act 3");
                break;
            case ACT4:
                LOG_DEBUG("Act 4");
                break;
        } 

        act_trigger = false;
    }
}

static ivec2s get_char_coord(char c) {
    static u8 text_index[3][27] = {
        "!@#$%^&*()_+-={}[]:\";\'<>,.?",
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ/",
        "abcdefghijklmnopqrstuvwxyz ",
    };

    ivec2s result = {0};

    for (u8 y = 0; y < 3; ++y) {
        for (u8 x = 0; x < 27; ++x) {
            if (c == text_index[y][x]) {
                result.x = x;
                result.y = y;
                break;
            }
        }
    }

    return result;
}

static void _collision_callback(Static_Body *body, Body *other) {
    Chunk *chunk = global.scnce->chunk;

    if ((body->collision_flag & COLLISION_LAYER_TELEPORTER) == COLLISION_LAYER_TELEPORTER) {
        for (u8 i = 0; i < chunk->teleporter_count; ++i) {
            Static_Body *teleporter_body = physics_static_body_get(global.physics, chunk->teleporter[i].body_id);

            if (body == teleporter_body) {
                other->velocity = glms_vec2_zero();
                player_set_animation(IDLE, global.PlayerState.direction);

                if (global.scnce->fade_state == FADE_NONE) global.scnce->fade_state = FADE_OUT;
                if (global.scnce->faded) scnce_change_chunk(global.scnce, other, chunk->teleporter[i].chunkId, chunk->teleporter[i].target_coord);
            }
        }
        return;
    }

    if (body->collision_flag == COLLISION_LAYER_DIALOG) {
        LOG_DEBUG("Physics: Hit the dialog layer");
    }
}

static void input_handling(void) {
    if (global.scnce->fade_state == FADE_NONE) player_input();

#ifdef DEBUG_INPUT
    static f32 delay = 0.0f;

    delay += global.dt;

    if (delay >= 0.15) {
        if (window_get_key(global.window, GLFW_KEY_X)) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            delay = 0.0f;
        }
        else if (window_get_key(global.window, GLFW_KEY_Z)) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            delay = 0.0f;
        }

        if (window_get_key(global.window, GLFW_KEY_I)) {
            global.toggle_collision = !global.toggle_collision;
            delay = 0.0f;
        }
        if (window_get_key(global.window, GLFW_KEY_U)) {
            global.toggle_show_collider = !global.toggle_show_collider;
            delay = 0.0f;
        }
        if (window_get_key(global.window, GLFW_KEY_Y)) {
            global.toggle_editor = !global.toggle_editor;
            delay = 0.0f;
        }

        if (glfwGetMouseButton(global.window->handle, GLFW_MOUSE_BUTTON_LEFT)) {
            if (global.cursor_mode == START_POINT) {
                global.start_point[0] = global.window->mouse.orthox;
                global.start_point[1] = global.window->mouse.orthoy;
            }
            else if (global.cursor_mode == END_POINT) {
                global.end_point[0] = global.window->mouse.orthox;
                global.end_point[1] = global.window->mouse.orthoy;
            }
            delay = 0.0f;
        }
        if (glfwGetMouseButton(global.window->handle, GLFW_MOUSE_BUTTON_RIGHT)) {
            global.cursor_mode = (global.cursor_mode + 1) % 3;
            delay = 0.0f;
        }
    }
#endif
}

void setup(void) {
    global.act = ACT0;
    global.gradient = glms_vec4_zero();
    global.collision_callback = _collision_callback;

    pthread_mutex_init(&global.lock, NULL);
    global.asset_manager = asset_manager_init();

    // Separate "default_shader" and "texture_shader" because default_shader is used with 8 slot textures with RGBa,
    // otherwise "texture_shader" is used only with 1 slot textures alpha.
    asset_manager_push_shader(global.asset_manager, "default_shader", "res/shaders/default.vert", "res/shaders/default.frag");
    asset_manager_push_shader(global.asset_manager, "texture_shader", "res/shaders/texture.vert", "res/shaders/texture.frag");
    asset_manager_push_shader(global.asset_manager, "line_shader",    "res/shaders/line.vert",    "res/shaders/line.frag");

    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_TEXT,         81,  3, 27, 32);
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_PLAYER,       32,  4,  8, 16);
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_TILE,         56,  7,  8, 16);
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_INSIDE,     1692, 36, 47, 16);
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_STRUCTURES,  368, 23, 18, 16);

    global.timer = timer_create();
    global.physics = physics_init(10);
    global.animations = animation_init();

    player_init();
    prefab_init();

    global.scnce = scnce_init();
    scnce_change_scnce(global.scnce, MENU);

    line_renderer = line_renderer_init();
    text_renderer = text_renderer_init(get_char_coord);
    quad_renderer = malloc(LAYER_LAST * sizeof(quad_renderer));
    for (u8 i = 0; i < LAYER_LAST; ++i) {
        quad_renderer[i] = quad_renderer_init();
    }

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

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);

    // for debugging
    editor_init();
}

void update(void) {

    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0,0.0,0.0,1.0);

    global.input_delay += global.dt;

    Body *player_body = physics_body_get(global.physics, global.PlayerState.body_id);

    scnce_update(global.scnce, player_body);
    timer_update(global.timer);

    _setup_act();

    if (global.scnce->scnce_state == INGAME) { 

        input_handling();

        animation_update(global.animations, global.dt);
        physics_update(global.physics, global.dt);

        // base layer
        struct Spritesheet *spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_TILE);
        for (s32 y = 0; y < CHUNK_SIZE_Y; y++) {
            for (s32 x = 0; x < CHUNK_SIZE_X; x++) {
                u32 uv = global.scnce->chunk->uv[CHUNK_SIZE_X * y + x]; 

                if (uv == (u32)-1) { continue; }

                u32 row    = uv / spritesheet->cols;
                u32 col    = uv % spritesheet->cols;
                f32 cellx  = spritesheet->stride / spritesheet->size.x;
                f32 celly  = spritesheet->stride / spritesheet->size.y; 

                f32 tex_coord[4] = {
                    (cellx * col), 
                    (cellx * col) + cellx, 
                    (celly * row), 
                    (celly * row) + celly
                };
                vec3s position   = {global.scnce->chunk->position.x + x * TILE_SIZE, global.scnce->chunk->position.y + y * TILE_SIZE, 0.0};
                quad_renderer_append_quad_texture(quad_renderer[LAYER_BASE], position, DEFAULT_SCALE, WHITE, spritesheet->texture, tex_coord);
            }
        }

        // base upper layer
        u32 offset = CHUNK_SIZE_Y * CHUNK_SIZE_X;
        for (s32 y = 0; y < CHUNK_SIZE_Y; y++) {
            for (s32 x = 0; x < CHUNK_SIZE_X; x++) {
                u32 uv = global.scnce->chunk->uv[offset + CHUNK_SIZE_X * y + x]; 

                if (uv == 0) continue;

                u32 row    = uv / spritesheet->cols;
                u32 col    = uv % spritesheet->cols;
                f32 cellx  = spritesheet->stride / spritesheet->size.x;
                f32 celly  = spritesheet->stride / spritesheet->size.y; 

                f32 tex_coord[4] = {
                    (cellx * col), 
                    (cellx * col) + cellx, 
                    (celly * row), 
                    (celly * row) + celly
                };
                vec3s position   = {global.scnce->chunk->position.x + x * TILE_SIZE, global.scnce->chunk->position.y + y * TILE_SIZE, 0.0};
                quad_renderer_append_quad_texture(quad_renderer[LAYER_BASE_UPPER], position, DEFAULT_SCALE, WHITE, spritesheet->texture, tex_coord);
            }
        }

        // prefab
        for (u8 i = 0; i < global.scnce->chunk->prefab_count; ++i) {
            quad_renderer_append_prefab(quad_renderer[LAYER_STRUCTURE], global.scnce->chunk->prefab[i].coord, global.scnce->chunk->prefab[i].name);
        }


        { // append player to LAYER_TOP
            struct Spritesheet *player_spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_PLAYER);
            f32 tex_coord[4];

            player_get_tex_coord(tex_coord);
            quad_renderer_append_quad_texture(quad_renderer[LAYER_TOP], (vec3s){player_body->position.x,player_body->position.y, 0.0f}, 
                    PLAYER_SIZE, WHITE, player_spritesheet->texture, tex_coord);
        }

        { // append start_point, end_point, and cursor to LAYER_TOP
            quad_renderer_append_quad(quad_renderer[LAYER_TOP], (vec3s){global.start_point[0], global.start_point[1], 0.0f}, (vec2s){1,1}, GREEN);
            quad_renderer_append_quad(quad_renderer[LAYER_TOP], (vec3s){global.end_point[0], global.end_point[1], 0.0f}, (vec2s){1,1}, BLUE);
            quad_renderer_append_quad(quad_renderer[LAYER_TOP], (vec3s){global.window->mouse.orthox, global.window->mouse.orthoy, 0.0f}, (vec2s){1,1}, WHITE);
        }
        if (global.toggle_show_collider) _append_collider();

        // render all layer by order
        for (u8 i = 0; i < LAYER_LAST; ++i) {
            quad_renderer_render(quad_renderer[i]);
        }
        line_renderer_render(line_renderer);
    }
    else {
        text_renderer->quad_count = 0;

        if (global.input_delay >= INPUT_DELAY && window_get_key(global.window, GLFW_KEY_SPACE)) {
            global.input_delay = 0.0f;
            scnce_fade_out(global.scnce);
        }

        if (global.scnce->scnce_state == MENU) {
            if (global.scnce->faded) { 
                scnce_change_scnce(global.scnce, INTRO); 
            }
            text_renderer_append_text(text_renderer, "Press SPACE to start game" , (vec3s){PROJECTION_WIDTH*0.5 - (25*3.5*0.5), 50}, 7, WHITE);
        }
        else { 
            if (!global.scnce->faded && !global.timer->busy) {
                timer_start(global.timer, 3.0f);
            } 

            text_renderer_append_text(text_renderer, "A few days ago, I received a letter, It was written about my" ,     (vec3s){PROJECTION_WIDTH*0.5 - (60*3.5*0.5), 120}, 7, YELLOW);
            text_renderer_append_text(text_renderer, "missing grandfather and where I could find him. I was so confused", (vec3s){PROJECTION_WIDTH*0.5 - (65*3.5*0.5), 112}, 7, YELLOW);
            text_renderer_append_text(text_renderer, "I had no choice, so I decided to go to the place",                  (vec3s){PROJECTION_WIDTH*0.5 - (48*3.5*0.5), 104}, 7, YELLOW);
            text_renderer_append_text(text_renderer, "where it was written, called 'CVillage'.",                          (vec3s){PROJECTION_WIDTH*0.5 - (40*3.5*0.5), 96},  7, YELLOW);

            if (global.timer->on_time) {
                scnce_fade_out(global.scnce);
            }
            
            if (global.scnce->faded && global.timer->busy) {
                timer_reset(global.timer);

                text_renderer->quad_count = 0;
                scnce_change_scnce(global.scnce, INGAME);
            }

        }
    }
 
    // TODO: Separate scnce fade layer and maybe gradient too
    { 
        quad_renderer_append_quad(quad_renderer[LAYER_TOP], (vec3s){global.scnce->camera->position.x,global.scnce->camera->position.y,0.0f}, 
                (vec2s){WIDTH, HEIGHT}, global.gradient);
        quad_renderer_append_quad(quad_renderer[LAYER_TOP], (vec3s){global.scnce->camera->position.x,global.scnce->camera->position.y,0.0f}, 
                (vec2s){WIDTH, HEIGHT}, (vec4s){0,0,0,global.scnce->fade_alpha});
    }

    text_renderer_render(text_renderer);
    quad_renderer_render(quad_renderer[LAYER_TOP]);

    if (global.toggle_editor) editor_render();
}


void cleanup(void) {
    pthread_mutex_destroy(&global.lock);
    asset_manager_destroy(global.asset_manager);
    scnce_destroy(global.scnce);

    line_renderer_destroy(line_renderer);
    text_renderer_destroy(text_renderer);

    for (u8 i = 0; i < LAYER_LAST; ++i) {
        quad_renderer_destroy(quad_renderer[i]);
    }
    free(quad_renderer);

    timer_destroy(global.timer);
    physics_destroy(global.physics);
    animation_destroy(global.animations);
    prefab_destroy();

    // debugging
    editor_destroy();

    LOG_TRACE("Window: Cleaning up");
}

int main(void) {
    struct Window window;
    window_init(&window, setup, update, cleanup);

    global.window = &window;
    window_loop(&window);

    window_destroy(&window);
    return 0;
}
