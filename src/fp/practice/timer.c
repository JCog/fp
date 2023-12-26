#include "timer.h"
#include "commands.h"
#include "fp.h"
#include "menu/menu.h"
#include "sys/gfx.h"
#include "sys/settings.h"

static s64 start = 0;
static s64 end = 0;
static u32 lagStart = 0;
static u32 lagEnd = 0;
static u16 frameStart = 0;
static u16 frameEnd = 0;
static bool prevCutsceneState = FALSE;
static enum TimerMode timerMode = 0;
static enum TimerState timerState = 0;
static u8 cutsceneTarget = 1;
static u8 cutsceneCount = 0;
static s64 timerCount = 0;
static s32 lagFrames = 0;
static u16 prevAreaID = 0;
static u16 prevMapID = 0;
static bool newMapWaiting = FALSE;

static s32 byteModProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    u8 *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menuIntinputGet(item) != *p) {
            menuIntinputSet(item, *p);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        *p = menuIntinputGet(item);
    }
    return 0;
}

static s32 timerModeProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    enum TimerMode *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menuOptionGet(item) != *p) {
            menuOptionSet(item, *p);
        }
    } else if (reason == MENU_CALLBACK_DEACTIVATE) {
        *p = menuOptionGet(item);
    }
    return 0;
}

static s32 showTimerProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        settings->bits.timerShow = 1;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        settings->bits.timerShow = 0;
    } else if (reason == MENU_CALLBACK_THINK) {
        menuCheckboxSet(item, settings->bits.timerShow);
    }
    return 0;
}

static s32 timerLoggingProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        settings->bits.timerLogging = 1;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        settings->bits.timerLogging = 0;
    } else if (reason == MENU_CALLBACK_THINK) {
        menuCheckboxSet(item, settings->bits.timerLogging);
    }
    return 0;
}

static void startStopProc(struct MenuItem *item, void *data) {
    timerStartStop();
}

static void resetProc(struct MenuItem *item, void *data) {
    timerReset();
}

static s32 timerDrawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
    struct GfxFont *font = drawParams->font;
    s32 x = drawParams->x;
    s32 y = drawParams->y;
    s32 chHeight = menuGetCellHeight(item->owner, TRUE);

    switch (timerState) {
        case TIMER_RUNNING:
            timerCount = fp.cpuCounter - start;
            lagFrames = (pm_viFrames - lagStart) / 2 - (pm_gGameStatus.frameCounter - frameStart);
            break;
        case TIMER_STOPPED:
            timerCount = end - start;
            lagFrames = (lagEnd - lagStart) / 2 - (frameEnd - frameStart);
            break;
        case TIMER_INACTIVE:
        case TIMER_WAITING:
            timerCount = 0;
            lagFrames = 0;
            break;
    }

    s32 hundredths = timerCount * 100 / fp.cpuCounterFreq;
    s32 seconds = hundredths / 100;
    s32 minutes = seconds / 60;
    s32 hours = minutes / 60;
    hundredths %= 100;
    seconds %= 60;
    minutes %= 60;
    if (hours > 0) {
        gfxPrintf(font, x, y, "timer  %d:%02d:%02d.%02d", hours, minutes, seconds, hundredths);
    } else if (minutes > 0) {
        gfxPrintf(font, x, y, "timer  %d:%02d.%02d", minutes, seconds, hundredths);
    } else {
        gfxPrintf(font, x, y, "timer  %d.%02d", seconds, hundredths);
    }
    gfxPrintf(font, x, y + chHeight, "lag    %d", lagFrames >= 0 ? lagFrames : 0);
    return 1;
}

static s32 timerStatusDrawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
    struct GfxFont *font = drawParams->font;
    s32 x = drawParams->x;
    s32 y = drawParams->y;

    switch (timerState) {
        case TIMER_INACTIVE: gfxPrintf(font, x, y, "inactive"); break;
        case TIMER_WAITING: gfxPrintf(font, x, y, "waiting to start"); break;
        case TIMER_RUNNING: gfxPrintf(font, x, y, "running"); break;
        case TIMER_STOPPED: gfxPrintf(font, x, y, "stopped"); break;
    }
    return 1;
}

enum TimerState timerGetState(void) {
    return timerState;
}

s64 timerGetTimerCount(void) {
    return timerCount;
}

s32 timerGetLagFrames(void) {
    return lagFrames;
}

void timerUpdate(void) {
    bool inCutscene = pm_gPlayerStatus.flags & 0x00002000;
    bool newMap = pm_gGameStatus.mapID != prevMapID || pm_gGameStatus.areaID != prevAreaID;

    switch (timerState) {
        case TIMER_WAITING:
            if (timerMode == TIMER_LOADING_ZONE) {
                if (newMap) {
                    newMapWaiting = TRUE;
                }
            } else {
                newMapWaiting = FALSE;
            }
            if (timerMode == TIMER_MANUAL || (prevCutsceneState && !inCutscene)) {
                if (timerMode == TIMER_LOADING_ZONE && !newMapWaiting) {
                    break;
                }
                timerState = TIMER_RUNNING;
                newMapWaiting = FALSE;
                start = fp.cpuCounter;
                lagStart = pm_viFrames;
                frameStart = pm_gGameStatus.frameCounter;
                if (settings->bits.timerLogging) {
                    fpLog("timer started");
                }
            }
            break;
        case TIMER_RUNNING:
            if (timerMode == TIMER_CUTSCENE && !prevCutsceneState && inCutscene) {
                cutsceneCount++;
                if (settings->bits.timerLogging && cutsceneCount != cutsceneTarget) {
                    fpLog("cutscene started");
                }
            } else if (timerMode == TIMER_LOADING_ZONE && newMap) {
                cutsceneCount++;
                if (settings->bits.timerLogging && cutsceneCount != cutsceneTarget) {
                    fpLog("loading zone");
                }
            }
            if (cutsceneCount == cutsceneTarget) {
                timerState = TIMER_STOPPED;
                end = fp.cpuCounter;
                lagEnd = pm_viFrames;
                frameEnd = pm_gGameStatus.frameCounter;
                fpLog("timer stopped");
            }
            timerCount = fp.cpuCounter - start;
            lagFrames = (pm_viFrames - lagStart) / 2 - (pm_gGameStatus.frameCounter - frameStart);
            break;
        case TIMER_STOPPED:
            timerCount = end - start;
            lagFrames = (lagEnd - lagStart) / 2 - (frameEnd - frameStart);
            break;
        case TIMER_INACTIVE: break;
    }

    prevCutsceneState = pm_gPlayerStatus.flags & 0x00002000;
    prevAreaID = pm_gGameStatus.areaID;
    prevMapID = pm_gGameStatus.mapID;
}

void timerStartStop(void) {
    if (timerState == TIMER_INACTIVE) {
        timerState = TIMER_WAITING;
        if (timerMode != TIMER_MANUAL) {
            fpLog("timer set to start");
        }
    } else if (timerState == TIMER_RUNNING) {
        cutsceneCount = cutsceneTarget;
    } else if (timerState == TIMER_STOPPED) {
        timerState = TIMER_WAITING;
        cutsceneCount = 0;
        if (timerMode != TIMER_MANUAL) {
            fpLog("timer set to start");
        }
    }
}

void timerReset(void) {
    timerState = TIMER_INACTIVE;
    cutsceneCount = 0;
    fpLog("timer reset");
}

void createTimerMenu(struct Menu *menu) {
    s32 y = 0;
    s32 menuX = 14;

    /* initialize menu */
    menuInit(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu->selector = menuAddSubmenu(menu, 0, y++, NULL, "return");

    /*build menu*/
    menuAddStatic(menu, 0, y, "status", 0xC0C0C0);
    menuAddStaticCustom(menu, 7, y++, timerStatusDrawProc, NULL, 0xC0C0C0);
    menuAddStaticCustom(menu, 0, y++, timerDrawProc, NULL, 0xC0C0C0);
    y++;
    menuAddButton(menu, 0, y, "start/stop", startStopProc, NULL);
    menuAddButton(menu, 11, y++, "reset", resetProc, NULL);
    y++;
    menuAddStatic(menu, 0, y, "timer mode", 0xC0C0C0);
    menuAddOption(menu, menuX, y++,
                  "cutscene\0"
                  "loading zone\0"
                  "manual\0",
                  timerModeProc, &timerMode);
    menuAddStatic(menu, 0, y, "cs/lz count", 0xC0C0C0);
    menuAddIntinput(menu, menuX, y++, 10, 2, byteModProc, &cutsceneTarget);
    menuAddStatic(menu, 0, y, "show timer", 0xC0C0C0);
    menuAddCheckbox(menu, menuX, y++, showTimerProc, NULL);
    menuAddStatic(menu, 0, y, "timer logging", 0xC0C0C0);
    menuAddCheckbox(menu, menuX, y++, timerLoggingProc, NULL);
    y++;
    menuAddButton(menu, 0, y++, "save settings", fpSaveSettingsProc, NULL);
}
