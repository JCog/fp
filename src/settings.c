#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "input.h"
#include "fp.h"
#include "resource.h"
#include "watchlist.h"

static _Alignas(128) struct settings settings_store;
struct settings_data *settings = &settings_store.data;

static uint16_t settings_checksum_compute(struct settings *settings) {
    uint16_t checksum = 0;
    uint16_t *p = (void *)&settings->data;
    uint16_t *e = p + sizeof(settings->data) / sizeof(*p);
    while (p < e) {
        checksum += *p++;
    }
    return checksum;
}

static _Bool settings_validate(struct settings *settings) {
    return settings->header.version == SETTINGS_VERSION && settings->header.data_size == sizeof(settings->data) &&
           settings->header.data_checksum == settings_checksum_compute(settings);
}

void settings_load_default(void) {
    settings_store.header.version = SETTINGS_VERSION;
    settings_store.header.data_size = sizeof(settings_store.data);
    struct settings_data *d = &settings_store.data;

    d->bits.font_resource = RES_FONT_PRESSSTART2P;
    d->bits.drop_shadow = 1;
    d->bits.input_display = 0;
    d->bits.log = 1;
    d->bits.timer_logging = 0;
    d->bits.timer_show = 0;
    d->bits.battle_debug = 0;
    d->bits.quizmo_debug = 0;
    d->bits.watches_visible = 1;
    d->menu_x = 16;
    d->menu_y = 60;
    d->input_display_x = 16;
    d->input_display_y = SCREEN_HEIGHT - 23;
    d->log_x = SCREEN_WIDTH - 20;
    d->log_y = SCREEN_HEIGHT - 33;
    d->timer_x = 16;
    d->timer_y = 68;
    d->n_watches = 0;
    d->cheats = 0;
    d->control_stick = 0;
    d->binds[COMMAND_MENU] = bind_make(2, BUTTON_R, BUTTON_D_UP);
    d->binds[COMMAND_RETURN] = bind_make(2, BUTTON_R, BUTTON_D_LEFT);
    d->binds[COMMAND_LEVITATE] = bind_make(1, BUTTON_D_UP);
    d->binds[COMMAND_TURBO] = bind_make(1, BUTTON_D_DOWN);
    d->binds[COMMAND_SAVEPOS] = bind_make(1, BUTTON_D_LEFT);
    d->binds[COMMAND_LOADPOS] = bind_make(1, BUTTON_D_RIGHT);
    d->binds[COMMAND_LZS] = bind_make(2, BUTTON_R, BUTTON_D_LEFT);
    d->binds[COMMAND_RELOAD] = bind_make(2, BUTTON_R, BUTTON_D_DOWN);
    d->binds[COMMAND_RELOAD_LAST_WARP] = bind_make(0);
    d->binds[COMMAND_TOGGLE_WATCHES] = bind_make(2, BUTTON_R, BUTTON_D_RIGHT);
    d->binds[COMMAND_REIMPORT_SAVE] = bind_make(2, BUTTON_R, BUTTON_Z);
    d->binds[COMMAND_SAVE_GAME] = bind_make(2, BUTTON_L, BUTTON_D_LEFT);
    d->binds[COMMAND_LOAD_GAME] = bind_make(2, BUTTON_L, BUTTON_D_RIGHT);
    d->binds[COMMAND_START_TIMER] = bind_make(0);
    d->binds[COMMAND_RESET_TIMER] = bind_make(0);
    d->binds[COMMAND_SHOW_HIDE_TIMER] = bind_make(0);
}

void apply_menu_settings() {
    struct gfx_font *font = resource_get(settings->bits.font_resource);
    menu_set_font(fp.main_menu, font);
    menu_set_cell_width(fp.main_menu, font->char_width + font->letter_spacing);
    menu_set_cell_height(fp.main_menu, font->char_height + font->line_spacing);
    gfx_mode_set(GFX_MODE_DROPSHADOW, settings->bits.drop_shadow);
    gfx_mode_configure(GFX_MODE_TEXT, GFX_TEXT_FAST);
    menu_set_pxoffset(fp.main_menu, settings->menu_x);
    menu_set_pyoffset(fp.main_menu, settings->menu_y);
    menu_imitate(fp.global, fp.main_menu);
    watchlist_fetch(fp.menu_watchlist);
    pm_status.battle_debug = settings->bits.battle_debug;
    pm_status.quizmo_debug = settings->bits.quizmo_debug;
}

void settings_save(int profile) {
    uint16_t *checksum = &settings_store.header.data_checksum;
    *checksum = settings_checksum_compute(&settings_store);

    // read in save file in the same slot as the profile
    char *start = malloc(SETTINGS_SAVE_FILE_SIZE + sizeof(settings_store));
    pm_FioReadFlash(profile, start, SETTINGS_SAVE_FILE_SIZE);

    // append settings data to the end and save to file
    char *offset = start + SETTINGS_SAVE_FILE_SIZE;
    memcpy(offset, &settings_store, sizeof(settings_store));
    pm_FioWriteFlash(profile, start, SETTINGS_SAVE_FILE_SIZE + sizeof(settings_store));
    free(start);
}

_Bool _save_file_exists() {
    char *file = malloc(SETTINGS_SAVE_FILE_SIZE);
    for (int i = 0; i < 8; i++) {
        pm_FioReadFlash(i, file, SETTINGS_SAVE_FILE_SIZE);
        if (pm_FioValidateFileChecksum(file)) {
            free(file);
            return 1;
        }
    }
    free(file);
    return 0;
}

_Bool settings_load(int profile) {
    // unfortunate side effect here is that you need at least one existing file to load settings - not a big deal for
    // now
    if (!_save_file_exists()) {
        return 0;
    }

    // read in save data along with the settings data in the same slot as the profile
    char *file = malloc(SETTINGS_SAVE_FILE_SIZE + sizeof(settings_store));
    pm_FioReadFlash(profile, file, SETTINGS_SAVE_FILE_SIZE + sizeof(settings_store));

    struct settings *settings_temp = (struct settings *)(file + SETTINGS_SAVE_FILE_SIZE);
    if (!settings_validate(settings_temp)) {
        free(file);
        return 0;
    }
    memcpy(&settings_store, settings_temp, sizeof(*settings_temp));
    free(file);
    return 1;
}
