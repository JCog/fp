#include "commands.h"
#include "input.h"
#include "fp.h"
#include "settings.h"

struct command fp_commands[COMMAND_MAX] = {
    {"show/hide menu",   COMMAND_PRESS,       0,   NULL},
    {"return from menu", COMMAND_PRESS,       0,   NULL},
    {"levitate",         COMMAND_HOLD,        0,   levitate_proc},
    {"turbo",            COMMAND_PRESS,       0,   turbo_proc},
    {"save position",    COMMAND_PRESS,       0,   save_pos_proc},
    {"load position",    COMMAND_PRESS,       0,   load_pos_proc},
    {"lzs",              COMMAND_PRESS,       0,   lzs_proc},
};

void levitate_proc(){
    pm_player.animation |= 3;
    pm_player.y_speed = 11;
    pm_player.y_snap = -0.75;
    pm_player.frames_in_air = 1;
}

void turbo_proc(){
    if(pm_player.run_speed == 4.0){
        pm_player.run_speed = 32.0;
    } else if(pm_player.run_speed == 32.0){
        pm_player.run_speed = 4.0;
    }
}

void save_pos_proc(){
    fp.saved_x = pm_player.position.x;
    fp.saved_y = pm_player.position.y;
    fp.saved_z = pm_player.position.z;
    fp.saved_facing_angle = pm_player.facing_angle;
    fp.saved_movement_angle = pm_player.movement_angle;
}

void load_pos_proc(){
    pm_player.position.x = fp.saved_x;
    pm_player.position.y = fp.saved_y;
    pm_player.position.z = fp.saved_z;
    pm_player.facing_angle = fp.saved_facing_angle;
    pm_player.movement_angle = fp.saved_movement_angle;
    
}

void lzs_proc(){
    if(pm_unk1.saveblock_freeze == 0){
        pm_unk1.saveblock_freeze = 1;
    } else if(pm_unk1.saveblock_freeze == 1){
        pm_unk1.saveblock_freeze = 0;
    }

}