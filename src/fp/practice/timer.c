#include "timer.h"
#include "commands.h"
#include "fp.h"
#include "menu/menu.h"
#include "sys/gfx.h"
#include "sys/settings.h"

#define TIMER_LOG(...)            \
    if (settings->timerLogging) { \
        fpLog(__VA_ARGS__);       \
    }

#define PRIMARY_ACTIVE(bitIndex)       (settings->timerPrimaryEvents & (1 << bitIndex))
#define SPLIT_LOGGING_ACTIVE(bitIndex) (settings->timerLogSplitTime & (1 << bitIndex))
#define TOTAL_LOGGING_ACTIVE(bitIndex) (settings->timerLogTotalTime & (1 << bitIndex))

enum TimerEvents {
    EVENT_CUTSCENE,
    EVENT_LOADING_ZONE,
    EVENT_BATTLE_START,
    EVENT_BATTLE_END,
};

enum TimerState timerState = 0;
s64 timerCount = 0;
s32 timerLagFrames = 0;
s64 timerEventSplitTime = 0;
s64 timerEventTotalTime = 0;
s16 timerEventSplitCountdown = 0;
s16 timerEventTotalCountdown = 0;
u8 timerEventTarget = 1;
u8 timerEventCount = 0;
static s64 start = 0;
static s64 end = 0;
static s64 lastEventTime = 0;
static u32 lagStart = 0;
static u32 lagEnd = 0;
static u16 frameStart = 0;
static u16 frameEnd = 0;
static bool prevInputEnabled = FALSE;
static u16 prevAreaID = 0;
static u16 prevMapID = 0;
static s8 prevContext = 0;
static bool newMapWaiting = FALSE;
static bool endCutsceneWaiting = FALSE;

void timerDraw(s64 timerCount, struct GfxFont *font, s32 x, s32 y) {
    s32 hundredths = timerCount * 100 / fp.cpuCounterFreq;
    s32 seconds = hundredths / 100;
    s32 minutes = seconds / 60;
    s32 hours = minutes / 60;
    hundredths %= 100;
    seconds %= 60;
    minutes %= 60;
    if (hours > 0) {
        gfxPrintf(font, x, y, "%d:%02d:%02d.%02d", hours, minutes, seconds, hundredths);
    } else if (minutes > 0) {
        gfxPrintf(font, x, y, "%d:%02d.%02d", minutes, seconds, hundredths);
    } else {
        gfxPrintf(font, x, y, "%d.%02d", seconds, hundredths);
    }
}

static s32 timerDrawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
    struct GfxFont *font = drawParams->font;
    s32 x = drawParams->x;
    s32 y = drawParams->y;
    s32 chH = menuGetCellHeight(item->owner, TRUE);
    s32 chW = menuGetCellWidth(item->owner, TRUE);
    s32 xOffset = 9 * chW;

    gfxPrintf(font, x, y, "status");
    switch (timerState) {
        case TIMER_RUNNING:
            gfxPrintf(font, x + xOffset, y, "running");
            timerCount = fp.cpuCounter - start;
            timerLagFrames = (nuScRetraceCounter - lagStart) / 2 - (pm_gGameStatus.frameCounter - frameStart);
            break;
        case TIMER_STOPPED:
            gfxPrintf(font, x + xOffset, y, "stopped");
            timerCount = end - start;
            timerLagFrames = (lagEnd - lagStart) / 2 - (frameEnd - frameStart);
            break;
        case TIMER_INACTIVE:
            gfxPrintf(font, x + xOffset, y, "inactive");
            timerCount = 0;
            timerLagFrames = 0;
            timerEventTotalTime = 0;
            break;
        case TIMER_WAITING:
            gfxPrintf(font, x + xOffset, y, "waiting to start");
            timerCount = 0;
            timerLagFrames = 0;
            timerEventTotalTime = 0;
            break;
    }

    y += chH;
    gfxPrintf(font, x, y, "timer");
    timerDraw(timerCount, font, x + xOffset, y);
    y += chH;
    gfxPrintf(font, x, y, "event");
    timerDraw(timerEventTotalTime, font, x + xOffset, y);
    y += chH;
    gfxPrintf(font, x, y, "lag");
    gfxPrintf(font, x + xOffset, y, "%d", timerLagFrames >= 0 ? timerLagFrames : 0);
    y += chH;
    if (settings->timerPrimaryEvents) {
        gfxPrintf(font, x, y, "ev count");
        gfxPrintf(font, x + xOffset, y, "%d/%d", timerEventCount, timerEventTarget);
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

static s32 bitFieldHelper(struct MenuItem *item, enum MenuCallbackReason reason, u32 *bitField, s32 index) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        *bitField |= (1 << index);
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        *bitField &= ~(1 << index);
    } else if (reason == MENU_CALLBACK_THINK) {
        menuCheckboxSet(item, *bitField & (1 << index));
    }
    return 0;
}

static s32 primaryEventProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    return bitFieldHelper(item, reason, &settings->timerPrimaryEvents, (s32)data);
}

static s32 logSplitTimeProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    return bitFieldHelper(item, reason, &settings->timerLogSplitTime, (s32)data);
}

static s32 logTotalTimeProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    return bitFieldHelper(item, reason, &settings->timerLogTotalTime, (s32)data);
}

// waitVar = NULL, waitStartCond = FALSE, waitEndCond = FALSE to skip waiting
static bool updateSingleEvent(enum TimerEvents event, const char *logMsg, bool *waitVar, bool waitStartCond,
                              bool waitEndCond, bool eventCond) {
    bool newEvent = FALSE;
    if (PRIMARY_ACTIVE(event) && timerState == TIMER_WAITING) {
        if (waitVar == NULL) {
            if (eventCond) {
                newEvent = TRUE;
            }
        } else if (*waitVar) {
            if (waitEndCond) {
                *waitVar = FALSE;
                newEvent = TRUE;
            }
        } else if (waitStartCond) {
            *waitVar = TRUE;
        }
    } else {
        if (eventCond && timerState == TIMER_RUNNING) {
            if (PRIMARY_ACTIVE(event)) {
                newEvent = TRUE;
                TIMER_LOG(logMsg);
            }
            if (SPLIT_LOGGING_ACTIVE(event)) {
                timerEventSplitTime = fp.cpuCounter - lastEventTime;
                timerEventSplitCountdown = 30;
            }
            if (TOTAL_LOGGING_ACTIVE(event)) {
                timerEventTotalTime = fp.cpuCounter - start;
                timerEventTotalCountdown = 30;
            }
        }
    }
    return newEvent;
}

// returns 0 if no events triggered, 1 if at least 1 was, 2 if no events enabled
static s8 updateEvents() {
    if (!settings->timerPrimaryEvents) {
        return 2;
    }
    bool inputEnabled = pm_gPlayerStatus.inputEnabledCounter == 0;
    bool mapChanged = pm_gGameStatus.mapID != prevMapID || pm_gGameStatus.areaID != prevAreaID;
    bool battleStarted = pm_gGameStatus.context == 1 && prevContext == 0;
    bool battleEnded = pm_gGameStatus.context == 0 && prevContext == 1;

    bool newCutscene = updateSingleEvent(EVENT_CUTSCENE, "cutscene", &endCutsceneWaiting, !inputEnabled, inputEnabled,
                                         !inputEnabled && prevInputEnabled);
    bool newMap =
        updateSingleEvent(EVENT_LOADING_ZONE, "loading zone", &newMapWaiting, mapChanged, inputEnabled, mapChanged);
    bool newBattleStart = updateSingleEvent(EVENT_BATTLE_START, "battle start", NULL, FALSE, FALSE, battleStarted);
    bool newBattleEnd = updateSingleEvent(EVENT_BATTLE_END, "battle end", NULL, FALSE, FALSE, battleEnded);

    return newCutscene || newMap || newBattleStart || newBattleEnd;
}

void timerUpdate(void) {
    if (timerEventSplitCountdown) {
        timerEventSplitCountdown--;
    }
    if (timerEventTotalCountdown) {
        timerEventTotalCountdown--;
    }
    switch (timerState) {
        case TIMER_WAITING:
            if (updateEvents() == 0) {
                break;
            }
            timerState = TIMER_RUNNING;
            start = fp.cpuCounter;
            lastEventTime = start;
            lagStart = nuScRetraceCounter;
            frameStart = pm_gGameStatus.frameCounter;
            TIMER_LOG("timer started");
            break;
        case TIMER_RUNNING:
            if (updateEvents() == 1) {
                timerEventCount++;
                lastEventTime = fp.cpuCounter;
            }
            if (timerEventCount == timerEventTarget) {
                timerState = TIMER_STOPPED;
                end = fp.cpuCounter;
                lagEnd = nuScRetraceCounter;
                frameEnd = pm_gGameStatus.frameCounter;
                timerEventSplitCountdown = 0;
                timerEventTotalCountdown = 0;
                lastEventTime = 0;
                fpLog("timer stopped");
            }
            timerCount = fp.cpuCounter - start;
            timerLagFrames = (nuScRetraceCounter - lagStart) / 2 - (pm_gGameStatus.frameCounter - frameStart);
            break;
        case TIMER_STOPPED:
            timerCount = end - start;
            timerLagFrames = (lagEnd - lagStart) / 2 - (frameEnd - frameStart);
            break;
        case TIMER_INACTIVE: break;
    }

    prevInputEnabled = pm_gPlayerStatus.inputEnabledCounter == 0;
    prevAreaID = pm_gGameStatus.areaID;
    prevMapID = pm_gGameStatus.mapID;
    prevContext = pm_gGameStatus.context;
}

void timerStartStop(void) {
    if (timerState == TIMER_INACTIVE) {
        timerState = TIMER_WAITING;
        if (settings->timerPrimaryEvents) {
            fpLog("timer set to start");
        }
    } else if (timerState == TIMER_RUNNING) {
        timerEventCount = timerEventTarget;
    } else if (timerState == TIMER_STOPPED) {
        timerState = TIMER_WAITING;
        timerEventCount = 0;
        if (settings->timerPrimaryEvents) {
            fpLog("timer set to start");
        }
    }
}

void timerReset(void) {
    timerState = TIMER_INACTIVE;
    timerEventCount = 0;
    fpLog("timer reset");
}

static void createEntryMenuLine(struct Menu *menu, s32 y, const char *text, enum TimerEvents event) {
    menuAddStatic(menu, 0, y, text, 0xC0C0C0);
    menuAddCheckbox(menu, 14, y, primaryEventProc, (void *)event);
    menuAddCheckbox(menu, 22, y, logSplitTimeProc, (void *)event);
    menuAddCheckbox(menu, 28, y, logTotalTimeProc, (void *)event);
}

void createTimerMenu(struct Menu *menu) {
    s32 xOffset = 15;
    s32 y = 0;

    /* initialize menus */
    static struct Menu menuEvents;
    menuInit(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&menuEvents, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    /* build timer menu */
    menu->selector = menuAddSubmenu(menu, 0, y++, NULL, "return");
    menuAddStaticCustom(menu, 0, y++, timerDrawProc, NULL, 0xC0C0C0);
    y += 5;
    menuAddButton(menu, 0, y, "start/stop", menuFuncProc, &timerStartStop);
    menuAddButton(menu, 11, y++, "reset", menuFuncProc, &timerReset);
    struct MenuItem *buttonsEnd = menuAddSubmenu(menu, 0, y++, &menuEvents, "events");
    y++;
    menuAddStatic(menu, 0, y, "event count", 0xC0C0C0);
    struct MenuItem *optionsStart = menuAddIntinput(menu, xOffset, y++, 10, 2, menuByteModProc, &timerEventTarget);
    menuAddStatic(menu, 0, y, "show timer", 0xC0C0C0);
    menuAddCheckbox(menu, xOffset, y++, menuByteCheckboxProc, &settings->timerShow);
    menuAddStatic(menu, 0, y, "show events", 0xC0C0C0);
    menuAddCheckbox(menu, xOffset, y++, menuByteCheckboxProc, &settings->timerEventDisplay);
    menuAddStatic(menu, 0, y, "timer logging", 0xC0C0C0);
    menuAddCheckbox(menu, xOffset, y++, menuByteCheckboxProc, &settings->timerLogging);
    menuAddStatic(menu, 0, y, "timer position", 0xC0C0C0);
    struct MenuItem *optionsEnd = menuAddPositioning(menu, xOffset, y++, timerPositionProc, NULL);
    y++;
    struct MenuItem *saveButton = menuAddButton(menu, 0, y++, "save settings", fpSaveSettingsProc, NULL);

    menuItemAddChainLink(buttonsEnd, optionsStart, MENU_NAVIGATE_DOWN);
    menuItemAddChainLink(optionsStart, buttonsEnd, MENU_NAVIGATE_UP);
    menuItemAddChainLink(saveButton, optionsEnd, MENU_NAVIGATE_UP);

    /* build events menu */
    xOffset = 14;
    y = 0;
    menuEvents.selector = menuAddSubmenu(&menuEvents, 0, y++, NULL, "return");
    menuAddStatic(&menuEvents, xOffset, y++, "primary split total", 0xC0C0C0);
    createEntryMenuLine(&menuEvents, y++, "cutscenes", EVENT_CUTSCENE);
    createEntryMenuLine(&menuEvents, y++, "loading zones", EVENT_LOADING_ZONE);
    createEntryMenuLine(&menuEvents, y++, "battle start", EVENT_BATTLE_START);
    createEntryMenuLine(&menuEvents, y++, "battle end", EVENT_BATTLE_END);
}
