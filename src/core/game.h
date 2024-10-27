#ifndef GAME_H
#define GAME_H
#include "../util/types.h"
#include "../core/physics.h"
#include "dialog.h"
#include "chunk.h"

// 19 slots for global game state and 13 slots free 
#define GAME_STATE_ACT1            ((u32)1 << 0)
#define GAME_STATE_ACT2            ((u32)1 << 1)
#define GAME_STATE_ACT3            ((u32)1 << 2)
#define GAME_STATE_ACT4            ((u32)1 << 3)
#define GAME_STATE_LOCK_TUNNEL     ((u32)1 << 4)
#define GAME_STATE_LOCK_RESTAURANT ((u32)1 << 5)
#define GAME_STATE_LOCK_FISH       ((u32)1 << 6) 
#define GAME_STATE_LOCK_LIBRARY    ((u32)1 << 7)
#define GAME_STATE_LOCK_CHURCH     ((u32)1 << 8)
#define GAME_STATE_LOCK_OG_HOME    ((u32)1 << 9)
#define GAME_STATE_LOCK_LJ_HOME    ((u32)1 << 10)
#define GAME_STATE_LOCK_VC_HOME    ((u32)1 << 11)
#define GAME_STATE_SHOW_ROXY       ((u32)1 << 12)
#define GAME_STATE_SHOW_EMMA       ((u32)1 << 13)
#define GAME_STATE_SHOW_PARMY      ((u32)1 << 14)
#define GAME_STATE_SHOW_NATHAN     ((u32)1 << 15)
#define GAME_STATE_SHOW_VC         ((u32)1 << 16)
#define GAME_STATE_SHOW_JOEY       ((u32)1 << 17)
#define GAME_STATE_SHOW_TOM        ((u32)1 << 18)
#define GAME_STATE_SHOW_INTERACT   ((u32)1 << 19)
#define GAME_STATE_TOGGLE_MAN_PAGE ((u32)1 << 20)

// act 1 flag (3 slots)
#define ACT1_ROXY_TALKED ((u32)1 << 21)
#define ACT1_OG_HOME_KEY ((u32)1 << 22)
#define ACT1_GET_G_IMAGE ((u32)1 << 23)
// (if all cleard will be go to the next act)
// -> lock all expect for restaurant, show only emma and roxy
// 00000000000000000011111111010001

// act 2 flag (3 slots)
#define ACT2_EMMA_TALKED   ((u32)1 << 21)
#define ACT2_NATHAN_TALKED ((u32)1 << 22)
#define ACT2_PARMY_TALKED  ((u32)1 << 23)
// (the player will get the info when talk to the last one)
// -> unlock all expect for tunnel, show only emma, roxy, nathan, and parmy
// 00000000000000001111000000000010

// act 3 flags (1 slots)
#define ACT3_TOM_TALKED ((u32)1 << 21)
// -> unlock all expect for tunnel, show only emma, roxy, nathan, parmy, and tom
// 00000000000001001111000000000100

// act 4 flags (2 slots)
#define ACT4_VC_QUEST     ((u32)1 << 21)
#define ACT4_FISH_GET     ((u32)1 << 22)
#define ACT4_FINISH_QUEST ((u32)1 << 23)
#define ACT4_TUNNEL_KEY   ((u32)1 << 24)
// -> unlock all expect for tunnel, show all
// (if all are true and talk to vc again will go to the next act)
// (tunnel will unlock when player interact to it)
// 00000000000001111111000000001000

#define ACT2_TOTAL_QUESTION 20
#define ACT3_TOTAL_QUESTION 20
#define ACT4_TOTAL_QUESTION 12

#define ACT3_QUESTION_COUNT 5
#define ACT2_QUESTION_COUNT 5
#define ACT4_QUESTION_COUNT 5

enum GameAct {
    GAME_ACT1,
    GAME_ACT2,
    GAME_ACT3,
    GAME_ACT4
};

void game_init(void);
void game_destroy(void);
void game_render(Body *player_body);
void game_update(void);

void game_setup_act(enum GameAct act);
void game_attach_dialog(DialogPacket *packet);

void game_get_dialog_tag(char buf[static 60], DialogPacket *packet);

void game_change_chunk(Body *player_body, Chunks chunk_id, vec2s target_coord);

// call everytime at the end of dialog to update game state flag 
void game_update_dialog_state(char *tag);

// unlock after talking to Roxy in Act 1
void game_toggle_man_page(void);

// toggle game state flag
void game_state_toggle(u32 flag);
void game_state_on(u32 flag);
void game_state_off(u32 flag);

DialogText* game_get_act_dialog(enum GameAct act);

b8 game_state_check(u32 flag);
u8 game_get_act(void);
#endif

