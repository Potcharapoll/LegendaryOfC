#ifndef CAMERA_H
#define CAMERA_H
#include <cglm/struct.h>

struct ViewProj {
    mat4s view;
    mat4s proj;
};

struct Camera {
    vec3s position;
    vec2s accel;

    vec3s front;
    vec3s up;
    struct ViewProj view_proj;
    struct ViewProj inverse_view_proj;
};

void camera_init(struct Camera **camera, vec3s position, vec2s accel);
void camera_destroy(struct Camera *camera);
void camera_update(struct Camera *camera);
void camera_center_to_obj(struct Camera *camera, vec3s obj, vec2s size);
struct ViewProj get_view_proj(struct Camera *camera);
struct ViewProj get_inverse_view_proj(struct Camera *camera);
#endif
