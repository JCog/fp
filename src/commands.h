#ifndef _COMMANDS_H
#define _COMMANDS_H

#include "pm64.h"

#define COMMAND_CNT 10

enum command_type{
    COMMAND_HOLD,
    COMMAND_PRESS
};

struct command {
    const char         *text;
    enum command_type   type;
    uint16_t            bind;
    void              (*proc)();
};

void command_levitate();
void command_turbo();
void command_levitate();
void command_save_pos();
void command_load_pos();
void command_lzs();

extern struct command fp_commands[COMMAND_CNT];

#endif