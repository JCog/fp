#include "menu.h"
#include "settings.h"

static int input_display_proc(struct menu_item *item,
                              enum menu_callback_reason reason,
                              void *data)
{
    if (reason == MENU_CALLBACK_SWITCH_ON)
        settings->bits.input_display = 1;
    else if (reason == MENU_CALLBACK_SWITCH_OFF)
        settings->bits.input_display = 0;
    else if (reason == MENU_CALLBACK_THINK)
        menu_checkbox_set(item, settings->bits.input_display);
    return 0;
}

struct menu *create_settings_menu(void)
{
    static struct menu menu;
    
    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu.selector = menu_add_submenu(&menu, 0, 0, NULL, "return");

    /*build menu*/
    menu_add_static(&menu, 0, 2, "input display", 0xC0C0C0);
    menu_add_checkbox(&menu, 16, 2, input_display_proc, NULL);

    menu_add_static(&menu, 0, 4, "most settings are hardcoded for now", 0xC0C0C0);
    menu_add_static(&menu, 0, 6, "default shortcuts:", 0xC0C0C0);
    menu_add_static(&menu, 0, 7, "save position:     d-left", 0xC0C0C0);
    menu_add_static(&menu, 0, 8, "load position:     d-right", 0xC0C0C0);
    menu_add_static(&menu, 0, 9, "moon jump:         d-up", 0xC0C0C0);
    menu_add_static(&menu, 0, 10, "turbo:             d-down", 0xC0C0C0);
    menu_add_static(&menu, 0, 11, "open menu:         r + d-up", 0xC0C0C0);
    menu_add_static(&menu, 0, 12, "reload room:       r + d-down", 0xC0C0C0);
    menu_add_static(&menu, 0, 13, "easy lzs:          r + d-left", 0xC0C0C0);
    menu_add_static(&menu, 0, 14, "open coordinates:  r + d-right", 0xC0C0C0);
    menu_add_static(&menu, 0, 15, "reload last trick: r + z", 0xC0C0C0);
    
    return &menu;
}