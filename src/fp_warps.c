#include "menu.h"
#include "locations.h"
#include "bosses.h"

struct menu *create_warps_menu(void) {
    static struct menu menu;
    static struct menu locations_menu;
    static struct menu bosses_menu;

    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    /*build menu*/
    menu.selector = menu_add_submenu(&menu, 0, 0, NULL, "return");
    menu_add_submenu(&menu, 0, 1, &locations_menu, "locations");
    menu_add_submenu(&menu, 0, 2, &bosses_menu, "bosses");

    create_locations_menu(&locations_menu);
    create_bosses_menu(&bosses_menu);

    return &menu;
}
