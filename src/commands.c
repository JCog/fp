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
#include "watchlist.h"

#if PM64_VERSION==PM64U
    #define RM_CHNGE_PTR 0x8003617C
#else
    #define RM_CHNGE_PTR 0x80035DFC
#endif

struct command fp_commands[COMMAND_MAX] = {
    {"show/hide menu",   COMMAND_PRESS_ONCE,  0, NULL},
    {"return from menu", COMMAND_PRESS_ONCE,  0, NULL},
    {"levitate",         COMMAND_HOLD,        0, command_levitate_proc},
    {"turbo",            COMMAND_PRESS_ONCE,  0, command_turbo_proc},
    {"save position",    COMMAND_PRESS_ONCE,  0, command_save_pos_proc},
    {"load position",    COMMAND_PRESS_ONCE,  0, command_load_pos_proc},
    {"lzs",              COMMAND_PRESS_ONCE,  0, command_lzs_proc},
    {"reload room",      COMMAND_PRESS_ONCE,  0, command_reload_proc},
    {"reload last warp", COMMAND_PRESS_ONCE,  0, command_reload_last_warp_proc},
    {"toggle watches",   COMMAND_PRESS_ONCE,  0, command_toggle_watches_proc},
    {"load trick",       COMMAND_PRESS_ONCE,  0, command_trick_proc},
    {"save game",        COMMAND_PRESS_ONCE,  0, command_save_game_proc},
    {"load game",        COMMAND_PRESS_ONCE,  0, command_load_game_proc},
    {"start/stop timer", COMMAND_PRESS_ONCE,  0, command_start_timer_proc},
    {"reset timer",      COMMAND_PRESS_ONCE,  0, command_reset_timer_proc},
    {"show/hide timer",  COMMAND_PRESS_ONCE,  0, command_show_hide_timer_proc}
};

void show_menu() {
    menu_signal_enter(fp.main_menu, MENU_SWITCH_SHOW);
    fp.menu_active = 1;
    input_reserve(BUTTON_D_UP | BUTTON_D_DOWN | BUTTON_D_LEFT | BUTTON_D_RIGHT | BUTTON_L);
}

void hide_menu() {
    menu_signal_leave(fp.main_menu, MENU_SWITCH_HIDE);
    fp.menu_active = 0;
    input_free(BUTTON_D_UP | BUTTON_D_DOWN | BUTTON_D_LEFT | BUTTON_D_RIGHT | BUTTON_L);
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

_Bool fp_warp(uint16_t group, uint16_t room, uint16_t entrance) {
    //this should prevent most warp crashes, but eventually it'd be ideal to figure out how to warp properly
    if ((pm_player.flags & (1 << 5)) || (pm_status.is_battle && pm_unk6.menu_open)) {
        fp_log("can't warp with menu open");
        return 0;
    }
    //would be nice to know why warping from this room crashes
    if (pm_status.group_id == 0 && pm_status.room_id == 0xe) {
        fp_log("can't warp from here");
        return 0;
    }
    pm_PlayAmbientSounds(-1, 0);
    pm_status.loading_zone_tangent = 0;
    pm_status.group_id = group;
    pm_status.room_id = room;
    pm_status.entrance_id = entrance;

    pm_unk2.room_change_state = 1;

    uint32_t val = RM_CHNGE_PTR;
    pm_warp.room_change_ptr = val;

    return 1;
}

void set_flag(uint32_t *flags, int flag_index, _Bool value) {
    int word_index = flag_index / 32;
    int bit = flag_index % 32;
    if (value) {
        flags[word_index] |= (1 << bit);
    }
    else {
        flags[word_index] &= ~(1 << bit);
    }
}

void fp_set_global_flag(int flag_index, _Bool value) {
    set_flag(pm_save_data.global_flags, flag_index, value);
}

void fp_set_area_flag(int flag_index, _Bool value) {
    set_flag(pm_save_data.area_flags, flag_index, value);
}

void fp_set_enemy_defeat_flag(int flag_index, _Bool value) {
    set_flag(pm_enemy_flags.enemy_defeat_flags, flag_index, value);
}

void fp_set_global_byte(int byte_index,  int8_t value) {
    pm_save_data.global_bytes[byte_index] = value;
}

void command_levitate_proc() {
    if (!(pm_status.peach_flags & (1 << 0))) {
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
    if (pm_unk1.saveblock_freeze == 0) {
        pm_unk1.saveblock_freeze = 1;
        fp_log("easy lzs enabled");
    }
    else if (pm_unk1.saveblock_freeze == 1) {
        pm_unk1.saveblock_freeze = 0;
        fp_log("easy lzs disabled");
    }
}

void command_reload_proc() {
    fp_warp(pm_status.group_id, pm_status.room_id, pm_status.entrance_id);
}

void command_reload_last_warp_proc() {
    if (fp.saved_group != 0x1c) {
        fp_warp(fp.saved_group, fp.saved_room, fp.saved_entrance);
    }
}

void command_toggle_watches_proc() {
    settings->bits.watches_visible = !settings->bits.watches_visible;
    if (settings->bits.watches_visible) {
        watchlist_show(fp.menu_watchlist);
    }
    else {
        watchlist_hide(fp.menu_watchlist);
    }
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
        if (fp.timer.mode == 0) {
            fp_log("timer set to start");
        }
    }
    else if (fp.timer.state == 2 && fp.timer.mode == 1) {
        fp.timer.cutscene_count = fp.timer.cutscene_target;
    }
    else if (fp.timer.state == 3) {
        fp.timer.state = 1;
        fp.timer.cutscene_count = 0;
        if (fp.timer.mode == 0) {
            fp_log("timer set to start");
        }
    }
}

void command_reset_timer_proc() {
    fp.timer.state = 0;
    fp.timer.cutscene_count = 0;
    fp_log("timer reset");
}

void command_show_hide_timer_proc() {
    settings->bits.timer_show = !settings->bits.timer_show;
}

int _save_not_empty(int slot) {
    save_data_ctxt_t *file = malloc(SETTINGS_SAVE_FILE_SIZE);
    for (int i = 0; i < 8; i++) {
        pm_FioReadFlash(i, file, SETTINGS_SAVE_FILE_SIZE);
        if (pm_FioValidateFileChecksum(file) && file->save_slot == slot) {
            free(file);
            return 1;
        }
    }
    free(file);
    return 0;
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
    if (_save_not_empty(pm_status.save_slot)) {
        pm_LoadGame(pm_status.save_slot);
        fp_warp(pm_status.group_id, pm_status.room_id, pm_status.entrance_id);
        fp_log("loaded from slot %d", pm_status.save_slot);
    }
    else {
        fp_log("no file in slot %d", pm_status.save_slot);
    }
}
