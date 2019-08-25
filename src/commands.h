#ifndef _COMMANDS_H
#define _COMMANDS_H

#include "pm64.h"

enum commands{
    COMMAND_MENU,
    COMMAND_RETURN,
    COMMAND_LEVITATE,
    COMMAND_TURBO,
    COMMAND_SAVEPOS,
    COMMAND_LOADPOS,
    COMMAND_LZS,
    COMMAND_MAX
};

enum activation_type{
    COMMAND_HOLD,
    COMMAND_PRESS,
    COMMAND_PRESS_ONCE
};

struct command {
    const char             *text;
    enum activation_type   command_type;
    uint16_t               bind;
    void                   (*proc)();
};

void show_menu();
void hide_menu();
void levitate_proc();
void turbo_proc();
void levitate_proc();
void save_pos_proc();
void load_pos_proc();
void lzs_proc();

extern struct command fp_commands[COMMAND_MAX];

#endif