#include "flags.h"
#include "fp.h"
#include "mem.h"
#include "menu.h"

struct menu *create_debug_menu(void) {
    static struct menu menu;
    static struct menu flags;
    static struct menu memory;

    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&flags, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&memory, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu.selector = menu_add_submenu(&menu, 0, 0, NULL, "return");

    /* build menu */
    menu_add_submenu(&menu, 0, 1, &flags, "flags");
    menu_add_submenu(&menu, 0, 2, &memory, "memory");

    /*build memory menu */
    flag_menu_create(&flags);
    mem_menu_create(&memory);
    fp.menu_mem = &memory;

    return &menu;
}
