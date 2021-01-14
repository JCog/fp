#include "menu.h"
#include "mem.h"

struct menu *create_debug_menu(void)
{
    static struct menu menu;
    static struct menu memory;

    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&memory, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu.selector = menu_add_submenu(&menu, 0, 0, NULL, "return");

    /* build menu */
    menu_add_submenu(&menu, 0, 1, &memory, "memory");

    /*build memory menu */
    mem_menu_create(&memory);

    return &menu;
}