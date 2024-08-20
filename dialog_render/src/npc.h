#ifndef NPC_H
#define NPC_H
#include "types.h"

struct NPC {
    char name[24];
    u32 dialog_id;
    u32 quest_id;
};

#endif
