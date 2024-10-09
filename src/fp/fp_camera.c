#include "fp.h"
#include "menu/menu.h"
#include "sys/input.h"
#include <string.h>

static bool firstUse = TRUE;

static void resetCam(void) {
    pm_Camera cam = pm_gCameras[pm_gCurrentCameraID];
    pm_gCameras[pm_gCurrentCameraID].targetPos = pm_gPlayerStatus.position;
    pm_gCameras[pm_gCurrentCameraID].updateMode = 3;
    pm_update_cameras();
    fp.cam.eye = pm_gCameras[pm_gCurrentCameraID].lookAt_eye;
    fp.cam.obj = pm_gCameras[pm_gCurrentCameraID].lookAt_obj;
    fp.cam.pitch = pm_gCameras[pm_gCurrentCameraID].currentPitch;
    fp.cam.yaw = pm_gCameras[pm_gCurrentCameraID].currentYaw;
    pm_gCameras[pm_gCurrentCameraID] = cam;
    fp.resetCam = TRUE;
}

static void setCamInputMask(void) {
    if (fp.freeCam && !fp.lockCam) {
        fpSetInputMask(BUTTON_C_RIGHT | BUTTON_C_LEFT | BUTTON_C_DOWN | BUTTON_C_UP | BUTTON_Z | BUTTON_L, 0xFF, 0xFF);
    } else {
        fpSetInputMask(BUTTON_L, 0x00, 0x00);
    }
}

static s32 enableCamProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        if (firstUse) {
            resetCam();
            firstUse = FALSE;
        }
        fp.freeCam = TRUE;
        setCamInputMask();
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        fp.freeCam = FALSE;
        pm_gCameras[pm_gCurrentCameraID].updateMode = 3;
        pm_gCameras[pm_gCurrentCameraID].moveSpeed = 1;
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
    resetCam();
}

struct Menu *createCameraMenu(void) {
    static struct Menu menu;

    int y = 0;
    int menuX = 13;

    /* initialize menu */
    menuInit(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu.selector = menuAddSubmenu(&menu, 0, y++, NULL, "return");

    menuAddStatic(&menu, 0, y, "enable", 0xC0C0C0);
    struct MenuItem *enableButton = menuAddCheckbox(&menu, menuX, y++, enableCamProc, NULL);
    menuAddStatic(&menu, 0, y, "lock", 0xC0C0C0);
    struct MenuItem *lockButton = menuAddCheckbox(&menu, menuX, y++, lockCamProc, NULL);
    struct MenuItem *resetButton = menuAddButton(&menu, 0, y++, "reset", resetCamProc, NULL);
    y++;
    menuAddStatic(&menu, 0, y, "behavior", 0xC0C0C0);
    struct MenuItem *firstOption = menuAddOption(&menu, menuX, y++,
                                                 "manual\0"
                                                 "birdseye follow\0"
                                                 "radial follow\0",
                                                 menuWordOptionmodProc, &fp.camBhv);
    menuAddStatic(&menu, 0, y, "distance min", 0xC0C0C0);
    menuAddIntinput(&menu, menuX, y++, 10, 4, menuHalfwordModProc, &fp.camDistMin);
    menuAddStatic(&menu, 0, y, "distance max", 0xC0C0C0);
    menuAddIntinput(&menu, menuX, y++, 10, 4, menuHalfwordModProc, &fp.camDistMax);
    menuAddStatic(&menu, 0, y, "move speed", 0xC0C0C0);
    menuAddIntinput(&menu, menuX, y++, 10, 3, menuHalfwordModProc, &fp.freeCamMoveSpeed);
    menuAddStatic(&menu, 0, y, "pan speed", 0xC0C0C0);
    menuAddIntinput(&menu, menuX, y++, 10, 3, menuHalfwordModProc, &fp.freeCamPanSpeed);

    menuItemAddChainLink(menu.selector, enableButton, MENU_NAVIGATE_DOWN);
    menuItemAddChainLink(lockButton, resetButton, MENU_NAVIGATE_DOWN);
    menuItemAddChainLink(firstOption, resetButton, MENU_NAVIGATE_UP);
    menuItemAddChainLink(resetButton, lockButton, MENU_NAVIGATE_UP);

    return &menu;
}
