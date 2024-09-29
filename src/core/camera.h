#ifndef CAMERA_H
#define CAMERA_H
#include <cglm/struct.h>

struct ViewProj {
    mat4s view;
    mat4s proj;
};

typedef struct Camera {
    vec2s position;
    vec3s front;
    vec3s up;
    struct ViewProj view_proj;
    struct ViewProj inverse_view_proj;
}Camera;

Camera *camera_init(vec2s position);
void camera_destroy(Camera *camera);
void camera_update(Camera *camera);
void camera_center_to_obj(Camera *camera, vec2s obj, vec2s size);
struct ViewProj get_view_proj(Camera *camera);
struct ViewProj get_inverse_view_proj(Camera *camera);
#endif
