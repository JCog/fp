#include "menu.h"

static s32 activate_proc(struct menu_item *item) {
    if (item->data) {
        menu_enter_top(item->owner, item->data);
    } else {
        menu_return_top(item->owner);
    }
    return 1;
}

struct menu_item *menu_add_submenu(struct menu *menu, s32 x, s32 y, struct menu *submenu, const char *name) {
    struct menu_item *item = menu_item_add(menu, x, y, name, 0xFFFFFF);
    item->data = submenu;
    item->activate_proc = activate_proc;
    return item;
}
