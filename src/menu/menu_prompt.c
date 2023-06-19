#include "menu.h"
#include <stdlib.h>
#include <string.h>

static struct Menu promptMenu;
static MenuPromptCallback promptCallbackProc;
static void *promptCallbackData;

static s32 doCallback(s32 index) {
    if (promptCallbackProc) {
        MenuPromptCallback proc = promptCallbackProc;
        promptCallbackProc = NULL;
        return proc(index, promptCallbackData);
    } else {
        return 0;
    }
}

static s32 leaveProc(struct MenuItem *item, enum MenuSwitchReason reason) {
    if (reason == MENU_SWITCH_RETURN) {
        doCallback(-1);
    }
    return 0;
}

static s32 activateProc(struct MenuItem *item) {
    s32 index = (s32)item->data;
    if (!doCallback(index)) {
        menuReturn(&promptMenu);
    }
    return 0;
}

static s32 destroyProc(struct MenuItem *item) {
    item->data = NULL;
    return 0;
}

void menuPrompt(struct Menu *menu, const char *prompt, const char *options, s32 defaultOption,
                MenuPromptCallback callbackProc, void *callbackData) {
    static bool ready = FALSE;
    if (ready) {
        if (menu == &promptMenu) {
            menu = promptMenu.parent;
            menuReturn(&promptMenu);
        }
        menuDestroy(&promptMenu);
    } else {
        ready = TRUE;
    }
    menuInit(&promptMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    struct MenuItem *item = menuAddStatic(&promptMenu, 0, 0, prompt, 0xC0C0C0);
    item->leaveProc = leaveProc;
    const char *option = options;
    for (s32 i = 0; *option; ++i) {
        item = menuItemAdd(&promptMenu, option - options, 1, NULL, 0xFFFFFF);
        item->data = (void *)i;
        size_t optionLength = strlen(option);
        char *newOption = malloc(optionLength + 1);
        strcpy(newOption, option);
        item->text = newOption;
        item->activateProc = activateProc;
        item->destroyProc = destroyProc;
        option += optionLength + 1;
        if (i == defaultOption) {
            promptMenu.selector = item;
        }
    }
    promptCallbackProc = callbackProc;
    promptCallbackData = callbackData;
    menuEnter(menu, &promptMenu);
}
