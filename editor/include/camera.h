#ifndef CAMERA_H
#define CAMERA_H
#include <cglm/struct.h>
#include "types.h"

typedef struct {
    mat4s view;
    mat4s proj;
}ViewProj;

typedef struct {
    vec3s position;
    vec2s accel;

    vec3s front, up;
    ViewProj view_proj;
    ViewProj inverse_view_proj;
    f32 zoom;
}Camera;

void camera_init(Camera **camera, vec3s position, vec2s accel);
void camera_destroy(Camera *camera);
void camera_update(Camera *camera);
ViewProj get_view_proj(Camera *camera);
ViewProj get_inverse_view_proj(Camera *camera);
#endif
