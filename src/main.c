#include "core/asset_manager.h"
#include "core/camera.h"
#include "core/player.h"
#include "core/renderer.h"
#include "core/animation.h"
#include "core/physics.h"

#include "global.h"
#include "defs.h"

// TODO: Text Render    -- ON GOING --
//       Dialog System  -- PLANNED  --
//       Teleport       -- PLANNED  --
//       Map            -- PLANNED  --


struct Global global;

static void border_collision(vec2s *a, vec2s size, vec4s position) {
    if (a->y < position.y) a->y = position.y;
    if (a->x < position.x) a->x = position.x;

    if (a->x + size.x > position.z) a->x = position.z - size.x;
    if (a->y + size.y > position.w) a->y = position.w - size.y;
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
    renderer_init();
}

void update(void) {
    input_handling();

    printf("FPS: %f\n", 1 / global.dt);

    { // Camera
        Body *player_body = physics_body_get(global.PlayerState.body_id);
        camera_center_to_obj(global.camera, player_body->position, PLAYER_HITBOX);

        border_collision(&global.camera->position, (vec2s){PROJECTION_WIDTH, PROJECTION_HEIGHT}, global.ChunkState.chunk->position);
        border_collision(&player_body->position, PLAYER_HITBOX, global.ChunkState.chunk->position);

        camera_update(global.camera); 
    }

    physics_update(global.dt);
    animation_update(global.dt);

    renderer_prepare();
    renderer_render();
}

void cleanup(void) {
    asset_manager_destroy(global.asset_manager);
    renderer_destroy();
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
