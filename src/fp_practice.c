#include "menu.h"
#include "settings.h"
#include "trainer.h"
#include "tricks.h"

struct menu *create_practice_menu(void)
{
    static struct menu menu;
    static struct menu tricks_menu;
    static struct menu trainer_menu;

    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&trainer_menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    /*build menu*/
    menu.selector = menu_add_submenu(&menu, 0, 0, NULL, "return");
    menu_add_submenu(&menu, 0, 1, &tricks_menu, "tricks");
    menu_add_submenu(&menu, 0, 2, &trainer_menu, "trainers");

    create_tricks_menu(&tricks_menu);
    create_trainer_menu(&trainer_menu);

    return &menu;
}