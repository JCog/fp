#ifndef TIMER_H
#define TIMER_H
#include "menu.h"

enum TimerMode {
    TIMER_AUTO,
    TIMER_MANUAL,
};

enum TimerState {
    TIMER_INACTIVE,
    TIMER_WAITING,
    TIMER_RUNNING,
    TIMER_STOPPED,
};

enum TimerState timerGetState(void);
s64 timerGetTimerCount(void);
s32 timerGetLagFrames(void);

void timerUpdate(void);
void timerStart(void);
void timerReset(void);

void createTimerMenu(struct Menu *menu);

#endif
