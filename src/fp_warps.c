#include "menu.h"
#include "settings.h"
#include "favorite_rooms.h"
#include "locations.h"
#include "bosses.h"

struct menu *create_warps_menu(void)
{
    static struct menu menu;
    static struct menu locations_menu;
    static struct menu bosses_menu;
    static struct menu favorite_rooms_menu;

    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    /*build menu*/
    menu.selector = menu_add_submenu(&menu, 0, 0, NULL, "return");
    menu_add_submenu(&menu, 0, 1, &locations_menu, "locations");
    menu_add_submenu(&menu, 0, 2, &bosses_menu, "bosses");
    menu_add_submenu(&menu, 0, 3, &favorite_rooms_menu, "favorite rooms");

    create_locations_menu(&locations_menu);
    create_bosses_menu(&bosses_menu);
    create_favorite_rooms_menu(&favorite_rooms_menu);

    return &menu;
}