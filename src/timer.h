#ifndef TIMER_H
#define TIMER_H
#include "menu.h"

enum timer_mode {
    TIMER_AUTO,
    TIMER_MANUAL,
};

enum timer_state {
    TIMER_INACTIVE,
    TIMER_WAITING,
    TIMER_RUNNING,
    TIMER_STOPPED,
};

enum timer_state timer_get_state(void);
s64 timer_get_timer_count(void);
s32 timer_get_lag_frames(void);

void timer_update(void);
void timer_start(void);
void timer_reset(void);

void create_timer_menu(struct menu *menu);

#endif
