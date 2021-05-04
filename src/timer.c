#include "menu.h"
#include "fp.h"
#include "gfx.h"
#include "commands.h"

static int byte_mod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    uint8_t *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != *p)
            menu_intinput_set(item, *p);
    }
    else if (reason == MENU_CALLBACK_CHANGED)
        *p = menu_intinput_get(item);
    return 0;
}

static int show_timer_proc(struct menu_item *item, enum menu_callback_reason reason, void *data)
{
    if (reason == MENU_CALLBACK_SWITCH_ON)
        settings->bits.timer_show = 1;
    else if (reason == MENU_CALLBACK_SWITCH_OFF)
        settings->bits.timer_show = 0;
    else if (reason == MENU_CALLBACK_THINK)
        menu_checkbox_set(item, settings->bits.timer_show);
    return 0;
}

static int timer_logging_proc(struct menu_item *item, enum menu_callback_reason reason, void *data)
{
    if (reason == MENU_CALLBACK_SWITCH_ON)
        settings->bits.timer_logging = 1;
    else if (reason == MENU_CALLBACK_SWITCH_OFF)
        settings->bits.timer_logging = 0;
    else if (reason == MENU_CALLBACK_THINK)
        menu_checkbox_set(item, settings->bits.timer_logging);
    return 0;
}

static void start_proc() {
    command_start_timer_proc();
}

static void reset_proc() {
    command_reset_timer_proc();
}

static int timer_draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color, draw_params->alpha));
    struct gfx_font *font = draw_params->font;
    int x = draw_params->x;
    int y = draw_params->y;
    int chHeight = menu_get_cell_height(item->owner, 1);

    int64_t timer_count = 0;
    int32_t lag_frames = 0;
    switch (fp.timer.state) {
        case 2:
            timer_count = fp.cpu_counter - fp.timer.start;
            lag_frames = (pm_unk5.vi_frames - fp.timer.lag_start) / 2
                         - (pm_status.frame_counter - fp.timer.frame_start);
            break;
        case 3:
            timer_count = fp.timer.end - fp.timer.start;
            lag_frames = (fp.timer.lag_end - fp.timer.lag_start) / 2 - (fp.timer.frame_end - fp.timer.frame_start);
            break;
    }

    int hundredths = timer_count * 100 / fp.cpu_counter_freq;
    int seconds = hundredths / 100;
    int minutes = seconds / 60;
    int hours = minutes / 60;
    hundredths %= 100;
    seconds %= 60;
    minutes %= 60;
    if (hours > 0) {
        gfx_printf(font, x, y, "timer %d:%02d:%02d.%02d", hours, minutes, seconds, hundredths);
    }
    else if (minutes > 0) {
        gfx_printf(font, x, y, "timer %d:%02d.%02d", minutes, seconds, hundredths);
    }
    else {
        gfx_printf(font, x, y, "timer %d.%02d", seconds, hundredths);
    }
    gfx_printf(font, x, y + chHeight, "lag   %d", lag_frames);
    return 1;
}

static int timer_status_draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color, draw_params->alpha));
    struct gfx_font *font = draw_params->font;
    int x = draw_params->x;
    int y = draw_params->y;

    switch (fp.timer.state) {
        case 0:
            gfx_printf(font, x, y, "inactive");
            break;
        case 1:
            gfx_printf(font, x, y, "waiting to start");
            break;
        case 2:
            gfx_printf(font, x, y, "running");
            break;
        case 3:
            gfx_printf(font, x, y, "stopped");
            break;
    }
    return 1;
}

void create_timer_menu(struct menu *menu)
{
    int y_main = 0;
    int MENU_X = 15;

    /* initialize menu */
    menu_init(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu->selector = menu_add_submenu(menu, 0, y_main++, NULL, "return");

    /*build menu*/
    menu_add_static_custom(menu, 0, y_main++, timer_draw_proc, NULL, 0xC0C0C0);
    y_main++;
    menu_add_button(menu, 0, y_main, "start", start_proc, NULL);
    menu_add_button(menu, 6, y_main++, "reset", reset_proc, NULL);
    y_main++;
    menu_add_static(menu, 0, y_main, "timer status", 0xC0C0C0);
    menu_add_static_custom(menu, MENU_X, y_main++, timer_status_draw_proc, NULL, 0xC0C0C0);
    menu_add_static(menu, 0, y_main, "cutscene count", 0xC0C0C0);
    menu_add_intinput(menu, MENU_X, y_main++, 10, 2, byte_mod_proc, &fp.timer.cutscene_target);
    menu_add_static(menu, 0, y_main, "show timer", 0xC0C0C0);
    menu_add_checkbox(menu, MENU_X, y_main++, show_timer_proc, NULL);
    menu_add_static(menu, 0, y_main, "timer logging", 0xC0C0C0);
    menu_add_checkbox(menu, MENU_X, y_main++, timer_logging_proc, NULL);
}