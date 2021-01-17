#include "menu.h"
#include "settings.h"
#include "flags.h"

static int byte_mod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    uint8_t *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != *p)
            menu_intinput_set(item, *p);
    }
    else if (reason == MENU_CALLBACK_CHANGED)
        *p = menu_intinput_get(item);
    return 0;
}

struct menu *create_progression_menu(void)
{
    static struct menu menu;
    static struct menu flags;

    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&flags, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    /*build menu*/
    menu.selector = menu_add_submenu(&menu, 0, 0, NULL, "return");
    menu_add_static(&menu, 0, 1, "story progress", 0xC0C0C0);
    menu_add_intinput(&menu, 16, 1, 16, 2, byte_mod_proc, &pm_unk3.story_progress);
    menu_add_submenu(&menu, 0, 2, &flags, "flags");

    flag_menu_create(&flags);

    return &menu;
}