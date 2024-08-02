#include "global.h"
#include "core/components.h"
#include "core/ecs.h"
#include "util/log.h"
#pragma GCC diagnostic ignored "-Wmissing-braces"

Global global;

void init(void) {
    // global settings
    global.settings.projection_size = (vec2s){1280.0f, 768.0f};

    // initialize asset manager
    global.asset_manager = asset_manager_init();

    // initialize imgui editor
    editor_init(&global.editor_state.editor);

    // initialize camera
    camera_init(&global.camera, (vec3s){0.0,0.0,0.0}, (vec2s){200.0f, 200.0f});
    
    // initialize renderer
    renderer_init(&global.renderer);

    // initialize ecs
    ecs_init(COMPONENT_LAST, sizeof(positionComponent), sizeof(spriteComponent), sizeof(updateComponent));
}

void update(void) {
    static const f32 DELAY = 0.1f;
    static b8 toggle_mode = false;
    static f32 delay = 0.0f;

    if (glfwGetKey(global.window->handle, GLFW_KEY_P) == GLFW_PRESS && glfwGetKey(global.window->handle, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && delay <= 0) {
        toggle_mode = !toggle_mode;
        glPolygonMode(GL_FRONT_AND_BACK, (!toggle_mode) ? GL_LINE : GL_FILL);
        LOG_EVENT("Change polygon mode to %s", (!toggle_mode) ? "GL_LINE" : "GL_FILL");
        delay = DELAY;
    }
    else {
        delay -= global.window->delta_time;
    }

    int state = glfwGetMouseButton(global.window->handle, GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS || state == GLFW_REPEAT) {
        global.tile_update = true;
    }
    else {
        global.tile_update = false;
    }

    // Camera movement
    if (glfwGetKey(global.window->handle, GLFW_KEY_W) == GLFW_PRESS) {
        global.camera->position.y += floor(global.camera->accel.y * global.window->delta_time);
    }
    else if (glfwGetKey(global.window->handle, GLFW_KEY_S) == GLFW_PRESS) {
        global.camera->position.y -= floor(global.camera->accel.y * global.window->delta_time);
    }
    if (glfwGetKey(global.window->handle, GLFW_KEY_A) == GLFW_PRESS) {
        global.camera->position.x -= floor(global.camera->accel.x * global.window->delta_time);
    }
    else if (glfwGetKey(global.window->handle, GLFW_KEY_D) == GLFW_PRESS) {
        global.camera->position.x += floor(global.camera->accel.x * global.window->delta_time);
    }

    camera_update(global.camera);

    // calculate ortho mouse coordinate
    {
        vec4s mouse = {global.window->mouse.normalx, global.window->mouse.normaly, 0.0f, 1.0f};
        ViewProj inv_view_proj = get_inverse_view_proj(global.camera);

        mat4s inv_vp = glms_mat4_mul(inv_view_proj.view, inv_view_proj.proj);
        mouse = glms_mat4_mulv(inv_vp, mouse);
        global.ortho_mouse.x = mouse.x;
        global.ortho_mouse.y = mouse.y;
    }

    renderer_prepare();
    renderer_render(global.renderer);

    editor_newframe();
    editor_render();
}

void cleanup(void) {
    ecs_destroy();
    editor_destroy(global.editor_state.editor); 
    camera_destroy(global.camera);
    renderer_destroy(global.renderer);
    asset_manager_destroy(global.asset_manager);
}

int main(void) {
    struct Window window;

    if (window_init(&window, init, update, cleanup)) { 
        global.window = &window;
        window_loop(&window); 
    }
    window_destroy(&window);
    return 0;
}

// TODO;
// - Fix tile cover rect [Done]        27/7/67
// - ECS                 [Done]        28/7/67
// - Renderer            [Done]        28/7/67 - 29/7/67
// - Camera              [Done]        29/7/67    
// - Asset Manager       [Done]        30/7/67 - 01/8/67
// - Tileset             [Done]        01/8/67 - 02/8/67
// - Create canvas       
//      = Manual         [Done]        29/7/67
//      = Auto           [Done]        29/7/67
//      = Resize         [Not Yet]     
// - Place tile          [half Done]   30/7/67 **MUST REFACTOR
// - Collision Lookup table O(1)
// - Tile Layers (LinkedList? Bit Fleid)
// - Serialization & Deserialization
//
// Make renderer and ecs connected
// Update vertex by update flag in ecs through render_update 
// Make i batch render for i layer
// Make layer view
