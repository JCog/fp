#include "flags.h"
#include "fp.h"
#include "mem.h"
#include "menu.h"

struct Menu *createDebugMenu(void) {
    static struct Menu menu;
    static struct Menu flags;
    static struct Menu memory;

    /* initialize menu */
    menuInit(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&flags, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&memory, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu.selector = menuAddSubmenu(&menu, 0, 0, NULL, "return");

    /* build menu */
    menuAddSubmenu(&menu, 0, 1, &flags, "flags");
    menuAddSubmenu(&menu, 0, 2, &memory, "memory");

    /*build memory menu */
    flagMenuCreate(&flags);
    memMenuCreate(&memory);
    fp.menuMem = &memory;

    return &menu;
}
