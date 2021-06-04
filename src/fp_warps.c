#include "menu.h"
#include "settings.h"
#include "locations.h"

struct menu *create_warps_menu(void)
{
    static struct menu menu;
    static struct menu locations_menu;

    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    /*build menu*/
    menu.selector = menu_add_submenu(&menu, 0, 0, NULL, "return");
    menu_add_submenu(&menu, 0, 1, &locations_menu, "locations");

    create_locations_menu(&locations_menu);

    return &menu;
}