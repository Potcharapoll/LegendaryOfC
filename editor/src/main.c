#include "global.h"
#include "camera.h"
#include "editor.h"
#include "ecs.h"
#include "components.h"
#include "log.h"
#pragma GCC diagnostic ignored "-Wmissing-braces"

Global global;

void init(void) {
    // global settings
    global.settings.projection_size = (vec2s){1280.0f, 768.0f};

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
    if (glfwGetKey(global.window->handle, GLFW_KEY_0) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        LOG_EVENT("Change polygon mode to GL_LINE");
    }
    if (glfwGetKey(global.window->handle, GLFW_KEY_9) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        LOG_EVENT("Change polygon mode to GL_FILL");
    }
    if (glfwGetKey(global.window->handle, GLFW_KEY_Z) == GLFW_PRESS) {
        global.camera->zoom *= 1.10;
    }
    if (glfwGetKey(global.window->handle, GLFW_KEY_X) == GLFW_PRESS) {
        global.camera->zoom *= 0.90;
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

    // update if need
    /* ecs_query_t query = ecs_query(COMPONENT_LAST, POSITION_COMPONENT, SPRITE_COMPONENT, UPDATE_COMPONENT); */
    /* for (u32 i = 0; i < query.len; i++) { */
    /*     updateComponent *update = ecs_get(query.list[i], UPDATE_COMPONENT); */
    /*     if (update->update & 1) { */
    /*         // update to vertex */
    /*     } */
    /* } */

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
// - Create canvas       
//      - Manual         [Done]        29/7/67
//      - Auto           [Done]        29/7/67
//      - Resize         [Not Yet]  
// - Place tile
// - Collision Lookup table O(1)
// - Tile Layers (LinkedList? Bit Fleid)
// - Serialization & Deserialization
