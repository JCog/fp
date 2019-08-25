#include "menu.h"
#include "settings.h"

struct menu *create_settings_menu(void)
{
    static struct menu menu;
    
    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu.selector = menu_add_submenu(&menu, 0, 0, NULL, "return");

    /*build menu*/
    menu_add_static(&menu, 0, 2, "settings are hardcoded for now", 0xC0C0C0);
    menu_add_static(&menu, 0, 4, "feel free to edit defaults", 0xC0C0C0);
    menu_add_static(&menu, 0, 5, "in settings.c", 0xC0C0C0);
    
    return &menu;
}