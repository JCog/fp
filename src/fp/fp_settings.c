#include "commands.h"
#include "fp.h"
#include "menu/menu.h"
#include "sys/input.h"
#include "sys/resource.h"
#include "sys/settings.h"
#include "util/watchlist.h"
#include <stdlib.h>

static u16 fontOptions[] = {
    RES_FONT_FIPPS,        RES_FONT_NOTALOT35, RES_FONT_ORIGAMIMOMMY,  RES_FONT_PCSENIOR, RES_FONT_PIXELINTV,
    RES_FONT_PRESSSTART2P, RES_FONT_SMWTEXTNC, RES_FONT_WERDNASRETURN, RES_FONT_PIXELZIM,
};

static s32 controlStickRangeProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    u8 *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menuIntinputGet(item) != *p) {
            menuIntinputSet(item, *p);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        if (menuIntinputGet(item) > 127) {
            *p = 127;
        } else {
            *p = menuIntinputGet(item);
        }
    }
    return 0;
}

static void profileDecProc(struct MenuItem *item, void *data) {
    fp.profile += SETTINGS_PROFILE_MAX - 1;
    fp.profile %= SETTINGS_PROFILE_MAX;
}

static void profileIncProc(struct MenuItem *item, void *data) {
    fp.profile += 1;
    fp.profile %= SETTINGS_PROFILE_MAX;
}

static s32 fontProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (settings->menuFontResource != fontOptions[menuOptionGet(item)]) {
            s32 nFontOptions = sizeof(fontOptions) / sizeof(*fontOptions);
            for (s32 i = 0; i < nFontOptions; ++i) {
                if (settings->menuFontResource == fontOptions[i]) {
                    menuOptionSet(item, i);
                    break;
                }
            }
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        s32 fontResource = fontOptions[menuOptionGet(item)];
        settings->menuFontResource = fontResource;
        if (settings->menuFontResource == RES_FONT_FIPPS) {
            gfxModeConfigure(GFX_MODE_TEXT, GFX_TEXT_NORMAL);
        } else {
            gfxModeConfigure(GFX_MODE_TEXT, GFX_TEXT_FAST);
        }
        struct GfxFont *font = resourceGet(fontResource);
        menuSetFont(fp.mainMenu, font);
        menuSetCellWidth(fp.mainMenu, font->charWidth + font->letterSpacing);
        menuSetCellHeight(fp.mainMenu, font->charHeight + font->lineSpacing);
        menuImitate(fp.global, fp.mainMenu);
    }
    return 0;
}

static s32 dropShadowProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_CHANGED) {
        settings->menuDropShadow = menuCheckboxGet(item);
        gfxModeSet(GFX_MODE_DROPSHADOW, settings->menuDropShadow);
    } else if (reason == MENU_CALLBACK_THINK) {
        menuCheckboxSet(item, settings->menuDropShadow);
    }
    return 0;
}

static s32 menuPositionProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    s32 r = menuGenericPositionProc(item, reason, &settings->menuX);
    menuSetPxoffset(fp.mainMenu, settings->menuX);
    menuSetPyoffset(fp.mainMenu, settings->menuY);
    return r;
}

static s32 logPositionProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    fpLog("test log message!");
    return menuGenericPositionProc(item, reason, &settings->logX);
}

static void activateCommandProc(struct MenuItem *item, void *data) {
    s32 commandIndex = (s32)data;
    if (fpCommands[commandIndex].proc) {
        fpCommands[commandIndex].proc();
    }
}

static void restoreSettingsProc(struct MenuItem *item, void *data) {
    settingsLoadDefault();
    applyMenuSettings();
    fpLog("loaded defaults");
}

void fpSaveSettingsProc(struct MenuItem *item, void *data) {
    watchlistStore(fp.menuWatchlist);
    settingsSave(fp.profile);
    fpLog("saved profile %i", fp.profile);
}

static void loadSettingsProc(struct MenuItem *item, void *data) {
    if (settingsLoad(fp.profile)) {
        applyMenuSettings();
        fpLog("loaded profile %i", fp.profile);
    } else {
        fpLog("could not load");
    }
}

struct Menu *createSettingsMenu(void) {
    static struct Menu menu;
    static struct Menu menuAppearance;
    static struct Menu menuDisplay;
    static struct Menu menuCommands;
    static struct Menu menuMisc;

    /* initialize menu */
    menuInit(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&menuAppearance, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&menuDisplay, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&menuCommands, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&menuMisc, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    /*build menu*/
    s32 y = 0;
    s32 menuX = 8;
    menu.selector = menuAddSubmenu(&menu, 0, y++, NULL, "return");
    menuAddStatic(&menu, 0, y, "profile", 0xC0C0C0);
    struct MenuItem *profileButton = menuAddButton(&menu, menuX, y, "-", profileDecProc, NULL);
    menuAddWatch(&menu, menuX + 2, y, (u32)&fp.profile, WATCH_TYPE_U8);
    menuAddButton(&menu, menuX + 4, y++, "+", profileIncProc, NULL);
    y++;
    struct MenuItem *firstMenuButton = menuAddSubmenu(&menu, 0, y++, &menuAppearance, "menu appearance");
    menuAddSubmenu(&menu, 0, y++, &menuDisplay, "display");
    menuAddSubmenu(&menu, 0, y++, &menuCommands, "commands");
    menuAddSubmenu(&menu, 0, y++, &menuMisc, "misc");
    y++;
    menuAddButton(&menu, 0, y++, "save settings", fpSaveSettingsProc, NULL);
    menuAddButton(&menu, 0, y++, "load settings", loadSettingsProc, NULL);
    menuAddButton(&menu, 0, y++, "restore defaults", restoreSettingsProc, NULL);

    menuItemAddChainLink(menu.selector, profileButton, MENU_NAVIGATE_DOWN);
    menuItemAddChainLink(firstMenuButton, profileButton, MENU_NAVIGATE_UP);

    /* appearance menu */
    menuX = 12;
    y = 0;
    menuAppearance.selector = menuAddSubmenu(&menuAppearance, 0, y++, NULL, "return");
    menuAddStatic(&menuAppearance, 0, y, "font", 0xC0C0C0);
    menuAddOption(&menuAppearance, menuX, y++,
                  "fipps\0"
                  "notalot35\0"
                  "origami mommy\0"
                  "pc senior\0"
                  "pixel intv\0"
                  "press start 2p\0"
                  "smw text nc\0"
                  "werdna's return\0"
                  "pixelzim\0",
                  fontProc, NULL);
    menuAddStatic(&menuAppearance, 0, y, "drop shadow", 0xC0C0C0);
    menuAddCheckbox(&menuAppearance, menuX, y++, dropShadowProc, NULL);
    menuAddStatic(&menuAppearance, 0, y, "position", 0xC0C0C0);
    menuAddPositioning(&menuAppearance, menuX, y++, menuPositionProc, NULL);
    y++;
    menuAddStatic(&menuAppearance, 0, y, "background", 0xC0C0C0);
    menuAddCheckbox(&menuAppearance, menuX, y++, menuByteCheckboxProc, &settings->menuBackground);
    menuAddStatic(&menuAppearance, 1, y, "alpha", 0xC0C0C0);
    menuAddIntinput(&menuAppearance, menuX, y++, 16, 2, menuByteModProc, &settings->menuBackgroundAlpha);

    /* display menu */
    menuX = 16;
    y = 0;
    menuDisplay.selector = menuAddSubmenu(&menuDisplay, 0, y++, NULL, "return");
    menuAddStatic(&menuDisplay, 0, y, "logging", 0xC0C0C0);
    menuAddCheckbox(&menuDisplay, menuX, y, menuByteCheckboxProc, &settings->log);
    menuAddPositioning(&menuDisplay, menuX + 2, y++, logPositionProc, NULL);
    menuAddStatic(&menuDisplay, 0, y, "flag logging", 0xC0C0C0);
    menuAddCheckbox(&menuDisplay, menuX, y++, menuByteCheckboxProc, &settings->flagLogging);
    y++;
    menuAddStatic(&menuDisplay, 0, y, "input display", 0xC0C0C0);
    menuAddCheckbox(&menuDisplay, menuX, y, menuByteCheckboxProc, &settings->inputDisplay);
    menuAddPositioning(&menuDisplay, menuX + 2, y++, menuGenericPositionProc, &settings->inputDisplayX);
    menuAddStatic(&menuDisplay, 0, y, "analog type", 0xC0C0C0);
    menuAddOption(&menuDisplay, menuX, y++,
                  "numerical\0"
                  "graphical\0"
                  "both\0",
                  menuByteOptionmodProc, &settings->controlStick);
    menuAddStatic(&menuDisplay, 0, y, "graphical range", 0xC0C0C0);
    menuAddIntinput(&menuDisplay, menuX, y++, 10, 3, controlStickRangeProc, &settings->controlStickRange);

    /* commands menu */
    menuCommands.selector = menuAddSubmenu(&menuCommands, 0, 0, NULL, "return");
    const s32 pageLength = 16;
    s32 nPages = (COMMAND_MAX + pageLength - 1) / pageLength;
    struct Menu *pages = malloc(sizeof(*pages) * nPages);
    struct MenuItem *tab = menuAddTab(&menuCommands, 0, 1, pages, nPages);
    for (s32 i = 0; i < nPages; i++) {
        struct Menu *page = &pages[i];
        menuInit(page, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
        for (s32 j = 0; j < pageLength; ++j) {
            s32 n = i * pageLength + j;
            if (n >= COMMAND_MAX) {
                break;
            }
            if (fpCommands[n].proc) {
                menuAddButton(page, 0, j, fpCommands[n].text, activateCommandProc, (void *)n);
            } else {
                menuAddStatic(page, 0, j, fpCommands[n].text, 0xC0C0C0);
            }
            binderCreate(page, 18, j, n);
        }
    }
    if (nPages > 0) {
        menuTabGoto(tab, 0);
    }
    menuAddButton(&menuCommands, 8, 0, "<", menuTabPrevProc, tab);
    menuAddButton(&menuCommands, 10, 0, ">", menuTabNextProc, tab);

    /* misc menu */
    menuX = 13;
    y = 0;
    menuMisc.selector = menuAddSubmenu(&menuMisc, 0, y++, NULL, "return");
    menuAddStatic(&menuMisc, 0, y, "quick launch", 0xC0C0C0);
    menuAddCheckbox(&menuMisc, menuX, y++, menuByteCheckboxProc, &settings->quickLaunch);

    return &menu;
}
