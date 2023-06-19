#include "menu/menu.h"
#include "sys/settings.h"

static const char *labels[] = {
    "hp",          "fp",
    "coins",       "star power",
    "star pieces", "peril",
    "auto mash",   "auto action command",
    "peekaboo",    "brighten room",
};

static s32 battleProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menuOptionGet(item) != pm_gGameStatus.debugEnemyContact) {
            menuOptionSet(item, pm_gGameStatus.debugEnemyContact);
        }
    } else if (reason == MENU_CALLBACK_DEACTIVATE) {
        pm_gGameStatus.debugEnemyContact = menuOptionGet(item);
        settings->bits.battleDebug = pm_gGameStatus.debugEnemyContact;
    }
    return 0;
}

static s32 quizmoProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_gGameStatus.debugQuizmo = 1;
        settings->bits.quizmoDebug = 1;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_gGameStatus.debugQuizmo = 0;
        settings->bits.quizmoDebug = 0;
    } else if (reason == MENU_CALLBACK_THINK) {
        menuCheckboxSet(item, pm_gGameStatus.debugQuizmo);
    }
    return 0;
}

static s32 cheatProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    s32 cheatIndex = (s32)data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        settings->cheats |= (1 << cheatIndex);
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        settings->cheats &= ~(1 << cheatIndex);
    } else if (reason == MENU_CALLBACK_THINK) {
        menuCheckboxSet(item, settings->cheats & (1 << cheatIndex));
    }
    return 0;
}

struct Menu *createCheatsMenu(void) {
    static struct Menu menu;

    /* initialize menu */
    menuInit(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu.selector = menuAddSubmenu(&menu, 0, 0, NULL, "return");

    /*build menu*/
    menuAddStatic(&menu, 0, 1, "encounters", 0xC0C0C0);
    struct MenuItem *encountersOption = menuAddOption(&menu, 11, 1,
                                                      "normal\0"
                                                      "no encounters\0"
                                                      "defeat on contact\0"
                                                      "auto-win\0"
                                                      "auto-runaway\0",
                                                      battleProc, NULL);
    s32 i;
    menuItemAddChainLink(menu.selector, encountersOption, MENU_NAVIGATE_DOWN);
    for (i = 0; i < CHEAT_MAX; ++i) {
        struct MenuItem *option = menuAddCheckbox(&menu, 0, 3 + i, cheatProc, (void *)i);
        menuAddStatic(&menu, 2, 3 + i, labels[i], 0xC0C0C0);
        if (i == 0) {
            menuItemAddChainLink(option, encountersOption, MENU_NAVIGATE_UP);
        }
    }
    menuAddCheckbox(&menu, 0, 3 + i, quizmoProc, NULL);
    menuAddStatic(&menu, 2, 3 + i, "quizmo spawns", 0xC0C0C0);

    return &menu;
}
