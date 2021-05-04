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

struct menu *create_file_menu(void)
{
    static struct menu menu;
    
    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    /*build menu*/
    int y = 0;
    int MENU_X = 11;
    menu.selector = menu_add_submenu(&menu, 0, y++, NULL, "return");
    menu_add_static(&menu, 0, y, "save slot", 0xC0C0C0);
    menu_add_button(&menu, MENU_X, y, "-", save_slot_dec_proc, NULL);
    menu_add_watch(&menu, MENU_X + 2, y, (uint32_t)&pm_status.save_slot, WATCH_TYPE_U8);
    menu_add_button(&menu, MENU_X + 4, y++, "+", save_slot_inc_proc, NULL);
    menu_add_button(&menu, 0, y, "save", save_proc, NULL);
    menu_add_button(&menu, 5, y++, "load", load_proc, NULL);
    
    return &menu;
}