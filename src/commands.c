#include "commands.h"
#include "input.h"
#include "fp.h"

struct command kz_commands[COMMAND_CNT] = {
    {"show/hide menu",   COMMAND_PRESS,  0,   NULL},
    {"return from menu", COMMAND_PRESS,  0,   NULL},
    {"levitate",         COMMAND_HOLD,   0,   command_levitate},
    {"turbo",            COMMAND_HOLD,   0,   command_turbo},
    {"save position",    COMMAND_PRESS,  0,   command_save_pos},
    {"load position",    COMMAND_PRESS,  0,   command_load_pos},

};


void command_levitate(){
   
}

void command_turbo(){
    
}

void command_save_pos(){
    
}

void command_load_pos(){
    
}