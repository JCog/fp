#ifndef TIMER_H
#define TIMER_H
#include "menu/menu.h"

enum TimerMode {
    TIMER_CUTSCENE,
    TIMER_LOADING_ZONE,
    TIMER_MANUAL,
};

enum TimerState {
    TIMER_INACTIVE,
    TIMER_WAITING,
    TIMER_RUNNING,
    TIMER_STOPPED,
};

enum TimerMode timerGetMode(void);
enum TimerState timerGetState(void);
s64 timerGetTimerCount(void);
s32 timerGetLagFrames(void);
u8 timerGetCutsceneTarget(void);
u8 timerGetCutsceneCount(void);

void timerUpdate(void);
void timerStartStop(void);
void timerReset(void);

void createTimerMenu(struct Menu *menu);

#endif
