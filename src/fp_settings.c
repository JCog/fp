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
    menu_add_static(&menu, 0, 4, "default shortcuts:", 0xC0C0C0);
    menu_add_static(&menu, 0, 5, "save position:     d-left", 0xC0C0C0);
    menu_add_static(&menu, 0, 6, "load position:     d-right", 0xC0C0C0);
    menu_add_static(&menu, 0, 7, "moon jump:         d-up", 0xC0C0C0);
    menu_add_static(&menu, 0, 8, "turbo:             d-down", 0xC0C0C0);
    menu_add_static(&menu, 0, 9, "open menu:         r + d-up", 0xC0C0C0);
    menu_add_static(&menu, 0, 10, "reload room:       r + d-down", 0xC0C0C0);
    menu_add_static(&menu, 0, 11, "easy lzs:          r + d-left", 0xC0C0C0);
    menu_add_static(&menu, 0, 12, "open coordinates:  r + d-right", 0xC0C0C0);
    menu_add_static(&menu, 0, 13, "reload last trick: r + z", 0xC0C0C0);
    
    return &menu;
}