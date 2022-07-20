#include "timer.h"
#include "commands.h"
#include "fp.h"
#include "gfx.h"
#include "menu.h"
#include "settings.h"

static s64 start = 0;
static s64 end = 0;
static u32 lag_start = 0;
static u32 lag_end = 0;
static u16 frame_start = 0;
static u16 frame_end = 0;
static _Bool prev_cutscene_state = 0;
static enum timer_mode timer_mode = 0;
static enum timer_state timer_state = 0;
static u8 cutscene_target = 1;
static u8 cutscene_count = 0;
static s64 timer_count = 0;
static s32 lag_frames = 0;

static s32 byte_mod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    u8 *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != *p) {
            menu_intinput_set(item, *p);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        *p = menu_intinput_get(item);
    }
    return 0;
}

static s32 timer_mode_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    enum timer_mode *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != *p) {
            menu_option_set(item, *p);
        }
    } else if (reason == MENU_CALLBACK_DEACTIVATE) {
        *p = menu_option_get(item);
    }
    return 0;
}

static s32 show_timer_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        settings->bits.timer_show = 1;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        settings->bits.timer_show = 0;
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, settings->bits.timer_show);
    }
    return 0;
}

static s32 timer_logging_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        settings->bits.timer_logging = 1;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        settings->bits.timer_logging = 0;
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, settings->bits.timer_logging);
    }
    return 0;
}

static void start_proc() {
    timer_start();
}

static void reset_proc() {
    timer_reset();
}

static s32 timer_draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color, draw_params->alpha));
    struct gfx_font *font = draw_params->font;
    s32 x = draw_params->x;
    s32 y = draw_params->y;
    s32 chHeight = menu_get_cell_height(item->owner, 1);

    switch (timer_state) {
        case TIMER_RUNNING:
            timer_count = fp.cpu_counter - start;
            lag_frames = (pm_ViFrames - lag_start) / 2 - (pm_gGameStatus.frameCounter - frame_start);
            break;
        case TIMER_STOPPED:
            timer_count = end - start;
            lag_frames = (lag_end - lag_start) / 2 - (frame_end - frame_start);
            break;
        case TIMER_INACTIVE:
        case TIMER_WAITING:
            timer_count = 0;
            lag_frames = 0;
            break;
    }

    s32 hundredths = timer_count * 100 / fp.cpu_counter_freq;
    s32 seconds = hundredths / 100;
    s32 minutes = seconds / 60;
    s32 hours = minutes / 60;
    hundredths %= 100;
    seconds %= 60;
    minutes %= 60;
    if (hours > 0) {
        gfx_printf(font, x, y, "timer  %d:%02d:%02d.%02d", hours, minutes, seconds, hundredths);
    } else if (minutes > 0) {
        gfx_printf(font, x, y, "timer  %d:%02d.%02d", minutes, seconds, hundredths);
    } else {
        gfx_printf(font, x, y, "timer  %d.%02d", seconds, hundredths);
    }
    gfx_printf(font, x, y + chHeight, "lag    %d", lag_frames >= 0 ? lag_frames : 0);
    return 1;
}

static s32 timer_status_draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color, draw_params->alpha));
    struct gfx_font *font = draw_params->font;
    s32 x = draw_params->x;
    s32 y = draw_params->y;

    switch (timer_state) {
        case TIMER_INACTIVE: gfx_printf(font, x, y, "inactive"); break;
        case TIMER_WAITING: gfx_printf(font, x, y, "waiting to start"); break;
        case TIMER_RUNNING: gfx_printf(font, x, y, "running"); break;
        case TIMER_STOPPED: gfx_printf(font, x, y, "stopped"); break;
    }
    return 1;
}

enum timer_state timer_get_state(void) {
    return timer_state;
}

s64 timer_get_timer_count(void) {
    return timer_count;
}

s32 timer_get_lag_frames(void) {
    return lag_frames;
}

void timer_update(void) {
    _Bool in_cutscene = pm_player.flags & 0x00002000;

    switch (timer_state) {
        case TIMER_WAITING:
            if (timer_mode == TIMER_MANUAL || (prev_cutscene_state && !in_cutscene)) {
                timer_state = TIMER_RUNNING;
                start = fp.cpu_counter;
                lag_start = pm_ViFrames;
                frame_start = pm_gGameStatus.frameCounter;
                if (settings->bits.timer_logging) {
                    fp_log("timer started");
                }
            }
            break;
        case TIMER_RUNNING:
            if (timer_mode == TIMER_AUTO && !prev_cutscene_state && in_cutscene) {
                cutscene_count++;
                if (settings->bits.timer_logging && cutscene_count != cutscene_target) {
                    fp_log("cutscene started");
                }
            }
            if (cutscene_count == cutscene_target) {
                timer_state = TIMER_STOPPED;
                end = fp.cpu_counter;
                lag_end = pm_ViFrames;
                frame_end = pm_gGameStatus.frameCounter;
                fp_log("timer stopped");
            }
            timer_count = fp.cpu_counter - start;
            lag_frames = (pm_ViFrames - lag_start) / 2 - (pm_gGameStatus.frameCounter - frame_start);
            break;
        case TIMER_STOPPED:
            timer_count = end - start;
            lag_frames = (lag_end - lag_start) / 2 - (frame_end - frame_start);
            break;
        case TIMER_INACTIVE: break;
    }

    prev_cutscene_state = pm_player.flags & 0x00002000;
}

void timer_start(void) {
    if (timer_state == TIMER_INACTIVE) {
        timer_state = TIMER_WAITING;
        if (timer_mode == TIMER_AUTO) {
            fp_log("timer set to start");
        }
    } else if (timer_state == TIMER_RUNNING && timer_mode == TIMER_MANUAL) {
        cutscene_count = cutscene_target;
    } else if (timer_state == TIMER_STOPPED) {
        timer_state = TIMER_WAITING;
        cutscene_count = 0;
        if (timer_mode == TIMER_AUTO) {
            fp_log("timer set to start");
        }
    }
}

void timer_reset(void) {
    timer_state = 0;
    cutscene_count = 0;
    fp_log("timer reset");
}

void create_timer_menu(struct menu *menu) {
    s32 y_main = 0;
    s32 MENU_X = 15;

    /* initialize menu */
    menu_init(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu->selector = menu_add_submenu(menu, 0, y_main++, NULL, "return");

    /*build menu*/
    menu_add_static(menu, 0, y_main, "status", 0xC0C0C0);
    menu_add_static_custom(menu, 7, y_main++, timer_status_draw_proc, NULL, 0xC0C0C0);
    menu_add_static_custom(menu, 0, y_main++, timer_draw_proc, NULL, 0xC0C0C0);
    y_main++;
    menu_add_button(menu, 0, y_main, "start/stop", start_proc, NULL);
    menu_add_button(menu, 11, y_main++, "reset", reset_proc, NULL);
    y_main++;
    menu_add_static(menu, 0, y_main, "timer mode", 0xC0C0C0);
    menu_add_option(menu, MENU_X, y_main++,
                    "automatic\0"
                    "manual\0",
                    timer_mode_proc, &timer_mode);
    menu_add_static(menu, 0, y_main, "cutscene count", 0xC0C0C0);
    menu_add_intinput(menu, MENU_X, y_main++, 10, 2, byte_mod_proc, &cutscene_target);
    menu_add_static(menu, 0, y_main, "show timer", 0xC0C0C0);
    menu_add_checkbox(menu, MENU_X, y_main++, show_timer_proc, NULL);
    menu_add_static(menu, 0, y_main, "timer logging", 0xC0C0C0);
    menu_add_checkbox(menu, MENU_X, y_main++, timer_logging_proc, NULL);
}
