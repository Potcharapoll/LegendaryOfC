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
    u32 rows;
    u32 cols;
    Entity *entities;
} Chunk;

Chunk *c1, *c2;
Entity player;

void setup(void) {
    asset_manager_init(&asset_manager);

    camera_init(&camera, (vec3s){0,0,0}, (vec2s){100,100});

    renderer_init(camera, asset_manager);

    player = (Entity){DEFAULT_SIZE, PLAYER_SPAWN_POSITION, WHITE};

    c1 = malloc(sizeof(*c1));
    c1->cols = 60;
    c1->rows = 60;
    c1->entities = malloc((c1->rows * c1->cols) * sizeof(Entity));

    c2 = malloc(sizeof(*c2));
    c2->cols = 60;
    c2->rows = 60;
    c2->entities = malloc((c2->rows * c2->cols) * sizeof(Entity));

    f32 _x = 0;
    f32 _y = 0;
    for (int y = 0; y < 60; y++) {
        for (int x = 0; x < 60; x++) {
            if ((int)_x % (32*60) == 0) {
                _x = 0;
            }
            c1->entities[y * 60 + x].color = GREEN;
            c1->entities[y * 60 + x].size  = DEFAULT_SIZE;
            c1->entities[y * 60 + x].pos   = (vec3s){_x, _y, 0};
            _x += 32;
        }
        _y += 32;
    }

    for (int i = 0; i < (60*60); i++) {
        printf("%f,%f,%f\n",c1->entities[i].pos.x, c1->entities[i].pos.y, c1->entities[i].pos.z);
        renderer_append_quad(TERRAIN_LAYER, c1->entities[i].size, c1->entities[i].pos, c1->entities[i].color);
    }
}

void update(void) {
    printf("FPS: %f\n",1 / window.delta_time);

    if (glfwGetKey(window.handle, GLFW_KEY_W) == GLFW_PRESS) {
        camera->position.y += camera->accel.y * window.delta_time;
    }
    else if (glfwGetKey(window.handle, GLFW_KEY_S) == GLFW_PRESS) {
        camera->position.y -= camera->accel.y * window.delta_time;
    }

    if (glfwGetKey(window.handle, GLFW_KEY_D) == GLFW_PRESS) {
        camera->position.x += camera->accel.x * window.delta_time;
    }
    else if (glfwGetKey(window.handle, GLFW_KEY_A) == GLFW_PRESS) {
        camera->position.x -= camera->accel.x * window.delta_time;
    }

    camera_update(camera);

    renderer_prepare();
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
 * Task
 * - Renderer
 * - Render Triangle
 * - 
 * */
