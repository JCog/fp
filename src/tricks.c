#include "menu.h"
#include "tricks.h"
#include "settings.h"

void set_global_flag(int flag_index, _Bool value) {
    int word_index = flag_index / 32;
    int bit = flag_index % 32;
    uint32_t *p = pm_flags.global_flags;
    if (value)
        p[word_index] |= (1 << bit);
    else
        p[word_index] &= ~(1 << bit);
}

void create_tricks_menu(struct menu *menu)
{
    /* initialize menu */
    menu_init(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu->selector = menu_add_submenu(menu, 0, 0, NULL, "return");
}