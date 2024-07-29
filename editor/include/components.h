#ifndef COMPONENTS_H
#define COMPONENTS_H
#include "types.h"

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

#endif
