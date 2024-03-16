#include "menu.h"

s32 menuByteModProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    s8 *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menuIntinputGet(item) != *p) {
            menuIntinputSet(item, *p);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        *p = menuIntinputGet(item);
    }
    return 0;
}

s32 menuByteOptionmodProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
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

s32 menuByteSwitchToggleProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    u8 *p = data;
    if (reason == MENU_CALLBACK_SWITCH_ON || reason == MENU_CALLBACK_SWITCH_OFF) {
        *p = !*p;
    } else if (reason == MENU_CALLBACK_THINK) {
        menuSwitchSet(item, *p);
    }
    return 0;
}

s32 menuByteCheckboxProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    u8 *p = data;
    if (reason == MENU_CALLBACK_SWITCH_ON || reason == MENU_CALLBACK_SWITCH_OFF) {
        *p = !*p;
    } else if (reason == MENU_CALLBACK_THINK) {
        menuCheckboxSet(item, *p);
    }
    return 0;
}

s32 menuByteCycleProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    u8 *p = data;
    if (reason == MENU_CALLBACK_CHANGED) {
        *p = menuCycleGet(item);
    } else if (reason == MENU_CALLBACK_THINK) {
        menuCycleSet(item, *p);
    }
    return 0;
}

s32 menuHalfwordModProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    u16 *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menuIntinputGet(item) != *p) {
            menuIntinputSet(item, *p);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        *p = menuIntinputGet(item);
    }
    return 0;
}

s32 menuWordOptionmodProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    u32 *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menuOptionGet(item) != *p) {
            menuOptionSet(item, *p);
        }
    } else if (reason == MENU_CALLBACK_DEACTIVATE) {
        *p = menuOptionGet(item);
    }
    return 0;
}

void menuFuncProc(struct MenuItem *item, void *data) {
    void (*func)() = data;
    func();
}

void menuTabPrevProc(struct MenuItem *item, void *data) {
    menuTabPrevious(data);
}

void menuTabNextProc(struct MenuItem *item, void *data) {
    menuTabNext(data);
}
