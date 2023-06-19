#include "bosses.h"
#include "fp.h"
#include "locations.h"
#include "menu/menu.h"

struct Menu *createWarpsMenu(void) {
    static struct Menu menu;
    static struct Menu locationsMenu;
    static struct Menu bossesMenu;

    /* initialize menu */
    menuInit(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    /*build menu*/
    menu.selector = menuAddSubmenu(&menu, 0, 0, NULL, "return");
    menuAddSubmenu(&menu, 0, 1, &locationsMenu, "locations");
    menuAddSubmenu(&menu, 0, 2, &bossesMenu, "bosses");

    createLocationsMenu(&locationsMenu);
    createBossesMenu(&bossesMenu);

    return &menu;
}
