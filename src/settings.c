#include <stdint.h>
#include <string.h>
#include "input.h"
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
    d->menu_x = 15;
    d->menu_y = 55;
    d->input_display_x = 16;
    d->input_display_y = PM64_SCREEN_HEIGHT - 10;
    d->log_x = PM64_SCREEN_WIDTH - 12;
    d->log_y = PM64_SCREEN_HEIGHT - 14;
    d->n_watches = 0;
    d->cheats = 0;
    d->binds[COMMAND_MENU] = make_bind(2, BUTTON_R, BUTTON_D_UP);
    d->binds[COMMAND_RETURN] = make_bind(2, BUTTON_R, BUTTON_D_LEFT);
    d->binds[COMMAND_LEVITATE] = make_bind(1, BUTTON_D_UP);
    d->binds[COMMAND_TURBO] = make_bind(1, BUTTON_D_DOWN);
    d->binds[COMMAND_SAVEPOS] = make_bind(1, BUTTON_D_LEFT);
    d->binds[COMMAND_LOADPOS] = make_bind(1, BUTTON_D_RIGHT);
    d->binds[COMMAND_LZS] = make_bind(2, BUTTON_R, BUTTON_D_DOWN);
}