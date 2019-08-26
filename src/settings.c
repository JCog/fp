#include <stdint.h>
#include <string.h>
#include "input.h"
#include "fp.h"
#include "resource.h"
#include "settings.h"

static _Alignas(16)
struct settings       settings_store;
struct settings_data *settings = &settings_store.data;

void settings_load_default(void){
    settings_store.header.version = SETTINGS_VERSION;
    struct settings_data *d = &settings_store.data;

    d->bits.font_resource = RES_FONT_PRESSSTART2P;
    d->bits.drop_shadow = 1;
    d->bits.input_display = 1;
    d->bits.log = 1;
    d->menu_x = 16;
    d->menu_y = 60;
    d->input_display_x = 16;
    d->input_display_y = PM64_SCREEN_HEIGHT - 23;
    d->log_x = PM64_SCREEN_WIDTH - 20;
    d->log_y = PM64_SCREEN_HEIGHT - 33;
    d->n_watches = 0;
    d->cheats = 0;
    d->binds[COMMAND_MENU] = input_bind_make(2, BUTTON_R, BUTTON_D_UP);
    d->binds[COMMAND_RETURN] = input_bind_make(2, BUTTON_R, BUTTON_D_LEFT);
    d->binds[COMMAND_LEVITATE] = input_bind_make(1, BUTTON_D_UP);
    d->binds[COMMAND_TURBO] = input_bind_make(1, BUTTON_D_DOWN);
    d->binds[COMMAND_SAVEPOS] = input_bind_make(1, BUTTON_D_LEFT);
    d->binds[COMMAND_LOADPOS] = input_bind_make(1, BUTTON_D_RIGHT);
    d->binds[COMMAND_LZS] = input_bind_make(2, BUTTON_R, BUTTON_D_DOWN);
    d->binds[COMMAND_RELOAD] = input_bind_make(2, BUTTON_B, BUTTON_D_DOWN);
}

void apply_menu_settings(){
    struct gfx_font *font = resource_get(settings->bits.font_resource);
    menu_set_font(fp.main_menu, font);
    menu_set_cell_width(fp.main_menu, font->char_width + font->letter_spacing);
    menu_set_cell_height(fp.main_menu, font->char_height + font->line_spacing);
    gfx_mode_set(GFX_MODE_DROPSHADOW, settings->bits.drop_shadow);
    gfx_mode_configure(GFX_MODE_TEXT, GFX_TEXT_FAST);
    menu_set_pxoffset(fp.main_menu, settings->menu_x);
    menu_set_pyoffset(fp.main_menu, settings->menu_y);
    menu_imitate(fp.global, fp.main_menu);
    /*watchlist_fetch(gz.menu_watchlist);*/
}
