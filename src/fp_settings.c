#include "commands.h"
#include "fp.h"
#include "input.h"
#include "menu.h"
#include "resource.h"
#include "settings.h"
#include "watchlist.h"
#include <stdlib.h>

static u16 fontOptions[] = {
    RES_FONT_FIPPS,        RES_FONT_NOTALOT35, RES_FONT_ORIGAMIMOMMY,  RES_FONT_PCSENIOR, RES_FONT_PIXELINTV,
    RES_FONT_PRESSSTART2P, RES_FONT_SMWTEXTNC, RES_FONT_WERDNASRETURN, RES_FONT_PIXELZIM,
};

static s32 byteOptionmodProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    u8 *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menuOptionGet(item) != *p) {
            menuOptionSet(item, *p);
        }
    } else if (reason == MENU_CALLBACK_DEACTIVATE) {
        *p = menuOptionGet(item);
    }
    return 0;
}

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
        if (settings->bits.fontResource != fontOptions[menuOptionGet(item)]) {
            s32 nFontOptions = sizeof(fontOptions) / sizeof(*fontOptions);
            for (s32 i = 0; i < nFontOptions; ++i) {
                if (settings->bits.fontResource == fontOptions[i]) {
                    menuOptionSet(item, i);
                    break;
                }
            }
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        s32 fontResource = fontOptions[menuOptionGet(item)];
        settings->bits.fontResource = fontResource;
        if (settings->bits.fontResource == RES_FONT_FIPPS) {
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
        settings->bits.dropShadow = menuCheckboxGet(item);
        gfxModeSet(GFX_MODE_DROPSHADOW, settings->bits.dropShadow);
    } else if (reason == MENU_CALLBACK_THINK) {
        menuCheckboxSet(item, settings->bits.dropShadow);
    }
    return 0;
}

static s32 genericPositionProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    s16 *x = data;
    s16 *y = x + 1;
    s32 dist = 2;
    if (inputPad() & BUTTON_Z) {
        dist *= 2;
    }
    switch (reason) {
        case MENU_CALLBACK_ACTIVATE: inputReserve(BUTTON_Z); break;
        case MENU_CALLBACK_DEACTIVATE: inputFree(BUTTON_Z); break;
        case MENU_CALLBACK_NAV_UP: *y -= dist; break;
        case MENU_CALLBACK_NAV_DOWN: *y += dist; break;
        case MENU_CALLBACK_NAV_LEFT: *x -= dist; break;
        case MENU_CALLBACK_NAV_RIGHT: *x += dist; break;
        default: break;
    }
    return 0;
}

static s32 menuPositionProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    s32 r = genericPositionProc(item, reason, &settings->menuX);
    menuSetPxoffset(fp.mainMenu, settings->menuX);
    menuSetPyoffset(fp.mainMenu, settings->menuY);
    return r;
}

static s32 timerPositionProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_ACTIVATE) {
        fp.timerMoving = TRUE;
    } else if (reason == MENU_CALLBACK_DEACTIVATE) {
        fp.timerMoving = FALSE;
    }
    return genericPositionProc(item, reason, &settings->timerX);
}

static s32 inputDisplayProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        settings->bits.inputDisplay = 1;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        settings->bits.inputDisplay = 0;
    } else if (reason == MENU_CALLBACK_THINK) {
        menuCheckboxSet(item, settings->bits.inputDisplay);
    }
    return 0;
}

static s32 logProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        settings->bits.log = 1;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        settings->bits.log = 0;
    } else if (reason == MENU_CALLBACK_THINK) {
        menuCheckboxSet(item, settings->bits.log);
    }
    return 0;
}

static s32 logPositionProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    fpLog("test log message!");
    return genericPositionProc(item, reason, &settings->logX);
}

static void activateCommandProc(struct MenuItem *item, void *data) {
    s32 commandIndex = (s32)data;
    if (fpCommands[commandIndex].proc) {
        fpCommands[commandIndex].proc();
    }
}

static void tabPrevProc(struct MenuItem *item, void *data) {
    menuTabPrevious(data);
}

static void tabNextProc(struct MenuItem *item, void *data) {
    menuTabNext(data);
}

static void restoreSettingsProc(struct MenuItem *item, void *data) {
    settingsLoadDefault();
    applyMenuSettings();
    fpLog("loaded defaults");
}

static void saveSettingsProc(struct MenuItem *item, void *data) {
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
    static struct Menu commands;

    /* initialize menu */
    menuInit(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&commands, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    /*build menu*/
    s32 y = 0;
    s32 menuX = 17;
    menu.selector = menuAddSubmenu(&menu, 0, y++, NULL, "return");
    /* appearance controls */
    menuAddStatic(&menu, 0, y, "profile", 0xC0C0C0);
    menuAddButton(&menu, menuX, y, "-", profileDecProc, NULL);
    menuAddWatch(&menu, menuX + 2, y, (u32)&fp.profile, WATCH_TYPE_U8);
    menuAddButton(&menu, menuX + 4, y++, "+", profileIncProc, NULL);
    menuAddStatic(&menu, 0, y, "font", 0xC0C0C0);
    menuAddOption(&menu, menuX, y++,
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
    menuAddStatic(&menu, 0, y, "drop shadow", 0xC0C0C0);
    menuAddCheckbox(&menu, menuX, y++, dropShadowProc, NULL);
    menuAddStatic(&menu, 0, y, "menu position", 0xC0C0C0);
    menuAddPositioning(&menu, menuX, y++, menuPositionProc, NULL);
    menuAddStatic(&menu, 0, y, "timer position", 0xC0C0C0);
    menuAddPositioning(&menu, menuX, y++, timerPositionProc, NULL);
    menuAddStatic(&menu, 0, y, "input display", 0xC0C0C0);
    menuAddCheckbox(&menu, menuX, y, inputDisplayProc, NULL);
    menuAddPositioning(&menu, menuX + 2, y++, genericPositionProc, &settings->inputDisplayX);
    menuAddStatic(&menu, 1, y, "control stick", 0xC0C0C0);
    menuAddOption(&menu, menuX, y++,
                  "numerical\0"
                  "graphical\0"
                  "both\0",
                  byteOptionmodProc, &settings->controlStick);
    menuAddStatic(&menu, 1, y, "graphical range", 0xC0C0C0);
    menuAddIntinput(&menu, menuX, y++, 10, 3, controlStickRangeProc, &settings->controlStickRange);
    menuAddStatic(&menu, 0, y, "log", 0xC0C0C0);
    menuAddCheckbox(&menu, menuX, y, logProc, NULL);
    menuAddPositioning(&menu, menuX + 2, y++, logPositionProc, NULL);
    menuAddSubmenu(&menu, 0, y++, &commands, "commands");
    /* settings commands */
    menuAddButton(&menu, 0, y++, "save settings", saveSettingsProc, NULL);
    menuAddButton(&menu, 0, y++, "load settings", loadSettingsProc, NULL);
    menuAddButton(&menu, 0, y++, "restore defaults", restoreSettingsProc, NULL);

    /* populate commands menu */
    commands.selector = menuAddSubmenu(&commands, 0, 0, NULL, "return");
    const s32 pageLength = 16;
    s32 nPages = (COMMAND_MAX + pageLength - 1) / pageLength;
    struct Menu *pages = malloc(sizeof(*pages) * nPages);
    struct MenuItem *tab = menuAddTab(&commands, 0, 1, pages, nPages);
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
    menuAddButton(&commands, 8, 0, "<", tabPrevProc, tab);
    menuAddButton(&commands, 10, 0, ">", tabNextProc, tab);

    return &menu;
}
