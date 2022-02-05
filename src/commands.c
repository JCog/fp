#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "commands.h"
#include "input.h"
#include "fp.h"
#include "watchlist.h"

struct command fp_commands[COMMAND_MAX] = {
    {"show/hide menu",   COMMAND_PRESS_ONCE, 0, NULL                         },
    {"return from menu", COMMAND_PRESS_ONCE, 0, NULL                         },
    {"levitate",         COMMAND_HOLD,       0, command_levitate_proc        },
    {"turbo",            COMMAND_PRESS_ONCE, 0, command_turbo_proc           },
    {"save position",    COMMAND_PRESS_ONCE, 0, command_save_pos_proc        },
    {"load position",    COMMAND_PRESS_ONCE, 0, command_load_pos_proc        },
    {"lzs",              COMMAND_PRESS_ONCE, 0, command_lzs_proc             },
    {"reload map",       COMMAND_PRESS_ONCE, 0, command_reload_proc          },
    {"reload last warp", COMMAND_PRESS_ONCE, 0, command_reload_last_warp_proc},
    {"toggle watches",   COMMAND_PRESS_ONCE, 0, command_toggle_watches_proc  },
    {"reimport save",    COMMAND_PRESS_ONCE, 0, command_import_save_proc     },
    {"save game",        COMMAND_PRESS_ONCE, 0, command_save_game_proc       },
    {"load game",        COMMAND_PRESS_ONCE, 0, command_load_game_proc       },
    {"start/stop timer", COMMAND_PRESS_ONCE, 0, command_start_timer_proc     },
    {"reset timer",      COMMAND_PRESS_ONCE, 0, command_reset_timer_proc     },
    {"show/hide timer",  COMMAND_PRESS_ONCE, 0, command_show_hide_timer_proc },
    {"break free",       COMMAND_PRESS_ONCE, 0, command_break_free_proc      },
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

void fp_set_input_mask(u16 pad, u8 x, u8 y) {
    fp.input_mask.buttons = pad;
    fp.input_mask.x_cardinal = x;
    fp.input_mask.y_cardinal = y;
}

_Bool fp_warp(u16 area, u16 map, u16 entrance) {
    pm_PlayAmbientSounds(-1, 0);   // clear ambient sounds
    pm_BgmSetSong(1, -1, 0, 0, 8); // clear secondary songs
    pm_SfxStopSound(0x19C);        // clear upward vine sound
    pm_SfxStopSound(0x19D);        // clear downward vine sound
    pm_disable_player_input();
    pm_status.loading_zone_tangent = 0;
    pm_status.area_id = area;
    pm_status.map_id = map;
    pm_status.entrance_id = entrance;

    pm_MapChangeState = 1;

    PRINTF("***** WARP TRIGGERED *****\n");

    if (pm_status.is_battle) {
        if (pm_battle_state_2 == 0xC9) {
            PRINTF("battle popup is open, destroying menu\n");
            pm_func_802A472C();
        }
        fp.warp_delay = 0;
    } else {
        if (pm_popup_menu_var == 1) {
            PRINTF("overworld popup is open, setting delay and hiding menu\n");
            fp.warp_delay = 15;
            pm_HidePopupMenu();
        } else {
            fp.warp_delay = 0;
        }
    }

    // set the global curtain to default+off state
    // this is mainly to prevent a crash when warping from "card obtained"
    pm_SetCurtainScaleGoal(2.0f);
    pm_SetCurtainDrawCallback(NULL);
    pm_SetCurtainFadeGoal(0.0f);

    fp.warp = 1;

    return 1;
}

void set_flag(u32 *flags, int flag_index, _Bool value) {
    int word_index = flag_index / 32;
    int bit = flag_index % 32;
    if (value) {
        flags[word_index] |= (1 << bit);
    } else {
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
    set_flag(pm_enemy_defeat_flags, flag_index, value);
}

void fp_set_global_byte(int byte_index, s8 value) {
    pm_save_data.global_bytes[byte_index] = value;
}

void command_levitate_proc() {
    // TODO: figure out how to get some version of this working for peach
    if (!(pm_status.peach_flags & (1 << 0))) {
        pm_player.flags |= 1 << 1;
        pm_player.flags &= ~(1 << 2);
        pm_player.y_speed = 11;
        pm_player.frames_in_air = 1;
        // these are the default starting values for when you fall
        pm_player.y_acceleration = -0.350080013275f;
        pm_player.y_jerk = -0.182262003422f;
        pm_player.y_snap = 0.0115200001746f;
    }
}

void command_turbo_proc() {
    if (fp.turbo) {
        fp.turbo = 0;
        fp_log("turbo disabled");
    } else {
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
    if (pm_TimeFreezeMode == 0) {
        pm_TimeFreezeMode = 1;
        fp_log("easy lzs enabled");
    } else if (pm_TimeFreezeMode == 1) {
        pm_TimeFreezeMode = 0;
        fp_log("easy lzs disabled");
    }
}

void command_reload_proc() {
    fp_warp(pm_status.area_id, pm_status.map_id, pm_status.entrance_id);
}

void command_reload_last_warp_proc() {
    if (fp.saved_area != 0x1c) {
        fp_warp(fp.saved_area, fp.saved_map, fp.saved_entrance);
    }
}

void command_toggle_watches_proc() {
    settings->bits.watches_visible = !settings->bits.watches_visible;
    if (settings->bits.watches_visible) {
        watchlist_show(fp.menu_watchlist);
    } else {
        watchlist_hide(fp.menu_watchlist);
    }
}

void command_import_save_proc() {
    if (fp.last_imported_save_path) {
        fp_import_file(fp.last_imported_save_path, NULL);
    }
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
    } else if (fp.timer.state == 2 && fp.timer.mode == 1) {
        fp.timer.cutscene_count = fp.timer.cutscene_target;
    } else if (fp.timer.state == 3) {
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

void command_load_game_proc() {
    save_data_ctxt_t *file = malloc(sizeof(*file));
    pm_FioFetchSavedFileInfo();
    pm_FioReadFlash(pm_save_info.logical_save_info[pm_status.save_slot][0], file, sizeof(*file));
    if (pm_FioValidateFileChecksum(file)) {
        pm_save_data = *file;
        pm_FioDeserializeState();
        fp_warp(file->area_id, file->map_id, file->entrance_id);
        fp_log("loaded from slot %d", pm_status.save_slot);
    } else {
        fp_log("no file in slot %d", pm_status.save_slot);
    }
    free(file);
}

void command_break_free_proc() {
    s32 third_byte_mask = 0xFFFF00FF;
    s32 check_mask = 0x0000FF00;

    if ((pm_player.flags & check_mask) == 0x2000) {
        pm_player.flags &= third_byte_mask;
    }
    fp_log("broke free");
}
