#include "camera.h"
#include "../defs.h"
#include "../engine/logger.h"

struct Camera *camera_init(vec2s position) {
    struct Camera *camera = malloc(sizeof(*camera));
    ASSERT(camera != NULL, "Failed to allocate memory for camera", __FILE__, __LINE__);

    camera->position = position;
    camera->front    = (vec3s){0.0f, 0.0f, -1.0f};
    camera->up       = (vec3s){0.0f, 1.0f, 0.0f};

    LOG_TRACE("Camera: Successfully initialized camera");

    return camera;
}

void camera_update(struct Camera *camera) {
    camera->up = (vec3s){0.0f, 1.0f, 0.0f};

    vec3s position = {camera->position.x, camera->position.y, 0.0f};
    camera->view_proj.view = glms_lookat(position, glms_vec3_add(camera->front, position), camera->up);

    camera->view_proj.proj = glms_ortho(0.0f, PROJECTION_WIDTH, 0.0f, PROJECTION_HEIGHT, 0.0f, 100.0f);
    camera->inverse_view_proj.view = glms_mat4_inv(camera->view_proj.view);
    camera->inverse_view_proj.proj = glms_mat4_inv(camera->view_proj.proj);

    camera->view_proj.view         = glms_lookat(position, glms_vec3_add(camera->front, position), camera->up);
    camera->inverse_view_proj.view = glms_mat4_inv(camera->view_proj.view);
}

void camera_center_to_obj(struct Camera *camera, vec2s obj, vec2s size) {
    camera->position.x = obj.x - PROJECTION_WIDTH /2.0f - size.x / 2.0f;
    camera->position.y = obj.y - PROJECTION_HEIGHT/2.0f - size.y / 2.0f;
}

void camera_destroy(struct Camera *camera) {
    free(camera);

    LOG_TRACE("Camera: Successfully destroyed camera");
}

struct ViewProj get_view_proj(struct Camera *camera) {
    return camera->view_proj;
}

struct ViewProj get_inverse_view_proj(struct Camera *camera) {
    return camera->inverse_view_proj;
}
