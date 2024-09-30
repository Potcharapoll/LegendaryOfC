#include "core/dialog.h"
#pragma GCC diagnostic ignored "-Wmissing-braces"

#include "core/timer.h"
#include "engine/logger.h"

#include "core/asset_manager.h"
#include "core/animation.h"
#include "core/renderer.h"
#include "core/player.h"
#include "core/prefab.h"
#include "core/scene.h"

#include "gfx/window.h"

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

// SUGGEST: Maybe attach the renderer to scene to make it can render text and fade when we want, and also the fade layer
// SUGGEST: Change from physics (Static_Body, Body) to ECS

Dialog *text_dialog;

typedef enum {
    LAYER_BASE,
    LAYER_BASE_UPPER,
    LAYER_STRUCTURE,
    LAYER_TOP,

    LAYER_LAST
} RenderLayer;

#ifdef DEBUG
LineRenderer *line_renderer;

static void _append_collider(void) {
    Body *body; 
    for (u32 i = 0; i < global.physics->body_list->len; i++) {
        body = physics_body_get(global.physics, i);
        line_renderer_append_aabb(line_renderer, body->aabb, GREEN);
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
#endif

static QuadRenderer **quad_renderer;

static ivec2s _get_char_coord(char c) {
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
    Chunk *chunk = global.scene->chunk;

    if ((body->collision_flag & COLLISION_LAYER_TELEPORTER) == COLLISION_LAYER_TELEPORTER) {
        for (u8 i = 0; i < chunk->teleporter_count; ++i) {
            Static_Body *teleporter_body = physics_static_body_get(global.physics, chunk->teleporter[i].body_id);

            if (body == teleporter_body) {
                other->velocity = glms_vec2_zero();
                player_set_animation(IDLE, global.PlayerState.direction);

                if (global.scene->fade_state == FADE_NONE) global.scene->fade_state = FADE_OUT;
                if (global.scene->faded) scene_change_chunk(global.scene, other, chunk->teleporter[i].chunkId, chunk->teleporter[i].target_coord);
            }
        }
        return;
    }

    if (body->collision_flag == COLLISION_LAYER_DIALOG) {
        LOG_DEBUG("Physics: Hit the dialog layer");
    }
}

static void input_handling(void) {
    if (global.scene->fade_state == FADE_NONE) player_input();

#ifdef DEBUG
    if (global.input_delay >= INPUT_DELAY) {
        if (window_get_key(global.window, GLFW_KEY_X)) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            global.input_delay = 0.0f;
        }
        else if (window_get_key(global.window, GLFW_KEY_Z)) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            global.input_delay = 0.0f;
        }

        if (window_get_key(global.window, GLFW_KEY_I)) {
            global.toggle_collision = !global.toggle_collision;
            global.input_delay = 0.0f;
        }
        if (window_get_key(global.window, GLFW_KEY_U)) {
            global.toggle_show_collider = !global.toggle_show_collider;
            global.input_delay = 0.0f;
        }
        if (window_get_key(global.window, GLFW_KEY_Y)) {
            global.toggle_editor = !global.toggle_editor;
            global.input_delay = 0.0f;
        }

        if (window_get_mouse_button(global.window, GLFW_MOUSE_BUTTON_LEFT)) {

            switch (global.cursor_mode) {
                case CURSOR_MODE_START_POINT:
                    global.start_point[0] = global.window->mouse.orthox;
                    global.start_point[1] = global.window->mouse.orthoy;
                    break;
                case CURSOR_MODE_END_POINT:
                    global.end_point[0] = global.window->mouse.orthox;
                    global.end_point[1] = global.window->mouse.orthoy;
                    break;
                default:
                    break;
            }
            global.input_delay = 0.0f;
        }
        if (window_get_mouse_button(global.window, GLFW_MOUSE_BUTTON_RIGHT)) {
            global.cursor_mode = (global.cursor_mode + 1) % CURSOR_MODE_LAST;
            global.input_delay = 0.0f;
        }
    }
#endif
}

void setup(void) {
    global.collision_callback = _collision_callback;
    global.get_char_coord = _get_char_coord;

    pthread_mutex_init(&global.lock, NULL);
    global.asset_manager = asset_manager_init();

    // Separate "default_shader" and "texture_shader" because default_shader is used with 8 slot textures with RGBa,
    // otherwise "texture_shader" is used only with 1 slot textures alpha.
    asset_manager_push_shader(global.asset_manager, "default_shader", "res/shaders/default.vert", "res/shaders/default.frag");
    asset_manager_push_shader(global.asset_manager, "texture_shader", "res/shaders/texture.vert", "res/shaders/texture.frag");

    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_TEXT,         81, (ivec2s){27, 3}, (ivec2s){32,32});
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_PLAYER,       32, (ivec2s){ 8, 4}, (ivec2s){16,22});
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_NPC,          21, (ivec2s){ 7, 3}, (ivec2s){16,23});
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_TILE,         56, (ivec2s){ 8, 7}, (ivec2s){16,16});
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_INSIDE,     1692, (ivec2s){47,36}, (ivec2s){16,16});
    asset_manager_push_spritesheet(global.asset_manager, TEXTURE_STRUCTURES,  368, (ivec2s){18,23}, (ivec2s){16,16});

    global.timer = timer_init();
    global.physics = physics_init(10);
    global.animations = animation_init();

    player_init();
    prefab_init();

    global.scene = scene_init();
    scene_change_scene(global.scene, MENU);

    quad_renderer = malloc(LAYER_LAST * sizeof(quad_renderer));
    for (u8 i = 0; i < LAYER_LAST; ++i) {
        quad_renderer[i] = quad_renderer_init();
    }

#ifdef DEBUG 
    global.cursor_mode = CURSOR_MODE_NORMAL;
    glm_vec2_zero(global.start_point);
    glm_vec2_zero(global.end_point);

    asset_manager_push_shader(global.asset_manager, "line_shader",    "res/shaders/line.vert",    "res/shaders/line.frag");
    line_renderer = line_renderer_init();
    editor_init();

    scene_change_scene(global.scene, INGAME);
#endif

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
    glCullFace(GL_BACK);

    text_dialog = dialog_create();

    DialogText text = {.text = "Here we go again! I Sus!"};
    dialog_append(text_dialog, "KEY", DIALOG_TYPE_TEXT, &text);
    dialog_append(text_dialog, "MEY", DIALOG_TYPE_TEXT, &text);
    dialog_append(text_dialog, "ABC", DIALOG_TYPE_TEXT, &text);
}

void update(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0,0.0,0.0,1.0);

    global.input_delay += global.dt;

    Body *player_body = physics_body_get(global.physics, global.PlayerState.body_id);

    scene_update(global.scene, player_body);
    timer_update(global.timer);

    if (global.scene->scene_state == INGAME) { 

        input_handling();

        animation_update(global.animations, global.dt);
        physics_update(global.physics, global.dt);

        // base layer
        struct Spritesheet *spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_TILE);
        for (s32 y = 0; y < CHUNK_SIZE_Y; y++) {
            for (s32 x = 0; x < CHUNK_SIZE_X; x++) {
                u32 uv = global.scene->chunk->uv[CHUNK_SIZE_X * y + x]; 

                if (uv == (u32)-1) { continue; }

                u32 row    = uv / spritesheet->grid_size.x;
                u32 col    = uv % spritesheet->grid_size.x;
                f32 cellx  = spritesheet->cell_size.x / spritesheet->size.x;
                f32 celly  = spritesheet->cell_size.y / spritesheet->size.y; 

                f32 tex_coord[4] = {
                    (cellx * col), 
                    (cellx * col) + cellx, 
                    (celly * row), 
                    (celly * row) + celly
                };
                vec3s position   = {
                    global.scene->chunk->position.x + x * TILE_SIZE, 
                    global.scene->chunk->position.y + y * TILE_SIZE, 
                    0.0
                };
                quad_renderer_append_quad_texture(quad_renderer[LAYER_BASE], position, DEFAULT_SCALE, WHITE, spritesheet->texture, tex_coord);
            }
        }

        // base upper layer
        u32 offset = CHUNK_SIZE_Y * CHUNK_SIZE_X;
        for (s32 y = 0; y < CHUNK_SIZE_Y; y++) {
            for (s32 x = 0; x < CHUNK_SIZE_X; x++) {
                u32 uv = global.scene->chunk->uv[offset + CHUNK_SIZE_X * y + x]; 

                if (uv == 0) continue;

                u32 row    = uv / spritesheet->grid_size.x;
                u32 col    = uv % spritesheet->grid_size.x;
                f32 cellx  = spritesheet->cell_size.x / spritesheet->size.x;
                f32 celly  = spritesheet->cell_size.y / spritesheet->size.y; 

                f32 tex_coord[4] = {
                    (cellx * col), 
                    (cellx * col) + cellx, 
                    (celly * row), 
                    (celly * row) + celly
                };
                vec3s position   = {global.scene->chunk->position.x + x * TILE_SIZE, global.scene->chunk->position.y + y * TILE_SIZE, 0.0};
                quad_renderer_append_quad_texture(quad_renderer[LAYER_BASE_UPPER], position, DEFAULT_SCALE, WHITE, spritesheet->texture, tex_coord);
            }
        }

        // prefab
        for (u8 i = 0; i < global.scene->chunk->prefab_count; ++i) {
            quad_renderer_append_prefab(quad_renderer[LAYER_STRUCTURE], global.scene->chunk->prefab[i].coord, global.scene->chunk->prefab[i].name);
        }


        { // append player to LAYER_TOP
            struct Spritesheet *player_spritesheet = asset_manager_get_spritesheet(global.asset_manager, TEXTURE_PLAYER);
            f32 tex_coord[4];

            player_get_tex_coord(tex_coord);
            quad_renderer_append_quad_texture(quad_renderer[LAYER_TOP], (vec3s){player_body->position.x,player_body->position.y, 0.0f}, 
                    PLAYER_SIZE, WHITE, player_spritesheet->texture, tex_coord);
        }

#ifdef DEBUG
        { // append start_point, end_point, and cursor to LAYER_TOP
            quad_renderer_append_quad(quad_renderer[LAYER_TOP], (vec3s){global.start_point[0], global.start_point[1], 0.0f}, (vec2s){1,1}, GREEN);
            quad_renderer_append_quad(quad_renderer[LAYER_TOP], (vec3s){global.end_point[0], global.end_point[1], 0.0f}, (vec2s){1,1}, BLUE);
            quad_renderer_append_quad(quad_renderer[LAYER_TOP], (vec3s){global.window->mouse.orthox, global.window->mouse.orthoy, 0.0f}, (vec2s){1,1}, WHITE);
        }
#endif

        // render all layer by order
        for (u8 i = 0; i < LAYER_LAST; ++i) {
            quad_renderer_render(quad_renderer[i]);
        }
    }
    else {
        if (global.input_delay >= INPUT_DELAY && window_get_key(global.window, GLFW_KEY_SPACE)) {
            scene_fade_out(global.scene);
            global.input_delay = 0.0f;
        }

        if (global.scene->scene_state == MENU) {
            if (global.scene->faded) { 
                scene_change_scene(global.scene, INTRO); 
            }
        }
        else { 
            if (!global.scene->faded && !global.timer->busy) {
                timer_start(global.timer, 5.0f);
            } 

            if (global.timer->on_time) {
                scene_fade_out(global.scene);
            }
            
            if (global.scene->faded && global.timer->busy) {
                timer_reset(global.timer);
                scene_change_scene(global.scene, INGAME);

            }

        }
    }
 
    scene_render(global.scene);

#ifdef DEBUG
    if (!global.scene->dialog) scene_attach_dialog(global.scene, text_dialog);
        if (global.toggle_show_collider) _append_collider();
        line_renderer_render(line_renderer);

        if (global.toggle_editor) editor_render();
#endif
}


void cleanup(void) {
    pthread_mutex_destroy(&global.lock);
    asset_manager_destroy(global.asset_manager);
    scene_destroy(global.scene);

#ifdef DEBUG
    line_renderer_destroy(line_renderer);
    editor_destroy();
#endif
    for (u8 i = 0; i < LAYER_LAST; ++i) {
        quad_renderer_destroy(quad_renderer[i]);
    }
    free(quad_renderer);

    timer_destroy(global.timer);
    physics_destroy(global.physics);
    animation_destroy(global.animations);
    prefab_destroy();

    dialog_delete(text_dialog);

    LOG_TRACE("Window: Cleaning up");
}

int main(void) {
#ifdef DEBUG
    LOG_INFO("LegendaryOfC Version 0.1 DEBUG Mode");
#else
    LOG_INFO("LegendaryOfC Version 0.1 Release Mode");
#endif

    struct Window window;
    window_init(&window, setup, update, cleanup);

    global.window = &window;
    window_loop(&window);

    window_destroy(&window);
    return 0;
}
