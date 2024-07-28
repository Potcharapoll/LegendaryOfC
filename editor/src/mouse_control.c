#include "mouse_control.h"
#include "../global.h"

void pickup_sprite(struct Sprite s) {
    renderer_add_sprite(global.renderer, s);
    global.state.holding = get_sprite_from_uid(global.state._uid - 1);
}

void place(void) {
    global.state.holding = NULL;
}
