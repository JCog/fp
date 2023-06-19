#include "fp.h"
#include "menu/menu.h"
#include "timer.h"
#include "trainer.h"

struct Menu *createPracticeMenu(void) {
    static struct Menu menu;
    static struct Menu trainerMenu;
    static struct Menu timerMenu;

    /* initialize menu */
    menuInit(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&trainerMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    /*build menu*/
    menu.selector = menuAddSubmenu(&menu, 0, 0, NULL, "return");
    menuAddSubmenu(&menu, 0, 1, &trainerMenu, "trainers");
    menuAddSubmenu(&menu, 0, 2, &timerMenu, "timer");

    createTrainerMenu(&trainerMenu);
    createTimerMenu(&timerMenu);

    return &menu;
}
