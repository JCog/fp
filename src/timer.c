#include "menu.h"
#include "fp.h"
#include "gfx.h"

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

static int checkbox_mod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    uint8_t *p = data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        *p = 1;
    }
    else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        *p = 0;
    }
    else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, *p);
    }
    return 0;
}

static void start_proc() {
    if (fp.timer.state == 0) {
        fp.timer.state = 1;
    }
    else if (fp.timer.state == 3) {
        fp.timer.state = 1;
        fp.timer.cutscene_count = 0;
    }
}

static void reset_proc() {
    fp.timer.state = 0;
    fp.timer.cutscene_count = 0;
}

static int timer_draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color, draw_params->alpha));
    struct gfx_font *font = draw_params->font;
    int x = draw_params->x;
    int y = draw_params->y;

    int64_t timer_count = 0;
    switch (fp.timer.state) {
        case 2:
            timer_count = fp.cpu_counter - fp.timer.start;
            break;
        case 3:
            timer_count = fp.timer.end - fp.timer.start;
            break;
    }

    int tenths = timer_count * 10 / fp.cpu_counter_freq;
    int seconds = tenths / 10;
    int minutes = seconds / 60;
    int hours = minutes / 60;
    tenths %= 10;
    seconds %= 60;
    minutes %= 60;
    if (hours > 0) {
        gfx_printf(font, x, y, "%d:%02d:%02d.%d", hours, minutes, seconds, tenths);
    }
    else if (minutes > 0) {
        gfx_printf(font, x, y, "%d:%02d.%d", minutes, seconds, tenths);
    }
    else {
        gfx_printf(font, x, y, "%d.%d", seconds, tenths);
    }
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

    /* initialize menu */
    menu_init(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu->selector = menu_add_submenu(menu, 0, y_main++, NULL, "return");

    /*build menu*/
    menu_add_static_custom(menu, 0, y_main++, timer_draw_proc, NULL, 0xC0C0C0);
    menu_add_button(menu, 0, y_main, "start", start_proc, NULL);
    menu_add_button(menu, 6, y_main++, "reset", reset_proc, NULL);
    y_main++;
    menu_add_static(menu, 0, y_main, "timer status:", 0xC0C0C0);
    menu_add_static_custom(menu, 16, y_main++, timer_status_draw_proc, NULL, 0xC0C0C0);
    menu_add_static(menu, 0, y_main, "cutscene count:", 0xC0C0C0);
    menu_add_intinput(menu, 16, y_main++, 10, 2, byte_mod_proc, &fp.timer.cutscene_target);
    menu_add_static(menu, 0, y_main, "show timer:", 0xC0C0C0);
    menu_add_checkbox(menu, 16, y_main++, checkbox_mod_proc, &fp.timer.show);
    menu_add_static(menu, 0, y_main, "timer logging:", 0xC0C0C0);
    menu_add_checkbox(menu, 16, y_main++, checkbox_mod_proc, &fp.timer.logging);
}