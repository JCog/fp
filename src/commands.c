#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include "commands.h"
#include "input.h"
#include "fp.h"
#include "settings.h"
#include "tricks.h"

struct command fp_commands[COMMAND_MAX] = {
    {"show/hide menu",   COMMAND_PRESS_ONCE,  0,   NULL},
    {"return from menu", COMMAND_PRESS_ONCE,  0,   NULL},
    {"levitate",         COMMAND_HOLD,        0,   command_levitate_proc},
    {"turbo",            COMMAND_PRESS_ONCE,  0,   command_turbo_proc},
    {"save position",    COMMAND_PRESS_ONCE,  0,   command_save_pos_proc},
    {"load position",    COMMAND_PRESS_ONCE,  0,   command_load_pos_proc},
    {"lzs",              COMMAND_PRESS_ONCE,  0,   command_lzs_proc},
    {"reload room",      COMMAND_PRESS_ONCE,  0,   command_reload_proc},
    {"show coordinates", COMMAND_PRESS_ONCE,  0,   command_coords_proc},
    {"load trick",       COMMAND_PRESS_ONCE,  0,   command_trick_proc},
    {"save game",        COMMAND_PRESS_ONCE,  0,   command_save_game_proc},
    {"load game",        COMMAND_PRESS_ONCE,  0,   command_load_game_proc},
    {"start timer",      COMMAND_PRESS_ONCE,  0,   command_start_timer_proc},
    {"reset timer",      COMMAND_PRESS_ONCE,  0,   command_reset_timer_proc}
};

void show_menu() {
    menu_signal_enter(fp.main_menu, MENU_SWITCH_SHOW);
    fp.menu_active = 1;
    input_reserve(BUTTON_D_UP | BUTTON_D_DOWN | BUTTON_D_LEFT | BUTTON_D_RIGHT | BUTTON_L);
    input_reservation_set(1);
}

void hide_menu() {
    menu_signal_leave(fp.main_menu, MENU_SWITCH_HIDE);
    fp.menu_active = 0;
    input_free(BUTTON_D_UP | BUTTON_D_DOWN | BUTTON_D_LEFT | BUTTON_D_RIGHT | BUTTON_L);
    input_reservation_set(0);
}

void fp_log(const char *fmt, ...) {
    struct log_entry *ent = &fp.log[SETTINGS_LOG_MAX - 1];
    if (ent->msg) {
        free(ent->msg);
    }
    for (int i = SETTINGS_LOG_MAX - 1; i > 0; --i) {
        fp.log[i] = fp.log[i - 1];
    }
      
    va_list va;
    va_start(va, fmt);
    int l = vsnprintf(NULL, 0, fmt, va);
    va_end(va);

    ent = &fp.log[0];
    ent->msg = malloc(l + 1);
    if (!ent->msg) {
        return;
    }
    va_start(va, fmt);
    vsprintf(ent->msg, fmt, va);
    va_end(va);
    ent->age = 0;
}

void fp_warp(uint16_t group, uint16_t room, uint16_t entrance) {
    //this should prevent most warp crashes, but eventually it'd be ideal to figure out how to warp properly
    if ((pm_player.flags & (1 << 5)) || (pm_status.is_battle && pm_unk6.menu_open)) {
        fp_log("can't warp with menu open");
        return;
    }
    //would be nice to know why warping from this room crashes
    if (pm_status.group_id == 0 && pm_status.room_id == 0xe) {
        fp_log("can't warp from here");
        return;
    }
    pm_PlayAmbientSounds(-1, 0);
    pm_status.loading_zone_tangent = 0;
    pm_status.group_id = group;
    pm_status.room_id = room;
    pm_status.entrance_id = entrance;

    pm_unk2.room_change_state = 1;

    uint32_t val = 0x80035DFC;
    pm_warp.room_change_ptr = val;

    return;
}

void fp_set_story_progress(int story_progress) {
    pm_unk3.story_progress = story_progress;
}

void fp_set_global_flag(int flag_index, _Bool value) {
    int word_index = flag_index / 32;
    int bit = flag_index % 32;
    uint32_t *p = pm_flags.global_flags;
    if (value)
        p[word_index] |= (1 << bit);
    else
        p[word_index] &= ~(1 << bit);
}

void fp_set_global_byte(int byte_index,  int8_t value) {
    pm_flags.global_bytes[byte_index] = value;
}

void command_levitate_proc() {
    if (pm_status.peach_flags == 0) {
        pm_player.flags |= 3;
        pm_player.y_speed = 11;
        pm_player.y_snap = -0.75;
        pm_player.frames_in_air = 1;
    }
}

void command_turbo_proc() {
    if (fp.turbo) {
        fp.turbo = 0;
        fp_log("turbo disabled");
    }
    else {
        fp.turbo = 1;
        fp_log("turbo enabled");
    }
}

void command_save_pos_proc() {
    fp.saved_x = pm_player.position.x;
    fp.saved_y = pm_player.position.y;
    fp.saved_z = pm_player.position.z;
    fp.saved_facing_angle = pm_player.facing_angle;
    fp.saved_movement_angle = pm_player.movement_angle;
    fp_log("position saved");
}

void command_load_pos_proc() {
    pm_player.position.x = fp.saved_x;
    pm_player.position.y = fp.saved_y;
    pm_player.position.z = fp.saved_z;
    pm_player.facing_angle = fp.saved_facing_angle;
    pm_player.movement_angle = fp.saved_movement_angle;
    fp_log("position loaded");
}

void command_lzs_proc() {
    if(pm_unk1.saveblock_freeze == 0) {
        pm_unk1.saveblock_freeze = 1;
        fp_log("lzs enabled");
    } else if(pm_unk1.saveblock_freeze == 1) {
        pm_unk1.saveblock_freeze = 0;
        fp_log("lzs disabled");
    }
}

void command_reload_proc() {
    fp_warp(pm_status.group_id, pm_status.room_id, pm_status.entrance_id);
}

void command_coords_proc() {
    fp.coord_active = !fp.coord_active;
}

void command_trick_proc() {
    load_trick(fp.saved_trick);
}

void command_save_game_proc() {
    pm_SaveGame();
    pm_PlaySfx(0x10);
    fp_log("saved to slot %d", pm_status.save_slot);
}

void command_start_timer_proc() {
    if (fp.timer.state == 0) {
        fp.timer.state = 1;
        fp_log("timer set to start");
    }
    else if (fp.timer.state == 3) {
        fp.timer.state = 1;
        fp.timer.cutscene_count = 0;
        fp_log("timer set to start");
    }
}

void command_reset_timer_proc() {
    fp.timer.state = 0;
    fp.timer.cutscene_count = 0;
    fp_log("timer reset");
}

void command_load_game_proc() {
    if ((pm_player.flags & (1 << 5)) || (pm_status.is_battle && pm_unk6.menu_open)) {
        fp_log("can't load with menu open");
        return;
    }
    if (pm_status.group_id == 0 && pm_status.room_id == 0xe) {
        fp_log("can't load from here");
        return;
    }
    pm_LoadGame(pm_status.save_slot);
    fp_warp(pm_status.group_id, pm_status.room_id, pm_status.entrance_id);
    fp_log("loaded from slot %d", pm_status.save_slot);
}