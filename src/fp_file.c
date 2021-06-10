#include "menu.h"
#include "settings.h"
#include "fp.h"
#include "commands.h"

static void save_slot_dec_proc(struct menu_item *item, void *data)
{
    pm_status.save_slot += 3;
    pm_status.save_slot %= 4;
}

static void save_slot_inc_proc(struct menu_item *item, void *data)
{
    pm_status.save_slot += 1;
    pm_status.save_slot %= 4;
}

static void save_proc() {
    command_save_game_proc();
}

static void load_proc() {
    command_load_game_proc();
}

static int byte_mod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    int8_t *p = data;
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

struct menu *create_file_menu(void)
{
    static struct menu menu;
    
    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    /*build menu*/
    int y = 0;
    int MENU_X = 16;

    menu.selector = menu_add_submenu(&menu, 0, y++, NULL, "return");
    menu_add_static(&menu, 0, y, "save slot", 0xC0C0C0);
    menu_add_button(&menu, 11, y, "-", save_slot_dec_proc, NULL);
    menu_add_watch(&menu, 13, y, (uint32_t)&pm_status.save_slot, WATCH_TYPE_U8);
    menu_add_button(&menu, 15, y++, "+", save_slot_inc_proc, NULL);
    menu_add_button(&menu, 0, y, "save", save_proc, NULL);
    menu_add_button(&menu, 5, y++, "load", load_proc, NULL);
    y++;
    menu_add_static(&menu, 0, y, "story progress", 0xC0C0C0);
    menu_add_intinput(&menu, MENU_X, y++, 16, 2, byte_mod_proc, &pm_unk3.story_progress);
    menu_add_static(&menu, 0, y, "music", 0xC0C0C0);
    menu_add_checkbox(&menu, MENU_X, y++, checkbox_mod_proc, &pm_status.music_enabled);
    
    return &menu;
}