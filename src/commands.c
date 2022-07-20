#include "commands.h"
#include "fp.h"
#include "input.h"
#include "timer.h"
#include "watchlist.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

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

void show_menu(void) {
    menu_signal_enter(fp.main_menu, MENU_SWITCH_SHOW);
    fp.menu_active = TRUE;
    input_reserve(BUTTON_D_UP | BUTTON_D_DOWN | BUTTON_D_LEFT | BUTTON_D_RIGHT | BUTTON_L);
}

void hide_menu(void) {
    menu_signal_leave(fp.main_menu, MENU_SWITCH_HIDE);
    fp.menu_active = FALSE;
    input_free(BUTTON_D_UP | BUTTON_D_DOWN | BUTTON_D_LEFT | BUTTON_D_RIGHT | BUTTON_L);
}

void fp_log(const char *fmt, ...) {
    struct log_entry *ent = &fp.log[SETTINGS_LOG_MAX - 1];
    if (ent->msg) {
        free(ent->msg);
    }
    for (s32 i = SETTINGS_LOG_MAX - 1; i > 0; --i) {
        fp.log[i] = fp.log[i - 1];
    }

    va_list va;
    va_start(va, fmt);
    s32 l = vsnprintf(NULL, 0, fmt, va);
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
    fp.input_mask.xCardinal = x;
    fp.input_mask.yCardinal = y;
}

bool fp_warp(enum Areas area, u16 map, u16 entrance) {
    if (pm_gameMode == 0xA || pm_gameMode == 0xB) { // paused/unpausing
        fp_log("can't warp while paused");
        return FALSE;
    } else if ((pm_battleState == 0xD &&
                (pm_battleState2 == 0xC9 || pm_battleState2 == 0x1F || pm_battleState2 == 0x29 ||
                 pm_battleState2 == 0x2 || pm_battleState2 == 0x3D)) ||
               (pm_battleState == 0xE &&
                (pm_battleState2 == 0xB || pm_battleState2 == 0xC9 || pm_battleState2 == 0x1F)) ||
               pm_battleState == 0x11) {
        // these are all the states I can find that crash when you try to warp from battle. 0x11 is slight overkill,
        // but the rest aren't. at some point we should figure out how to back out of these states automatically.
        fp_log("can't warp in battle menu");
        return FALSE;
    } else if (pm_gGameStatus.isBattle) {
        pm_D_800A0900 = 1;
        pm_state_step_end_battle();
    }

    pm_func_800554A4(0); // stop koopa village radio from playing
    pm_func_800554A4(1);
    pm_func_800554A4(2);
    pm_func_800554A4(3);
    pm_bgmSetSong(1, -1, 0, 0, 8); // clear secondary songs
    pm_sfxStopSound(0x19C);        // clear upward vine sound
    pm_sfxStopSound(0x19D);        // clear downward vine sound
    pm_disable_player_input();
    pm_gGameStatus.loadingZoneTangent = 0;
    pm_gGameStatus.areaID = area;
    pm_gGameStatus.mapID = map;
    pm_gGameStatus.entryID = entrance;

    pm_mapChangeState = 1;

    PRINTF("***** WARP TRIGGERED *****\n");
    if (pm_popupMenuVar == 1) {
        PRINTF("overworld popup is open, setting delay and hiding menu\n");
        fp.warp_delay = 15;
        pm_hidePopupMenu();
    } else {
        fp.warp_delay = 0;
    }

    // set the global curtain to default+off state
    // this is mainly to prevent a crash when warping from "card obtained"
    pm_setCurtainScaleGoal(2.0f);
    pm_setCurtainDrawCallback(NULL);
    pm_setCurtainFadeGoal(0.0f);

    fp.warp = TRUE;

    return TRUE;
}

static void set_flag(u32 *flags, s32 flag_index, bool value) {
    s32 word_index = flag_index / 32;
    s32 bit = flag_index % 32;
    if (value) {
        flags[word_index] |= (1 << bit);
    } else {
        flags[word_index] &= ~(1 << bit);
    }
}

void fp_set_global_flag(s32 flag_index, bool value) {
    set_flag(pm_gCurrentSaveFile.globalFlags, flag_index, value);
}

void fp_set_area_flag(s32 flag_index, bool value) {
    set_flag(pm_gCurrentSaveFile.areaFlags, flag_index, value);
}

void fp_set_global_byte(s32 byte_index, s8 value) {
    pm_gCurrentSaveFile.globalBytes[byte_index] = value;
}

void command_levitate_proc(void) {
    // TODO: figure out how to get some version of this working for peach
    if (!(pm_gGameStatus.peachFlags & (1 << 0))) {
        pm_gPlayerStatus.flags |= 1 << 1;
        pm_gPlayerStatus.flags &= ~(1 << 2);
        pm_gPlayerStatus.ySpeed = 11;
        pm_gPlayerStatus.framesInAir = 1;
        // these are the default starting values for when you fall
        pm_gPlayerStatus.yAcceleration = -0.350080013275f;
        pm_gPlayerStatus.yJerk = -0.182262003422f;
        pm_gPlayerStatus.ySnap = 0.0115200001746f;
    }
}

void command_turbo_proc(void) {
    if (fp.turbo) {
        fp.turbo = FALSE;
        fp_log("turbo disabled");
    } else {
        fp.turbo = TRUE;
        fp_log("turbo enabled");
    }
}

void command_save_pos_proc(void) {
    fp.saved_x = pm_gPlayerStatus.position.x;
    fp.saved_y = pm_gPlayerStatus.position.y;
    fp.saved_z = pm_gPlayerStatus.position.z;
    fp.saved_facing_angle = pm_gPlayerStatus.currentYaw;
    fp.saved_movement_angle = pm_gPlayerStatus.targetYaw;
    fp_log("position saved");
}

void command_load_pos_proc(void) {
    pm_gPlayerStatus.position.x = fp.saved_x;
    pm_gPlayerStatus.position.y = fp.saved_y;
    pm_gPlayerStatus.position.z = fp.saved_z;
    pm_gPlayerStatus.currentYaw = fp.saved_facing_angle;
    pm_gPlayerStatus.targetYaw = fp.saved_movement_angle;
    fp_log("position loaded");
}

void command_lzs_proc(void) {
    if (pm_timeFreezeMode == 0) {
        pm_timeFreezeMode = 1;
        fp_log("easy lzs enabled");
    } else if (pm_timeFreezeMode == 1) {
        pm_timeFreezeMode = 0;
        fp_log("easy lzs disabled");
    }
}

void command_reload_proc(void) {
    fp_warp(pm_gGameStatus.areaID, pm_gGameStatus.mapID, pm_gGameStatus.entryID);
}

void command_reload_last_warp_proc(void) {
    if (fp.saved_area != 0x1c) {
        fp_warp(fp.saved_area, fp.saved_map, fp.saved_entrance);
    }
}

void command_toggle_watches_proc(void) {
    settings->bits.watches_visible = !settings->bits.watches_visible;
    if (settings->bits.watches_visible) {
        watchlist_show(fp.menu_watchlist);
    } else {
        watchlist_hide(fp.menu_watchlist);
    }
}

void command_import_save_proc(void) {
    if (fp.last_imported_save_path) {
        fp_import_file(fp.last_imported_save_path, NULL);
    }
}

void command_save_game_proc(void) {
    pm_saveGame();
    pm_playSfx(0x10);
    fp_log("saved to slot %d", pm_gGameStatus.saveSlot);
}

void command_start_timer_proc(void) {
    timer_start();
}

void command_reset_timer_proc(void) {
    timer_reset();
}

void command_show_hide_timer_proc(void) {
    settings->bits.timer_show = !settings->bits.timer_show;
}

void command_load_game_proc(void) {
    pm_SaveData *file = malloc(sizeof(*file));
    pm_fioFetchSavedFileInfo();
    pm_fioReadFlash(pm_logicalSaveInfo[pm_gGameStatus.saveSlot][0], file, sizeof(*file));
    if (pm_fioValidateFileChecksum(file)) {
        pm_gCurrentSaveFile = *file;
        pm_fioDeserializeState();
        fp_warp(file->areaID, file->mapID, file->entryID);
        fp_log("loaded from slot %d", pm_gGameStatus.saveSlot);
    } else {
        fp_log("no file in slot %d", pm_gGameStatus.saveSlot);
    }
    free(file);
}

void command_break_free_proc(void) {
    s32 third_byte_mask = 0xFFFF00FF;
    s32 check_mask = 0x0000FF00;

    if ((pm_gPlayerStatus.flags & check_mask) == 0x2000) {
        pm_gPlayerStatus.flags &= third_byte_mask;
    }
    fp_log("broke free");
}
