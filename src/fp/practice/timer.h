#ifndef TIMER_H
#define TIMER_H
#include "menu/menu.h"

enum TimerState {
    TIMER_INACTIVE,
    TIMER_WAITING,
    TIMER_RUNNING,
    TIMER_STOPPED,
};

void timerDraw(s64 timerCount, struct GfxFont *font, s32 x, s32 y);
void timerUpdate(void);
void timerStartStop(void);
void timerReset(void);

void createTimerMenu(struct Menu *menu);

extern enum TimerState timerState;
extern s64 timerCount;
extern s32 timerLagFrames;
extern s64 timerEventSplitTime;
extern s64 timerEventTotalTime;
extern s16 timerEventSplitCountdown;
extern s16 timerEventTotalCountdown;
extern u8 timerEventTarget;
extern u8 timerEventCount;

#endif
