#include "global.h"
#include "renderer.h"
#include "editor.h"
#include "ecs.h"

#pragma GCC diagnostic ignored "-Wmissing-braces"

#define DEBUG_INFO(...) printf("[info]: "); fprintf(stdout, __VA_ARGS__); putchar('\n');
#define DEBUG_EVENT(...) printf("[event]: "); fprintf(stdout, __VA_ARGS__); putchar('\n');
#define DEBUG_ERROR(...) printf("[error]: ") fprintf(stderr,  __VA_ARGS__); putchar('\n');

// ====================== ECS Components ============================
typedef struct {
    f32 x;
    f32 y;
    f32 z;
}positionComponent;

typedef struct {
    u32 textureId;
    u32 textureWidth;
    u32 textureHeight;
    u32 spriteWidth;
    u32 spriteHeight;
}spriteComponent;

typedef struct {
    b8 update : 1;
}updateComponent;

enum Components {
    POSITION_COMPONENT,
    SPRITE_COMPONENT,
    UPDATE_COMPONENT,

    COMPONENT_LAST
};
// ==================================================================

Global global;

void init(void) {
    // initialize imgui editor
    editor_init();
    
    // initialize renderer
    renderer_init();

    // initialize ecs
    ecs_init(COMPONENT_LAST, sizeof(positionComponent), sizeof(spriteComponent), sizeof(updateComponent));

    ecs_entity_t e;
    for (int y = 30; y < 300; y++) {
        for (int x = 30; x < 300; x++) {
            e = ecs_create();
            ecs_add(e.Id, POSITION_COMPONENT, &(positionComponent){x,y,0});
            ecs_add(e.Id, SPRITE_COMPONENT, &(spriteComponent){0,0,0,32,32});
            ecs_add(e.Id, UPDATE_COMPONENT, &(updateComponent){0});
        }
    }

    ecs_query_t query = ecs_query(COMPONENT_LAST, POSITION_COMPONENT, SPRITE_COMPONENT);
    for (u32 i = 0; i < query.len; i++) {
        positionComponent *pos = ecs_get(query.list[i], POSITION_COMPONENT);
        spriteComponent   *spr = ecs_get(query.list[i], SPRITE_COMPONENT);

        renderer_append_quad((vec2s){spr->spriteWidth, spr->spriteHeight}, 
                (vec3s){pos->x,pos->y,pos->z}, 
                (vec4s){1.0,1.0,1.0,1.0});
   }
}

void update(void) {
    if (glfwGetKey(global.window->handle, GLFW_KEY_0) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        DEBUG_EVENT("Change polygon mode to GL_LINE");
    }
    if (glfwGetKey(global.window->handle, GLFW_KEY_9) == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        DEBUG_EVENT("Change polygon mode to GL_FILL");
    }


    // update if need
    ecs_query_t query = ecs_query(COMPONENT_LAST, POSITION_COMPONENT, SPRITE_COMPONENT, UPDATE_COMPONENT);
    for (u32 i = 0; i < query.len; i++) {
        updateComponent *update = ecs_get(query.list[i], UPDATE_COMPONENT);
        if (update->update & 1) {
            // update to vertex
        }
    }

    renderer_prepare();
    renderer_render();

    editor_newframe();
    editor_render();
}

void cleanup(void) {
    ecs_destroy();
    editor_destroy(); 
    renderer_destroy();
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
// - Renderer            [Working 90%] 28/7/67 
// - Create canvas       [Working]
// - Place tile
// - Collision Lookup table O(1)
// - Tile Layers (LinkedList? Bit Fleid)
// - Serialization & Deserialization
