#ifndef COMPONENTS_H
#define COMPONENTS_H
#include "../util/types.h"

typedef struct {
    f32 x, y, z;
}Position;

typedef struct {
    u32 textureId;
    u32 spriteWidth;
    u32 spriteHeight;
}Sprite;

typedef struct {
    u8 update_flag;
}Updatable;

typedef enum {
    UPDATE_POSITION  = (1 << 0),
    UPDATE_TEXTURE   = (1 << 1),
    UPDATE_SIZE      = (1 << 2),
} UpdateFlag;

typedef enum {
    POSITION_COMPONENT,
    SPRITE_COMPONENT,
    UPDATABLE_COMPONENT,

    COMPONENT_LAST
}Components;

#endif
