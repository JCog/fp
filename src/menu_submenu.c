#include "menu.h"

static s32 activateProc(struct MenuItem *item) {
    if (item->data) {
        menuEnterTop(item->owner, item->data);
    } else {
        menuReturnTop(item->owner);
    }
    return 1;
}

struct MenuItem *menuAddSubmenu(struct Menu *menu, s32 x, s32 y, struct Menu *submenu, const char *name) {
    struct MenuItem *item = menuItemAdd(menu, x, y, name, 0xFFFFFF);
    item->data = submenu;
    item->activateProc = activateProc;
    return item;
}
