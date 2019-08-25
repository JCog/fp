#include "menu.h"
#include "settings.h"

struct menu *create_inventory_menu(void)
{
    static struct menu menu;
    
    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu.selector = menu_add_submenu(&menu, 0, 0, NULL, "return");

    /*build menu*/
    
    return &menu;
}