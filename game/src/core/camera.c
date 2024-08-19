#include "camera.h"
#include "../defs.h"
#include "../util/log.h"

void camera_init(struct Camera **camera, vec3s position, vec2s accel) {
    *camera = malloc(sizeof(**camera));
    if (*camera == NULL) {
        LOG_FETAL("Failed to initialize camera");
        abort();
    }

    (*camera)->accel    = accel;
    (*camera)->position = position;

    (*camera)->front = (vec3s){0.0f, 0.0f, -1.0f};
    (*camera)->up    = (vec3s){0.0f, 1.0f, 0.0f};

    (*camera)->view_proj.view = glms_lookat((*camera)->position, glms_vec3_add((*camera)->front, (*camera)->position), (*camera)->up);
    (*camera)->view_proj.proj = glms_ortho(0.0f, PROJECTION_WIDTH, 0.0f, PROJECTION_HEIGHT, 0.0f, 100.0f);

    (*camera)->inverse_view_proj.view = glms_mat4_inv((*camera)->view_proj.view);
    (*camera)->inverse_view_proj.proj = glms_mat4_inv((*camera)->view_proj.proj);
}

void camera_update(struct Camera *camera) {
    camera->up = (vec3s){0.0f, 0.0f, 0.0f};

    camera->view_proj.view         = glms_lookat(camera->position, glms_vec3_add(camera->front, camera->position), camera->up);
    camera->inverse_view_proj.view = glms_mat4_inv(camera->view_proj.view);
}

void camera_destroy(struct Camera *camera) {
    free(camera);

    LOG_DEBUG("struct Camera destroyed");
}

struct ViewProj get_view_proj(struct Camera *camera) {
    return camera->view_proj;
}

struct ViewProj get_inverse_view_proj(struct Camera *camera) {
    return camera->inverse_view_proj;
}
