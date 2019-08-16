#include "commands.h"
#include "input.h"
#include "fp.h"

struct command fp_commands[COMMAND_CNT] = {
    {"show/hide menu",   COMMAND_PRESS,  0,   NULL},
    {"return from menu", COMMAND_PRESS,  0,   NULL},
    {"levitate",         COMMAND_HOLD,   0,   command_levitate},
    {"turbo",            COMMAND_PRESS,  0,   command_turbo},
    {"save position",    COMMAND_PRESS,  0,   command_save_pos},
    {"load position",    COMMAND_PRESS,  0,   command_load_pos},
    {"lzs",              COMMAND_PRESS,  0,   command_lzs},

};


void command_levitate(){
   pm_player.position.y += 1.0;
}

void command_turbo(){
    if(pm_player.run_speed == 4.0){
        pm_player.run_speed = 32.0;
    } else if(pm_player.run_speed == 32.0){
        pm_player.run_speed = 4.0;
    }
}

void command_save_pos(){
    fp.saved_x = pm_player.position.x;
    fp.saved_y = pm_player.position.y;
    fp.saved_z = pm_player.position.z;
    fp.saved_facing_angle = pm_player.facing_angle;
    fp.saved_movement_angle = pm_player.movement_angle;
}

void command_load_pos(){
    pm_player.position.x = fp.saved_x;
    pm_player.position.y = fp.saved_y;
    pm_player.position.z = fp.saved_z;
    pm_player.facing_angle = fp.saved_facing_angle;
    pm_player.movement_angle = fp.saved_movement_angle;
    
}

void command_lzs(){
    if(pm_unk1.saveblock_freeze == 0){
        pm_unk1.saveblock_freeze = 1;
    } else if(pm_unk1.saveblock_freeze == 1){
        pm_unk1.saveblock_freeze = 0;
    }

}