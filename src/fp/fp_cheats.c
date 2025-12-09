#include "fp.h"
#include "menu/menu.h"
#include "sys/settings.h"

static const char *labels[] = {
    "hp",       "fp",         "power",           "coins",        "star power", "star pieces",
    "peril",    "auto mash",  "action commands", "power bounce", "peekaboo",   "brighten room",
    "hide hud", "mute music", "quizmo spawns",
};

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
    s32 y = 0;
    menu.selector = menuAddSubmenu(&menu, 0, y++, NULL, "return");

    /*build menu*/
    menuAddStatic(&menu, 0, y, "encounters", 0xC0C0C0);
    struct MenuItem *encountersOption = menuAddOption(&menu, 11, y++,
                                                      "normal\0"
                                                      "no encounters\0"
                                                      "defeat on contact\0"
                                                      "auto-win\0"
                                                      "auto-runaway\0",
                                                      menuByteOptionmodProc, &settings->cheatEnemyContact);
    s32 x = 0;
    s32 yBase = ++y;
    s32 yMax = 0;
    menuItemAddChainLink(menu.selector, encountersOption, MENU_NAVIGATE_DOWN);
    for (s32 i = 0; i < CHEAT_MAX; i++) {
        if (i % 10 == 0 && i != 0) {
            x += 18;
            y = yBase;
        }
        struct MenuItem *option = menuAddCheckbox(&menu, x, y, cheatProc, (void *)i);
        menuAddStatic(&menu, x + 2, y++, labels[i], 0xC0C0C0);
        if (i == 0) {
            menuItemAddChainLink(encountersOption, option, MENU_NAVIGATE_DOWN);
        }
        if (i % 10 == 0) {
            menuItemAddChainLink(option, encountersOption, MENU_NAVIGATE_UP);
        }
        if (y > yMax) {
            yMax = y;
        }
    }
    menuAddButton(&menu, 0, yMax + 1, "save settings", fpSaveSettingsProc, NULL);

    return &menu;
}
