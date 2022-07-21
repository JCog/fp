#ifndef COMMANDS_H
#define COMMANDS_H
#include "common.h"

enum Commands {
    COMMAND_MENU,
    COMMAND_RETURN,
    COMMAND_LEVITATE,
    COMMAND_TURBO,
    COMMAND_SAVEPOS,
    COMMAND_LOADPOS,
    COMMAND_LZS,
    COMMAND_RELOAD,
    COMMAND_RELOAD_LAST_WARP,
    COMMAND_TOGGLE_WATCHES,
    COMMAND_REIMPORT_SAVE,
    COMMAND_SAVE_GAME,
    COMMAND_LOAD_GAME,
    COMMAND_START_TIMER,
    COMMAND_RESET_TIMER,
    COMMAND_SHOW_HIDE_TIMER,
    COMMAND_BREAK_FREE,
    COMMAND_MAX
};

enum ActivationType {
    COMMAND_HOLD,
    COMMAND_PRESS,
    COMMAND_PRESS_ONCE
};

struct Command {
    const char *text;
    enum ActivationType commandType;
    u16 bind;
    void (*proc)();
};

void showMenu(void);
void hideMenu(void);
void commandLevitateProc(void);
void commandTurboProc(void);
void commandSavePosProc(void);
void commandLoadPosProc(void);
void commandLzsProc(void);
void commandReloadProc(void);
void commandReloadLastWarpProc(void);
void commandToggleWatchesProc(void);
void commandImportSaveProc(void);
void commandSaveGameProc(void);
void commandLoadGameProc(void);
void commandStartTimerProc(void);
void commandResetTimerProc(void);
void commandShowHideTimerProc(void);
void commandBreakFreeProc(void);

extern struct Command fpCommands[COMMAND_MAX];

#endif
