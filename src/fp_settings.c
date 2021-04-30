#include <stdlib.h>
#include "menu.h"
#include "settings.h"
#include "commands.h"

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

static void activate_command_proc(struct menu_item *item, void *data)
{
    int command_index = (int)data;
    if (fp_commands[command_index].proc)
        fp_commands[command_index].proc();
}

static void tab_prev_proc(struct menu_item *item, void *data)
{
    menu_tab_previous(data);
}

static void tab_next_proc(struct menu_item *item, void *data)
{
    menu_tab_next(data);
}

struct menu *create_settings_menu(void)
{
    static struct menu menu;
    static struct menu commands;
    
    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&commands, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);


    /*build menu*/
    int y = 0;
    menu.selector = menu_add_submenu(&menu, 0, y++, NULL, "return");
    menu_add_static(&menu, 0, y, "input display", 0xC0C0C0);
    menu_add_checkbox(&menu, 16, y++, input_display_proc, NULL);
    menu_add_submenu(&menu, 0, y++, &commands, "commands");

    /* populate commands menu */
    commands.selector = menu_add_submenu(&commands, 0, 0, NULL, "return");
    const int page_length = 16;
    int n_pages = (COMMAND_MAX + page_length - 1) / page_length;
    struct menu *pages = malloc(sizeof(*pages) * n_pages);
    struct menu_item *tab = menu_add_tab(&commands, 0, 1, pages, n_pages);
    for (int i = 0; i < n_pages; i++) {
        struct menu *page = &pages[i];
        menu_init(page, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
        for (int j = 0; j < page_length; j++) {
            int n = i * page_length + j;
            if (n >= COMMAND_MAX - 1) { //honestly not sure why the -1 is needed
                break;
            }
            if (fp_commands[n].proc) {
                menu_add_button(page, 0, j, fp_commands[n].text, activate_command_proc, (void*)n);
            }
            else {
                menu_add_static(page, 0, j, fp_commands[n].text, 0xC0C0C0);
            }
            binder_create(page, 18, j, n);
        }
    }
    if (n_pages > 0) {
        menu_tab_goto(tab, 0);
    }
    menu_add_button(&commands, 8, 0, "<", tab_prev_proc, tab);
    menu_add_button(&commands, 10, 0, ">", tab_next_proc, tab);
    
    return &menu;
}