#include <stdint.h>
#include <string.h>
#include "fp.h"
#include "menu.h"
#include "pm64.h"

static void set_cam_input_mask(void) {
    if (fp.free_cam && !fp.lock_cam) {
        fp_set_input_mask(BUTTON_C_RIGHT | BUTTON_C_LEFT | BUTTON_C_DOWN | BUTTON_C_UP | BUTTON_Z | BUTTON_L, 0xFF,
                          0xFF);
    } else {
        fp_set_input_mask(BUTTON_L, 0x00, 0x00);
    }
}

static int enable_cam_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        fp.free_cam = 1;
        set_cam_input_mask();
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        fp.free_cam = 0;
        pm_gCameras[pm_gCurrentCameraID].updateMode = 3;
        set_cam_input_mask();
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, fp.free_cam);
    }
    return 0;
}

static int lock_cam_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        fp.lock_cam = 1;
        set_cam_input_mask();
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        fp.lock_cam = 0;
        set_cam_input_mask();
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, fp.lock_cam);
    }
    return 0;
}

static void reset_cam_proc(struct menu_item *item, void *data) {
    fp.cam_yaw = 0.f;
    fp.cam_pitch = 0.f;
    fp.cam_pos.x = 0.f;
    fp.cam_pos.y = 0.f;
    fp.cam_pos.z = 0.f;
}

static int cam_bhv_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_CHANGED) {
        fp.cam_bhv = menu_option_get(item);
    } else if (reason == MENU_CALLBACK_THINK) {
        if (menu_option_get(item) != fp.cam_bhv) {
            menu_option_set(item, fp.cam_bhv);
        }
    }
    return 0;
}

static int cam_dist_min_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_CHANGED) {
        fp.cam_dist_min = menu_intinput_gets(item);
    } else if (reason == MENU_CALLBACK_THINK) {
        if (menu_intinput_gets(item) != fp.cam_dist_min) {
            menu_intinput_set(item, fp.cam_dist_min);
        }
    }
    return 0;
}

static int cam_dist_max_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_CHANGED) {
        fp.cam_dist_max = menu_intinput_gets(item);
    } else if (reason == MENU_CALLBACK_THINK) {
        if (menu_intinput_gets(item) != fp.cam_dist_max) {
            menu_intinput_set(item, fp.cam_dist_max);
        }
    }
    return 0;
}

struct menu *create_camera_menu(void) {
    static struct menu menu;

    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu.selector = menu_add_submenu(&menu, 0, 0, NULL, "return");

    menu_add_static(&menu, 0, 1, "enable", 0xC0C0C0);
    menu_add_checkbox(&menu, 16, 1, enable_cam_proc, NULL);
    menu_add_static(&menu, 0, 2, "lock", 0xC0C0C0);
    menu_add_checkbox(&menu, 16, 2, lock_cam_proc, NULL);
    menu_add_static(&menu, 0, 4, "behavior", 0xC0C0C0);
    menu_add_option(&menu, 16, 4,
                    "manual\0"
                    "birdseye follow\0"
                    "radial follow\0",
                    cam_bhv_proc, NULL);
    menu_add_static(&menu, 0, 5, "distance min", 0xC0C0C0);
    menu_add_intinput(&menu, 16, 5, -10, 5, cam_dist_min_proc, NULL);
    menu_add_static(&menu, 0, 6, "distance max", 0xC0C0C0);
    menu_add_intinput(&menu, 16, 6, -10, 5, cam_dist_max_proc, NULL);
    menu_add_button(&menu, 16, 7, "reset", reset_cam_proc, NULL);

    return &menu;
}
