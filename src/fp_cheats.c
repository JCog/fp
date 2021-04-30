#include "menu.h"
#include "settings.h"

static const char *labels[] =
{
    "hp",
    "fp",
    "coins",
    "star power",
    "star pieces",
    "peril",
    "break free"
    
};

static int battle_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != pm_status.battle_debug)
            menu_option_set(item, pm_status.battle_debug);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        pm_status.battle_debug = menu_option_get(item);
    return 0;
}

static int quizmo_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_status.quizmo_debug = 1;
    }
    else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_status.quizmo_debug = 0;
    }
    else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_status.quizmo_debug);
    }
    return 0;
}

static int cheat_proc(struct menu_item *item,
                      enum menu_callback_reason reason,
                      void *data)
{
    int cheat_index = (int)data;
    if (reason == MENU_CALLBACK_SWITCH_ON){
        settings->cheats |= (1 << cheat_index);
    }
    else if (reason == MENU_CALLBACK_SWITCH_OFF){
        settings->cheats &= ~(1 << cheat_index);
    }
    else if (reason == MENU_CALLBACK_THINK){
        menu_checkbox_set(item, settings->cheats & (1 << cheat_index));
    }
    return 0;
}

struct menu *create_cheats_menu(void)
{
    static struct menu menu;
    
    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu.selector = menu_add_submenu(&menu, 0, 0, NULL, "return");
    
    /*build menu*/
    menu_add_static(&menu, 0, 1, "encounters", 0xC0C0C0);
    menu_add_option(&menu, 11, 1, "normal\0""no encounters\0""defeat on contact\0""auto-win\0""auto-runaway\0", battle_proc, NULL);
    int i;
    for (i = 0; i < CHEAT_MAX; ++i) {
        menu_add_checkbox(&menu, 0, 3 + i, cheat_proc, (void*)i);
        menu_add_static(&menu, 2, 3 + i, labels[i], 0xC0C0C0);
    }
    menu_add_checkbox(&menu, 0, 3 + i, quizmo_proc, NULL);
    menu_add_static(&menu, 2, 3 + i, "quizmo spawns", 0xC0C0C0);
    
    return &menu;
}