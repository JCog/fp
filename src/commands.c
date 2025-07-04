#include "commands.h"
#include "fp.h"
#include "fp/practice/timer.h"
#include "pm64.h"
#include "sys/input.h"
#include "util/watchlist.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

struct Command fpCommands[COMMAND_MAX] = {
    {"show/hide menu",   COMMAND_PRESS_ONCE, 0, NULL                     },
    {"return from menu", COMMAND_PRESS_ONCE, 0, NULL                     },
    {"levitate",         COMMAND_HOLD,       0, commandLevitateProc      },
    {"turbo",            COMMAND_PRESS_ONCE, 0, commandTurboProc         },
    {"save position",    COMMAND_PRESS_ONCE, 0, commandSavePosProc       },
    {"load position",    COMMAND_PRESS_ONCE, 0, commandLoadPosProc       },
    {"lzs",              COMMAND_PRESS_ONCE, 0, commandLzsProc           },
    {"reload map",       COMMAND_PRESS_ONCE, 0, commandReloadProc        },
    {"reload last warp", COMMAND_PRESS_ONCE, 0, commandReloadLastWarpProc},
    {"toggle watches",   COMMAND_PRESS_ONCE, 0, commandToggleWatchesProc },
    {"reimport save",    COMMAND_PRESS_ONCE, 0, commandImportSaveProc    },
    {"save game",        COMMAND_PRESS_ONCE, 0, commandSaveGameProc      },
    {"load game",        COMMAND_PRESS_ONCE, 0, commandLoadGameProc      },
    {"start/stop timer", COMMAND_PRESS_ONCE, 0, commandStartStopTimerProc},
    {"reset timer",      COMMAND_PRESS_ONCE, 0, commandResetTimerProc    },
    {"show/hide timer",  COMMAND_PRESS_ONCE, 0, commandShowHideTimerProc },
    {"break free",       COMMAND_PRESS_ONCE, 0, commandBreakFreeProc     },
    {"toggle in. disp.", COMMAND_PRESS_ONCE, 0, commandToggleInpDispProc },
    {"clippy",           COMMAND_PRESS_ONCE, 0, commandClippyProc        },
    {"store ability",    COMMAND_PRESS_ONCE, 0, commandStoreAbility      },
    {"ignore walls",     COMMAND_PRESS_ONCE, 0, commandIgnoreWalls       },
    {"floor clip",       COMMAND_PRESS_ONCE, 0, commandFloorClip         },
};

void showMenu(void) {
    menuSignalEnter(fp.mainMenu, MENU_SWITCH_SHOW);
    fp.menuActive = TRUE;
    inputReserve(BUTTON_D_UP | BUTTON_D_DOWN | BUTTON_D_LEFT | BUTTON_D_RIGHT | BUTTON_L);
}

void hideMenu(void) {
    menuSignalLeave(fp.mainMenu, MENU_SWITCH_HIDE);
    fp.menuActive = FALSE;
    inputFree(BUTTON_D_UP | BUTTON_D_DOWN | BUTTON_D_LEFT | BUTTON_D_RIGHT | BUTTON_L);
}

void fpLog(const char *fmt, ...) {
    struct LogEntry *ent = &fp.log[SETTINGS_LOG_MAX - 1];
    if (ent->msg) {
        free(ent->msg);
    }
    for (s32 i = SETTINGS_LOG_MAX - 1; i > 0; --i) {
        fp.log[i] = fp.log[i - 1];
    }

    va_list va;
    va_start(va, fmt);
    s32 msgSize = vsnprintf(NULL, 0, fmt, va) + 1;
    va_end(va);

    ent = &fp.log[0];
    ent->msg = malloc(msgSize);
    if (!ent->msg) {
        return;
    }
    va_start(va, fmt);
    vsnprintf(ent->msg, msgSize, fmt, va);
    va_end(va);
    ent->age = 0;
}

void fpSetInputMask(u32 pad, s8 x, s8 y) {
    fp.inputMask.buttons = pad;
    fp.inputMask.stickX = x;
    fp.inputMask.stickY = y;
}

bool fpWarp(enum Areas area, u16 map, u16 entrance) {
    pm_bgm_set_song(1, -1, 0, 0, 8); // clear secondary songs
    pm_snd_ambient_stop_all(0);
    pm_au_sfx_reset_players(pm_gSoundManager);
    pm_disable_player_input();
    pm_gGameStatus.loadingZoneTangent = 0;
    pm_gGameStatus.areaID = area;
    pm_gGameStatus.mapID = map;
    pm_gGameStatus.entryID = entrance;

    PRINTF("***** WARP TRIGGERED *****\n");
    if (pm_gGameStatus.isBattle || pm_gPopupState == 1) {
        // prevent crashes from warping when in battle menus or with partner/item menu open
        pm_clear_windows();
    }

    if (pm_CurGameMode == 0xA) { // paused
        fp.warpDelay = 5;
        pm_set_game_mode(0xB);
    } else {
        fp.warpDelay = 0;
    }

    // set the global curtain to default+off state
    // this is mainly to prevent a crash when warping from "card obtained"
    pm_set_curtain_scale_goal(2.0f);
    pm_set_curtain_draw_callback(NULL);
    pm_set_curtain_fade_goal(0.0f);

    fp.warp = TRUE;

    return TRUE;
}

static void setFlag(u32 *flags, s32 flagIndex, bool value) {
    s32 wordIndex = flagIndex / 32;
    s32 bit = flagIndex % 32;
    if (value) {
        flags[wordIndex] |= (1 << bit);
    } else {
        flags[wordIndex] &= ~(1 << bit);
    }
}

void fpSetGlobalFlag(s32 flagIndex, bool value) {
    setFlag(pm_gCurrentSaveFile.globalFlags, flagIndex, value);
}

void fpSetAreaFlag(s32 flagIndex, bool value) {
    setFlag(pm_gCurrentSaveFile.areaFlags, flagIndex, value);
}

void fpSetGlobalByte(s32 byteIndex, s8 value) {
    pm_gCurrentSaveFile.globalBytes[byteIndex] = value;
}

void commandLevitateProc(void) {
    pm_gPlayerStatus.flags |= 1 << 1;
    pm_gPlayerStatus.flags &= ~(1 << 2);
    pm_gPlayerStatus.ySpeed = 11;
    pm_gPlayerStatus.framesInAir = 1;
    // these are the default starting values for when you fall
    pm_gPlayerStatus.yAcceleration = -0.350080013275f;
    pm_gPlayerStatus.yJerk = -0.182262003422f;
    pm_gPlayerStatus.ySnap = 0.0115200001746f;

    if (pm_gGameStatus.peachFlags & (1 << 0)) {
        pm_PlayerActionsTable[5].flag = TRUE;
    }
}

void commandTurboProc(void) {
    if (fp.turbo) {
        fp.turbo = FALSE;
        fpLog("turbo disabled");
    } else {
        fp.turbo = TRUE;
        fpLog("turbo enabled");
    }
}

void commandSavePosProc(void) {
    fp.savedPos.x = pm_gPlayerStatus.position.x;
    fp.savedPos.y = pm_gPlayerStatus.position.y;
    fp.savedPos.z = pm_gPlayerStatus.position.z;
    fp.savedFacingAngle = pm_gPlayerStatus.currentYaw;
    fp.savedMovementAngle = pm_gPlayerStatus.targetYaw;
    fp.savedCam = pm_gCameras[pm_gCurrentCameraID];
    fpLog("position saved");
}

void commandLoadPosProc(void) {
    pm_gPlayerStatus.position.x = fp.savedPos.x;
    pm_gPlayerStatus.position.y = fp.savedPos.y;
    pm_gPlayerStatus.position.z = fp.savedPos.z;
    pm_gPlayerStatus.currentYaw = fp.savedFacingAngle;
    pm_gPlayerStatus.targetYaw = fp.savedMovementAngle;

    if (pm_gPlayerStatus.actionState & ACTION_STATE_RIDE) {
        pm_Npc *partner = pm_get_npc_safe(-4); // NPC_PARTNER
        if (partner) {
            partner->pos.x = fp.savedPos.x;
            partner->pos.y = fp.savedPos.y;
            partner->pos.z = fp.savedPos.z;
            partner->moveToPos.y = fp.savedPos.y;
            partner->yaw = fp.savedMovementAngle;
        }
    }

    if (fp.savedCam.flags) {
        pm_gCameras[pm_gCurrentCameraID] = fp.savedCam;
    }
    fpLog("position loaded");
}

void commandLzsProc(void) {
    if (pm_timeFreezeMode == 0) {
        pm_timeFreezeMode = 1;
        fpLog("easy lzs enabled");
    } else if (pm_timeFreezeMode == 1) {
        pm_timeFreezeMode = 0;
        fpLog("easy lzs disabled");
    }
}

void commandReloadProc(void) {
    fpWarp(pm_gGameStatus.areaID, pm_gGameStatus.mapID, pm_gGameStatus.entryID);
}

void commandReloadLastWarpProc(void) {
    if (fp.savedArea != 0x1c) {
        fpWarp(fp.savedArea, fp.savedMap, fp.savedEntrance);
    }
}

void commandToggleWatchesProc(void) {
    settings->watchesVisible = !settings->watchesVisible;
    if (settings->watchesVisible) {
        watchlistShow(fp.menuWatchlist);
    } else {
        watchlistHide(fp.menuWatchlist);
    }
}

void commandImportSaveProc(void) {
    if (fp.lastImportedSavePath) {
        fpImportFile(fp.lastImportedSavePath, NULL);
    }
}

void commandSaveGameProc(void) {
    pm_entity_SaveBlock_save_data();
    pm_sfx_play_sound(0x10);
    fpLog("saved to slot %d", pm_gGameStatus.saveSlot);
}

void commandLoadGameProc(void) {
    // prevent loading before being in game
    if (pm_CurGameMode < 4 || pm_CurGameMode >= 12 || pm_gGameStatus.demoState) {
        fpLog("can't load right now");
        return;
    }

    pm_SaveData *file = malloc(sizeof(*file));
    pm_fio_fetch_saved_file_info();
    pm_fio_read_flash(pm_LogicalSaveInfo[pm_gGameStatus.saveSlot][0], file, sizeof(*file));
    if (pm_fio_validate_globals_checksums(file)) {
        pm_gCurrentSaveFile = *file;
        pm_fio_deserialize_state();
        fpWarp(file->areaID, file->mapID, file->entryID);
        fpLog("loaded from slot %d", pm_gGameStatus.saveSlot);
    } else {
        fpLog("no file in slot %d", pm_gGameStatus.saveSlot);
    }
    free(file);
}

void commandStartStopTimerProc(void) {
    timerStartStop();
}

void commandResetTimerProc(void) {
    timerReset();
}

void commandShowHideTimerProc(void) {
    settings->timerShow = !settings->timerShow;
}

void commandBreakFreeProc(void) {
    s32 thirdByteMask = 0xFFFF00FF;
    s32 checkMask = 0x0000FF00;

    if ((pm_gPlayerStatus.flags & checkMask) == 0x2000) {
        pm_gPlayerStatus.flags &= thirdByteMask;
    }
    fpLog("broke free");
}

void commandToggleInpDispProc(void) {
    settings->inputDisplay ^= 1;
}

void commandClippyProc(void) {
    if (pm_gPlayerStatus.enableCollisionOverlapsCheck) {
        pm_gPlayerStatus.enableCollisionOverlapsCheck = 0;
        pm_gPartnerStatus.shouldResumeAbility = 0;
        fpLog("clippy disabled");
    } else {
        pm_gPlayerStatus.enableCollisionOverlapsCheck = 1;
        pm_gPartnerStatus.shouldResumeAbility = 1;
        fpLog("clippy enabled");
    }
}

void commandStoreAbility(void) {
    // useful for getting sushie glitch and warping to entrances over lava with laki
    if (pm_gGameStatus.keepUsingPartnerOnMapChange) {
        pm_gPartnerStatus.partnerActionState = 0;
        pm_gGameStatus.keepUsingPartnerOnMapChange = 0;
        fpLog("partner ability disabled");
    } else {
        pm_gPartnerStatus.partnerActionState = 1;
        pm_gGameStatus.keepUsingPartnerOnMapChange = 1;
        fpLog("partner ability stored");
    }
}

void commandIgnoreWalls(void) {
    if (fp.ignoreWalls) {
        fp.ignoreWalls = FALSE;
        fpLog("walls enabled");
    } else {
        fp.ignoreWalls = TRUE;
        fpLog("walls disabled");
    }
}

void commandFloorClip(void) {
    pm_gPlayerStatus.position.y -= 20;
    if (pm_gPlayerStatus.actionState & ACTION_STATE_RIDE) {
        pm_Npc *partner = pm_get_npc_safe(-4); // NPC_PARTNER
        if (partner) {
            partner->pos.y -= 20;
            partner->moveToPos.y -= 20;
        }
    }
}
