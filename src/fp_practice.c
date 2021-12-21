#include "menu.h"
#include "timer.h"
#include "trainer.h"

struct menu *create_practice_menu(void) {
    static struct menu menu;
    static struct menu trainer_menu;
    static struct menu timer_menu;

    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&trainer_menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    /*build menu*/
    menu.selector = menu_add_submenu(&menu, 0, 0, NULL, "return");
    menu_add_submenu(&menu, 0, 1, &trainer_menu, "trainers");
    menu_add_submenu(&menu, 0, 2, &timer_menu, "timer");

    create_trainer_menu(&trainer_menu);
    create_timer_menu(&timer_menu);

    return &menu;
}
