#include "fp.h"
#include "menu/menu.h"
#include "sys/input.h"
#include <string.h>

static void setCamInputMask(void) {
    if (fp.freeCam && !fp.lockCam) {
        fpSetInputMask(BUTTON_C_RIGHT | BUTTON_C_LEFT | BUTTON_C_DOWN | BUTTON_C_UP | BUTTON_Z | BUTTON_L, 0xFF, 0xFF);
    } else {
        fpSetInputMask(BUTTON_L, 0x00, 0x00);
    }
}

static s32 enableCamProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        fp.freeCam = TRUE;
        setCamInputMask();
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        fp.freeCam = FALSE;
        pm_gCameras[pm_gCurrentCameraID].updateMode = 3;
        setCamInputMask();
    } else if (reason == MENU_CALLBACK_THINK) {
        menuCheckboxSet(item, fp.freeCam);
    }
    return 0;
}

static s32 lockCamProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        fp.lockCam = TRUE;
        setCamInputMask();
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        fp.lockCam = FALSE;
        setCamInputMask();
    } else if (reason == MENU_CALLBACK_THINK) {
        menuCheckboxSet(item, fp.lockCam);
    }
    return 0;
}

static void resetCamProc(struct MenuItem *item, void *data) {
    fp.camYaw = 0.f;
    fp.camPitch = 0.f;
    fp.camPos.x = 0.f;
    fp.camPos.y = 0.f;
    fp.camPos.z = 0.f;
}

static s32 camBhvProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_CHANGED) {
        fp.camBhv = menuOptionGet(item);
    } else if (reason == MENU_CALLBACK_THINK) {
        if (menuOptionGet(item) != fp.camBhv) {
            menuOptionSet(item, fp.camBhv);
        }
    }
    return 0;
}

static s32 camDistMinProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_CHANGED) {
        fp.camDistMin = menuIntinputGets(item);
    } else if (reason == MENU_CALLBACK_THINK) {
        if (menuIntinputGets(item) != fp.camDistMin) {
            menuIntinputSet(item, fp.camDistMin);
        }
    }
    return 0;
}

static s32 camDistMaxProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_CHANGED) {
        fp.camDistMax = menuIntinputGets(item);
    } else if (reason == MENU_CALLBACK_THINK) {
        if (menuIntinputGets(item) != fp.camDistMax) {
            menuIntinputSet(item, fp.camDistMax);
        }
    }
    return 0;
}

static s32 camMoveSpeedProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_CHANGED) {
        fp.moveSpeed = menuIntinputGets(item);
        setMoveSpeed(fp.moveSpeed);
    } else if (reason == MENU_CALLBACK_THINK) {
        if (menuIntinputGets(item) != fp.moveSpeed) {
            menuIntinputSet(item, fp.moveSpeed);
        }
    }
    return 0;
}

static s32 camRotSpeedProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_CHANGED) {
        fp.panSpeed = menuIntinputGets(item);
        setPanSpeed(fp.panSpeed);
    } else if (reason == MENU_CALLBACK_THINK) {
        if (menuIntinputGets(item) != fp.panSpeed) {
            menuIntinputSet(item, fp.panSpeed);
        }
    }
    return 0;
}

struct Menu *createCameraMenu(void) {
    static struct Menu menu;

    int y = 0;

    /* initialize menu */
    menuInit(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu.selector = menuAddSubmenu(&menu, 0, y++, NULL, "return");

    menuAddStatic(&menu, 0, y, "enable", 0xC0C0C0);
    menuAddCheckbox(&menu, 16, y++, enableCamProc, NULL);
    menuAddStatic(&menu, 0, y, "lock", 0xC0C0C0);
    menuAddCheckbox(&menu, 16, y++, lockCamProc, NULL);
    menuAddStatic(&menu, 0, ++y, "behavior", 0xC0C0C0);
    menuAddOption(&menu, 16, y++,
                  "manual\0"
                  "birdseye follow\0"
                  "radial follow\0",
                  camBhvProc, NULL);
    menuAddStatic(&menu, 0, y, "distance min", 0xC0C0C0);
    menuAddIntinput(&menu, 16, y++, -10, 5, camDistMinProc, NULL);
    menuAddStatic(&menu, 0, y, "distance max", 0xC0C0C0);
    menuAddIntinput(&menu, 16, y++, -10, 5, camDistMaxProc, NULL);
    menuAddStatic(&menu, 0, y, "move speed", 0xC0C0C0);
    menuAddIntinput(&menu, 16, y++, -10, 2, camMoveSpeedProc, NULL);
    menuAddStatic(&menu, 0, y, "pan speed", 0xC0C0C0);
    menuAddIntinput(&menu, 16, y++, -10, 2, camRotSpeedProc, NULL);
    menuAddButton(&menu, 16, y++, "reset", resetCamProc, NULL);

    return &menu;
}
