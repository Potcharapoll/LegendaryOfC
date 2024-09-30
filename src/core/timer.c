#include "timer.h"
#include "../global.h"
#include "../engine/logger.h"

#include <stdlib.h>

Timer *timer_init(void) {
    Timer *timer = malloc(sizeof(*timer));
    if (timer == NULL) {
        LOG_FETAL("Cannot allocate memory for timer");
    }

    timer->target = 0.0f;
    timer->time = 0.0f;
    timer->on_time = false;
    timer->busy = false;

    LOG_TRACE("Timer: Successfully created timer");
    return timer;
}

void timer_destroy(Timer *timer) {
    free(timer);

    LOG_TRACE("Timer: Successfully destroyed timer");
}

void timer_start(Timer *timer, f32 target) {
    if (timer->busy) return;

    timer->target = target;   
    timer->busy = true;
}

void timer_update(Timer *timer) {
    if (timer->on_time || !timer->busy) return;

    if (timer->time < timer->target) {
        timer->time += global.dt;
    }
    else {
        timer->on_time = true;
    }

    LOG_DEBUG("Timer: At %f / %f", timer->time, timer->target);
}

void timer_reset(Timer *timer) {
    timer->time = 0.0f;
    timer->target = 0.0f;
    timer->on_time = false;
    timer->busy = false;
}
