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

void timerUpdate(void);
void timerStartStop(void);
void timerReset(void);

void createTimerMenu(struct Menu *menu);

extern enum TimerMode timerMode;
extern enum TimerState timerState;
extern s64 timerCount;
extern s32 timerLagFrames;
extern u8 timerCutsceneTarget;
extern u8 timerCutsceneCount;

#endif
