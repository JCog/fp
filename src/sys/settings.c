#include "settings.h"
#include "fp.h"
#include "input.h"
#include "resource.h"
#include "util/watchlist.h"
#include <stdlib.h>
#include <string.h>

static _Alignas(128) struct Settings settingsStore;
struct SettingsData *settings = &settingsStore.data;

static u16 settingsChecksumCompute(struct Settings *settings) {
    u16 checksum = 0;
    u16 *p = (void *)&settings->data;
    u16 *e = p + sizeof(settings->data) / sizeof(*p);
    while (p < e) {
        checksum += *p++;
    }
    return checksum;
}

static bool settingsValidate(struct Settings *settings) {
    return settings->header.version == SETTINGS_VERSION && settings->header.dataSize == sizeof(settings->data) &&
           settings->header.dataChecksum == settingsChecksumCompute(settings);
}

void settingsLoadDefault(void) {
    settingsStore.header.version = SETTINGS_VERSION;
    settingsStore.header.dataSize = sizeof(settingsStore.data);
    struct SettingsData *d = &settingsStore.data;

    d->bits.fontResource = RES_FONT_PRESSSTART2P;
    d->bits.dropShadow = 1;
    d->bits.inputDisplay = 0;
    d->bits.log = 1;
    d->bits.timerLogging = 0;
    d->bits.timerShow = 0;
    d->bits.battleDebug = 0;
    d->bits.quizmoDebug = 0;
    d->bits.watchesVisible = 1;
    d->menuX = 16;
    d->menuY = 60;
    d->inputDisplayX = 16;
    d->inputDisplayY = SCREEN_HEIGHT - 23;
    d->logX = SCREEN_WIDTH - 20;
    d->logY = SCREEN_HEIGHT - 33;
    d->timerX = 16;
    d->timerY = 68;
    d->nWatches = 0;
    d->cheats = 0;
    d->controlStick = 0;
    d->controlStickRange = 90;
    d->binds[COMMAND_MENU] = bindMake(2, BUTTON_R, BUTTON_D_UP);
    d->binds[COMMAND_RETURN] = bindMake(2, BUTTON_R, BUTTON_D_LEFT);
    d->binds[COMMAND_LEVITATE] = bindMake(1, BUTTON_D_UP);
    d->binds[COMMAND_TURBO] = bindMake(1, BUTTON_D_DOWN);
    d->binds[COMMAND_SAVEPOS] = bindMake(1, BUTTON_D_LEFT);
    d->binds[COMMAND_LOADPOS] = bindMake(1, BUTTON_D_RIGHT);
    d->binds[COMMAND_LZS] = bindMake(2, BUTTON_R, BUTTON_D_LEFT);
    d->binds[COMMAND_RELOAD] = bindMake(2, BUTTON_R, BUTTON_D_DOWN);
    d->binds[COMMAND_RELOAD_LAST_WARP] = bindMake(0);
    d->binds[COMMAND_TOGGLE_WATCHES] = bindMake(2, BUTTON_R, BUTTON_D_RIGHT);
    d->binds[COMMAND_REIMPORT_SAVE] = bindMake(2, BUTTON_R, BUTTON_Z);
    d->binds[COMMAND_SAVE_GAME] = bindMake(2, BUTTON_L, BUTTON_D_LEFT);
    d->binds[COMMAND_LOAD_GAME] = bindMake(2, BUTTON_L, BUTTON_D_RIGHT);
    d->binds[COMMAND_START_TIMER] = bindMake(0);
    d->binds[COMMAND_RESET_TIMER] = bindMake(0);
    d->binds[COMMAND_SHOW_HIDE_TIMER] = bindMake(0);
    d->binds[COMMAND_BREAK_FREE] = bindMake(2, BUTTON_L, BUTTON_D_DOWN);
    d->binds[COMMAND_TOGGLE_INPUT_DISPLAY] = bindMake(0);
    d->binds[COMMAND_CLIPPY] = bindMake(0);
}

void applyMenuSettings(void) {
    struct GfxFont *font = resourceGet(settings->bits.fontResource);
    menuSetFont(fp.mainMenu, font);
    menuSetCellWidth(fp.mainMenu, font->charWidth + font->letterSpacing);
    menuSetCellHeight(fp.mainMenu, font->charHeight + font->lineSpacing);
    gfxModeSet(GFX_MODE_DROPSHADOW, settings->bits.dropShadow);
    gfxModeConfigure(GFX_MODE_TEXT, GFX_TEXT_FAST);
    menuSetPxoffset(fp.mainMenu, settings->menuX);
    menuSetPyoffset(fp.mainMenu, settings->menuY);
    menuImitate(fp.global, fp.mainMenu);
    watchlistFetch(fp.menuWatchlist);
    pm_gGameStatus.debugEnemyContact = settings->bits.battleDebug;
    pm_gGameStatus.debugQuizmo = settings->bits.quizmoDebug;
}

void settingsSave(s32 profile) {
    u16 *checksum = &settingsStore.header.dataChecksum;
    *checksum = settingsChecksumCompute(&settingsStore);

    // read in save file in the same slot as the profile
    char *start = malloc(SETTINGS_SAVE_FILE_SIZE + sizeof(settingsStore));
    pm_fioReadFlash(profile, start, SETTINGS_SAVE_FILE_SIZE);

    // append settings data to the end and save to file
    char *offset = start + SETTINGS_SAVE_FILE_SIZE;
    memcpy(offset, &settingsStore, sizeof(settingsStore));
    pm_fioWriteFlash(profile, start, SETTINGS_SAVE_FILE_SIZE + sizeof(settingsStore));
    free(start);
}

static bool saveFileExists(void) {
    char *file = malloc(SETTINGS_SAVE_FILE_SIZE);
    for (s32 i = 0; i < 8; i++) {
        pm_fioReadFlash(i, file, SETTINGS_SAVE_FILE_SIZE);
        if (pm_fioValidateFileChecksum(file)) {
            free(file);
            return TRUE;
        }
    }
    free(file);
    return FALSE;
}

bool settingsLoad(s32 profile) {
    // unfortunate side effect here is that you need at least one existing file to load settings - not a big deal for
    // now
    if (!saveFileExists()) {
        return FALSE;
    }

    // read in save data along with the settings data in the same slot as the profile
    char *file = malloc(SETTINGS_SAVE_FILE_SIZE + sizeof(settingsStore));
    pm_fioReadFlash(profile, file, SETTINGS_SAVE_FILE_SIZE + sizeof(settingsStore));

    struct Settings *settingsTemp = (struct Settings *)(file + SETTINGS_SAVE_FILE_SIZE);
    if (!settingsValidate(settingsTemp)) {
        free(file);
        return FALSE;
    }
    memcpy(&settingsStore, settingsTemp, sizeof(*settingsTemp));
    free(file);
    return TRUE;
}
