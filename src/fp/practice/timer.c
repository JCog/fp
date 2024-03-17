#include "timer.h"
#include "commands.h"
#include "fp.h"
#include "menu/menu.h"
#include "sys/gfx.h"
#include "sys/settings.h"

enum TimerMode timerMode = 0;
enum TimerState timerState = 0;
s64 timerCount = 0;
s32 timerLagFrames = 0;
u8 timerCutsceneTarget = 1;
u8 timerCutsceneCount = 0;
static s64 start = 0;
static s64 end = 0;
static u32 lagStart = 0;
static u32 lagEnd = 0;
static u16 frameStart = 0;
static u16 frameEnd = 0;
static bool prevCutsceneState = FALSE;
static u16 prevAreaID = 0;
static u16 prevMapID = 0;
static bool newMapWaiting = FALSE;

static s32 timerDrawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
    struct GfxFont *font = drawParams->font;
    s32 x = drawParams->x;
    s32 y = drawParams->y;
    s32 chHeight = menuGetCellHeight(item->owner, TRUE);

    switch (timerState) {
        case TIMER_RUNNING:
            timerCount = fp.cpuCounter - start;
            timerLagFrames = (pm_viFrames - lagStart) / 2 - (pm_gGameStatus.frameCounter - frameStart);
            break;
        case TIMER_STOPPED:
            timerCount = end - start;
            timerLagFrames = (lagEnd - lagStart) / 2 - (frameEnd - frameStart);
            break;
        case TIMER_INACTIVE:
        case TIMER_WAITING:
            timerCount = 0;
            timerLagFrames = 0;
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
    gfxPrintf(font, x, y + chHeight, "lag    %d", timerLagFrames >= 0 ? timerLagFrames : 0);
    if (timerMode != TIMER_MANUAL) {
        gfxPrintf(font, x, y + chHeight * 2, "cs/lz  %d/%d", timerCutsceneCount, timerCutsceneTarget);
    }
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

static s32 timerPositionProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_ACTIVATE) {
        fp.timerMoving = TRUE;
    } else if (reason == MENU_CALLBACK_DEACTIVATE) {
        fp.timerMoving = FALSE;
    }
    return menuGenericPositionProc(item, reason, &settings->timerX);
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
                if (settings->timerLogging) {
                    fpLog("timer started");
                }
            }
            break;
        case TIMER_RUNNING:
            if (timerMode == TIMER_CUTSCENE && !prevCutsceneState && inCutscene) {
                timerCutsceneCount++;
                if (settings->timerLogging && timerCutsceneCount != timerCutsceneTarget) {
                    fpLog("cutscene started");
                }
            } else if (timerMode == TIMER_LOADING_ZONE && newMap) {
                timerCutsceneCount++;
                if (settings->timerLogging && timerCutsceneCount != timerCutsceneTarget) {
                    fpLog("loading zone");
                }
            }
            if (timerCutsceneCount == timerCutsceneTarget) {
                timerState = TIMER_STOPPED;
                end = fp.cpuCounter;
                lagEnd = pm_viFrames;
                frameEnd = pm_gGameStatus.frameCounter;
                fpLog("timer stopped");
            }
            timerCount = fp.cpuCounter - start;
            timerLagFrames = (pm_viFrames - lagStart) / 2 - (pm_gGameStatus.frameCounter - frameStart);
            break;
        case TIMER_STOPPED:
            timerCount = end - start;
            timerLagFrames = (lagEnd - lagStart) / 2 - (frameEnd - frameStart);
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
        timerCutsceneCount = timerCutsceneTarget;
    } else if (timerState == TIMER_STOPPED) {
        timerState = TIMER_WAITING;
        timerCutsceneCount = 0;
        if (timerMode != TIMER_MANUAL) {
            fpLog("timer set to start");
        }
    }
}

void timerReset(void) {
    timerState = TIMER_INACTIVE;
    timerCutsceneCount = 0;
    fpLog("timer reset");
}

void createTimerMenu(struct Menu *menu) {
    s32 y = 0;
    s32 menuX = 15;

    /* initialize menu */
    menuInit(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu->selector = menuAddSubmenu(menu, 0, y++, NULL, "return");

    /*build menu*/
    menuAddStatic(menu, 0, y, "status", 0xC0C0C0);
    menuAddStaticCustom(menu, 7, y++, timerStatusDrawProc, NULL, 0xC0C0C0);
    menuAddStaticCustom(menu, 0, y++, timerDrawProc, NULL, 0xC0C0C0);
    y++;
    y++;
    struct MenuItem *startStopButton = menuAddButton(menu, 0, y, "start/stop", menuFuncProc, &timerStartStop);
    menuAddButton(menu, 11, y++, "reset", menuFuncProc, &timerReset);
    y++;
    menuAddStatic(menu, 0, y, "timer mode", 0xC0C0C0);
    struct MenuItem *optionsStart = menuAddOption(menu, menuX, y++,
                                                  "cutscene\0"
                                                  "loading zone\0"
                                                  "manual\0",
                                                  menuWordOptionmodProc, &timerMode);
    menuAddStatic(menu, 0, y, "cs/lz count", 0xC0C0C0);
    menuAddIntinput(menu, menuX, y++, 10, 2, menuByteModProc, &timerCutsceneTarget);
    menuAddStatic(menu, 0, y, "show timer", 0xC0C0C0);
    menuAddCheckbox(menu, menuX, y++, menuByteCheckboxProc, &settings->timerShow);
    menuAddStatic(menu, 0, y, "timer logging", 0xC0C0C0);
    menuAddCheckbox(menu, menuX, y++, menuByteCheckboxProc, &settings->timerLogging);
    menuAddStatic(menu, 0, y, "timer position", 0xC0C0C0);
    struct MenuItem *optionsEnd = menuAddPositioning(menu, menuX, y++, timerPositionProc, NULL);
    y++;
    struct MenuItem *saveButton = menuAddButton(menu, 0, y++, "save settings", fpSaveSettingsProc, NULL);

    menuItemAddChainLink(startStopButton, optionsStart, MENU_NAVIGATE_DOWN);
    menuItemAddChainLink(saveButton, optionsEnd, MENU_NAVIGATE_UP);
}
