#ifndef COMMANDS_H
#define COMMANDS_H
#include "pm64.h"

enum commands {
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
    COMMAND_TRICK,
    COMMAND_SAVE_GAME,
    COMMAND_LOAD_GAME,
    COMMAND_START_TIMER,
    COMMAND_RESET_TIMER,
    COMMAND_SHOW_HIDE_TIMER,
    COMMAND_MAX
};

enum activation_type {
    COMMAND_HOLD,
    COMMAND_PRESS,
    COMMAND_PRESS_ONCE
};

struct command {
    const char *text;
    enum activation_type command_type;
    uint16_t bind;
    void (*proc)();
};

void show_menu();
void hide_menu();
void command_levitate_proc();
void command_turbo_proc();
void command_levitate_proc();
void command_save_pos_proc();
void command_load_pos_proc();
void command_lzs_proc();
void command_reload_proc();
void command_reload_last_warp_proc();
void command_toggle_watches_proc();
void command_import_save_proc();
void command_save_game_proc();
void command_load_game_proc();
void command_start_timer_proc();
void command_reset_timer_proc();
void command_show_hide_timer_proc();

extern struct command fp_commands[COMMAND_MAX];

#endif
