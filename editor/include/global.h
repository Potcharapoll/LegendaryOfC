#ifndef GLOBAL_H
#define GLOBAL_H
#include "window.h"

typedef struct {
    struct Window *window;
} Global;

extern Global global;
#endif
