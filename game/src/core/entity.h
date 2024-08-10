#ifndef ENTITY_H
#define ENTITY_H
#include "../util/types.h"
#include <cglm/types-struct.h>

typedef struct {
    vec2s size;
    vec3s position;
    vec4s color;
    s32 animation_id;
} Entity;


#endif
