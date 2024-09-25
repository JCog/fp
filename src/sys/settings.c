#include "settings.h"
#include "fp.h"
#include "input.h"
#include "resource.h"
#include "util/watchlist.h"
#include <stdlib.h>
#include <string.h>

// aligned because the game's flash read function does so in 128-byte chunks
static _Alignas(128) struct Settings settingsStore;
static _Alignas(128) struct Settings settingsBuffer[SETTINGS_PROFILE_MAX];
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

    d->cheats = 0;
    d->timerMode = 0;
    d->menuFontResource = RES_FONT_PRESSSTART2P;
    d->menuDropShadow = 1;
    d->menuBackground = 1;
    d->menuBackgroundAlpha = 0xC0;
    d->inputDisplay = 0;
    d->log = 1;
    d->timerLogging = 0;
    d->timerShow = 0;
    d->battleDebug = 0;
    d->watchesVisible = 1;
    d->trainerAcEnabled = 0;
    d->trainerBowserEnabled = 0;
    d->trainerClippyEnabled = 0;
    d->trainerLzsEnabled = 0;
    d->menuX = 16;
    d->menuY = 60;
    d->inputDisplayX = 16;
    d->inputDisplayY = SCREEN_HEIGHT - 23;
    d->logX = SCREEN_WIDTH - 20;
    d->logY = SCREEN_HEIGHT - 33;
    d->timerX = 16;
    d->timerY = 68;
    d->binds[COMMAND_MENU] = bindMake(2, BUTTON_R, BUTTON_D_UP);
    d->binds[COMMAND_RETURN] = bindMake(2, BUTTON_R, BUTTON_D_LEFT);
    d->binds[COMMAND_LEVITATE] = bindMake(2, BUTTON_R, BUTTON_A);
    d->binds[COMMAND_TURBO] = bindMake(2, BUTTON_L, BUTTON_D_DOWN);
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
    d->binds[COMMAND_BREAK_FREE] = bindMake(2, BUTTON_L, BUTTON_D_UP);
    d->binds[COMMAND_TOGGLE_INPUT_DISPLAY] = bindMake(0);
    d->binds[COMMAND_CLIPPY] = bindMake(0);
    d->binds[COMMAND_PAUSE] = bindMake(1, BUTTON_D_DOWN);
    d->binds[COMMAND_ADVANCE] = bindMake(1, BUTTON_D_UP);
    d->cheatEnemyContact = 0;
    d->controlStickRange = 90;
    d->controlStick = 0;
    d->nWatches = 0;
}

void applyMenuSettings(void) {
    struct GfxFont *font = resourceGet(settings->menuFontResource);
    menuSetFont(fp.mainMenu, font);
    menuSetCellWidth(fp.mainMenu, font->charWidth + font->letterSpacing);
    menuSetCellHeight(fp.mainMenu, font->charHeight + font->lineSpacing);
    gfxModeSet(GFX_MODE_DROPSHADOW, settings->menuDropShadow);
    gfxModeConfigure(GFX_MODE_TEXT, GFX_TEXT_FAST);
    menuSetPxoffset(fp.mainMenu, settings->menuX);
    menuSetPyoffset(fp.mainMenu, settings->menuY);
    menuImitate(fp.global, fp.mainMenu);
    watchlistFetch(fp.menuWatchlist);
}

void settingsSave(s32 profile) {
    pm_fioReadFlash(SETTINGS_FIO_PAGE, &settingsBuffer, sizeof(settingsBuffer));
    settingsStore.header.dataChecksum = settingsChecksumCompute(&settingsStore);
    memcpy(&settingsBuffer[profile], &settingsStore, sizeof(settingsStore));
    pm_fioEraseFlash(SETTINGS_FIO_PAGE);
    pm_fioWriteFlash(SETTINGS_FIO_PAGE, &settingsBuffer, sizeof(settingsBuffer));
}

bool settingsLoad(s32 profile) {
    pm_fioReadFlash(SETTINGS_FIO_PAGE, &settingsBuffer, sizeof(settingsBuffer));
    if (!settingsValidate(&settingsBuffer[profile])) {
        return FALSE;
    }
    memcpy(&settingsStore, &settingsBuffer[profile], sizeof(settingsStore));
    return TRUE;
}
