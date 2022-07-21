#include "trainer.h"
#include "fp.h"
#include "menu.h"
#include "settings.h"
#include <math.h>

char messageForASM[] = "Success";

extern void setACEHook(void);

s32 getMatrixTotal(void) {
    s32 matrixCount = 0;

    for (s32 i = 0; i < 0x60; i++) {
        if (pm_gEffectInstances[i] != NULL) {
            matrixCount += pm_gEffectInstances[i]->numParts;
        }
    }
    return matrixCount;
}

void clearAllEffectsManual(s32 matrixCount) {
    s32 var = 0;

    if (matrixCount == 0x215) {
        var = 1;
        fp.aceLastFlagStatus = pm_gPlayerStatus.animFlags == 0x01000000;
        fp.aceLastTimer = pm_gPlayerStatus.currentStateTime;
        fp.aceLastJumpStatus = (pm_gPlayerStatus.flags & 0xff) == 3;
        fpLog("Successful ACE, jump prevented");
    }
    if (matrixCount > 0x215) { // matrix limit reached, destroy all effects
        var = 1;
        fpLog("Matrix overflow, crash prevented");
    }

    if (var == 1) {
        for (s32 i = 0; i < 0x60; i++) {
            if (pm_gEffectInstances[i] != NULL) {
                pm_removeEffect(pm_gEffectInstances[i]);
            }
        }
    }
}

asm(".set noreorder;"
    "ace_remove_matrices:;"
    "JAL getMatrixTotal;"
    "NOP;"
    "JAL clearAllEffectsManual;"
    "DADDU $a0, $v0, $zero;"
    "LUI $s1, 0x800B;"
    "ADDIU $s1, $s1, 0x4378;"
    "J 0x80059A7C;"
    "NOP;"
    "jump_instruction_ace_remove_matrices:;"
    "J ace_remove_matrices;"
    "NOP;");

asm(".set noreorder;"
    "setACEHook:;"
    "LA $t0, 0x80059A74;" // where to hook
    "LA $t1, jump_instruction_ace_remove_matrices;"
    "LW $t1, 0x0000 ($t1);"
    "SW $zero, 0x0004 ($t0);"
    "JR $ra;"
    "SW $t1, 0x0000 ($t0);");

static s32 checkboxModProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    u8 *p = data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        *p = 1;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        *p = 0;
    } else if (reason == MENU_CALLBACK_THINK) {
        menuCheckboxSet(item, *p);
    }
    return 0;
}

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

static s32 issDrawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
    struct GfxFont *font = drawParams->font;
    s32 chHeight = menuGetCellHeight(item->owner, TRUE);
    s32 chWidth = menuGetCellWidth(item->owner, TRUE);
    s32 x = drawParams->x;
    s32 y = drawParams->y;

    s32 xPos = ceil(pm_gPlayerStatus.position.x);
    s32 zPos = ceil(pm_gPlayerStatus.position.z);
    bool goodPos = 0;
    bool willClip = 0;

    if (pm_gPlayerStatus.position.z >= -26.3686f) {
        // check if in a known position that will clip and respawn OoB
        if (zPos == -24 && xPos == -184) {
            goodPos = TRUE;
        } else if (zPos == -25 && (xPos >= -186 && xPos <= -183)) {
            goodPos = TRUE;
        } else if (zPos == -26 && (xPos >= -186 && xPos <= -182)) {
            goodPos = TRUE;
        }

        // check if in a known position that will clip
        if (xPos == -186 && (zPos >= -26 && zPos <= -21)) {
            willClip = TRUE;
        } else if (xPos == -185 && (zPos >= -26 && zPos <= -22)) {
            willClip = TRUE;
        } else if (xPos == -184 && (zPos >= -26 && zPos <= -23)) {
            willClip = TRUE;
        } else if (xPos == -183 && (zPos >= -26 && zPos <= -24)) {
            willClip = TRUE;
        } else if (xPos == -182 && (zPos >= -26 && zPos <= -25)) {
            willClip = TRUE;
        } else if (xPos == -181 && zPos == -26) {
            willClip = TRUE;
        }
    }

    s32 menuY = 0;
    gfxPrintf(font, x, y + chHeight * menuY++, "x: %.4f", pm_gPlayerStatus.position.x);
    gfxPrintf(font, x, y + chHeight * menuY++, "z: %.4f", pm_gPlayerStatus.position.z);
    gfxPrintf(font, x, y + chHeight * menuY, "angle: ");
    if (pm_gPlayerStatus.currentYaw >= 43.9f && pm_gPlayerStatus.currentYaw <= 46.15f) {
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0xFF, 0x00, 0xFF));
    }
    gfxPrintf(font, x + chWidth * 7, y + chHeight * menuY++, "%.2f", pm_gPlayerStatus.currentYaw);
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0xFF, 0xFF, 0xFF));
    gfxPrintf(font, x, y + chHeight * menuY, "position: ");
    if (goodPos) {
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0xFF, 0x00, 0xFF));
        gfxPrintf(font, x + chWidth * 10, y + chHeight * menuY++, "good");
    } else if (willClip) {
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0xFF, 0x00, 0xFF));
        gfxPrintf(font, x + chWidth * 10, y + chHeight * menuY++, "inconsistent");
    } else {
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0x00, 0x00, 0xFF));
        gfxPrintf(font, x + chWidth * 10, y + chHeight * menuY++, "bad");
    }
    return 1;
}

static s32 aceDrawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
    struct GfxFont *font = drawParams->font;
    s32 chHeight = menuGetCellHeight(item->owner, TRUE);
    s32 chWidth = menuGetCellWidth(item->owner, TRUE);
    s32 x = drawParams->x;
    s32 y = drawParams->y;

    s32 effectCount = 0;
    s32 i;
    for (i = 0; i < 96; i++) {
        if (pm_gEffectInstances[i]) {
            effectCount++;
        }
    }

    gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0xC0, 0xC0, 0xC0, 0xFF)); // gray
    gfxPrintf(font, x + chWidth * 0, y + chHeight * 0, "effects:");
    gfxPrintf(font, x + chWidth * 0, y + chHeight * 1, "flags:");
    gfxPrintf(font, x + chWidth * 0, y + chHeight * 2, "frame window:");

    if (effectCount == 81) {
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0xFF, 0x00, 0xFF)); // green
    } else {
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0xFF, 0xFF, 0xFF)); // white
    }
    gfxPrintf(font, x + chWidth * 14, y + chHeight * 0, "%d", effectCount);
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0xFF, 0xFF, 0xFF)); // white
    if (pm_gPlayerStatus.animFlags == 0x01000000) {
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0xFF, 0x00, 0xFF)); // green
        gfxPrintf(font, x + chWidth * 14, y + chHeight * 1, "good");
    } else {
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0x00, 0x00, 0xFF)); // red
        gfxPrintf(font, x + chWidth * 14, y + chHeight * 1, "bad");
    }
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0xFF, 0xFF, 0xFF)); // white
    gfxPrintf(font, x + chWidth * 14, y + chHeight * 2, "%d", fp.aceFrameWindow);

    if (fp.aceLastTimer != 0) {
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0xC0, 0xC0, 0xC0, 0xFF)); // gray
        gfxPrintf(font, x + chWidth * 0, y + chHeight * 7, "last attempt status:");
        gfxPrintf(font, x + chWidth * 0, y + chHeight * 8, "timer:");
        gfxPrintf(font, x + chWidth * 0, y + chHeight * 9, "flags:");
        gfxPrintf(font, x + chWidth * 0, y + chHeight * 10, "jump:");

        if (fp.aceLastTimer <= 0x81f && fp.aceLastTimer > 0x81f - fp.aceFrameWindow) {
            gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0xFF, 0x00, 0xFF)); // green
        } else {
            gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0x00, 0x00, 0xFF)); // red
        }
        gfxPrintf(font, x + chWidth * 7, y + chHeight * 8, "0x%x", fp.aceLastTimer);

        if (fp.aceLastFlagStatus) {
            gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0xFF, 0x00, 0xFF)); // green
            gfxPrintf(font, x + chWidth * 7, y + chHeight * 9, "good");
        } else {
            gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0x00, 0x00, 0xFF)); // red
            gfxPrintf(font, x + chWidth * 7, y + chHeight * 9, "bad");
        }

        if (fp.aceLastJumpStatus) {
            gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0xFF, 0x00, 0xFF)); // green
            gfxPrintf(font, x + chWidth * 7, y + chHeight * 10, "good");
        } else {
            gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0x00, 0x00, 0xFF)); // red
            gfxPrintf(font, x + chWidth * 7, y + chHeight * 10, "bad");
        }

        if (fp.aceLastFlagStatus && fp.aceLastJumpStatus && fp.aceLastTimer <= 0x81f &&
            fp.aceLastTimer > 0x81f - fp.aceFrameWindow) {
            gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0xFF, 0x00, 0xFF)); // green
            gfxPrintf(font, x + chWidth * 0, y + chHeight * 11, "success");
        } else {
            gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0x00, 0x00, 0xFF)); // red
            gfxPrintf(font, x + chWidth * 0, y + chHeight * 11, "failure");
        }
    }
    return 1;
}

static void acePracticePayloadProc(struct MenuItem *item, void *data) {
    setACEHook();
    fpLog("practice payload placed");
}

static void aceOotInstrProc(struct MenuItem *item, void *data) {
    // write jump to jp file names to addr 0x807C0000
    fpLog("oot instruction placed");
    __asm__("LA $t0, 0x807C0000;"
            "LA $t1, 0x0801DE67;"
            "SW $t1, 0x0000 ($t0);");
}

static s32 lzsDrawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
    struct GfxFont *font = drawParams->font;
    s32 chHeight = menuGetCellHeight(item->owner, TRUE);
    s32 chWidth = menuGetCellWidth(item->owner, TRUE);
    s32 x = drawParams->x;
    s32 y = drawParams->y;

    gfxPrintf(font, x, y + chHeight * 1, "current lzs jumps: ");
    gfxPrintf(font, x + chWidth * 20, y + chHeight * 1, "%d", fp.currentLzsJumps);
    gfxPrintf(font, x + chWidth * 0, y + chHeight * 2, "record lzs jumps: ");
    gfxPrintf(font, x + chWidth * 20, y + chHeight * 2, "%d", fp.recordLzsJumps);

    return 1;
}

void createTrainerMenu(struct Menu *menu) {
    static struct Menu bowserMenu;
    static struct Menu issMenu;
    static struct Menu aceMenu;
    static struct Menu lzsMenu;
    static struct Menu clippyMenu;
    static struct Menu actionCommandMenu;

    /* initialize menu */
    menuInit(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&bowserMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&issMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&aceMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&lzsMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&clippyMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&actionCommandMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu->selector = menuAddSubmenu(menu, 0, 0, NULL, "return");

    /*build menu*/
    s32 yValue = 1;
    menuAddSubmenu(menu, 0, yValue++, &actionCommandMenu, "action commands");
    menuAddSubmenu(menu, 0, yValue++, &bowserMenu, "bowser blocks");
    menuAddSubmenu(menu, 0, yValue++, &clippyMenu, "clippy");
#if PM64_VERSION == JP
    menuAddSubmenu(menu, 0, yValue++, &issMenu, "ice staircase skip");
#endif
    menuAddSubmenu(menu, 0, yValue++, &lzsMenu, "lzs jumps");
#if PM64_VERSION == JP
    menuAddSubmenu(menu, 0, yValue++, &aceMenu, "oot ace");
#endif

    /*build bowser menu*/
    yValue = 0;
    bowserMenu.selector = menuAddSubmenu(&bowserMenu, 0, yValue++, NULL, "return");
    menuAddStatic(&bowserMenu, 0, yValue, "enabled", 0xC0C0C0);
    menuAddCheckbox(&bowserMenu, 8, yValue++, checkboxModProc, &fp.bowserBlocksEnabled);
    menuAddStatic(&bowserMenu, 0, yValue, "attack", 0xC0C0C0);
    menuAddOption(&bowserMenu, 8, yValue++,
                  "fire\0"
                  "butt stomp\0"
                  "claw\0"
                  "wave\0"
                  "lightning\0",
                  byteOptionmodProc, &fp.bowserBlock);

    /*build iss menu*/
    issMenu.selector = menuAddSubmenu(&issMenu, 0, 0, NULL, "return");
    menuAddStaticCustom(&issMenu, 0, 1, issDrawProc, NULL, 0xFFFFFF);

    /*build ace menu*/
    aceMenu.selector = menuAddSubmenu(&aceMenu, 0, 0, NULL, "return");
    menuAddStaticCustom(&aceMenu, 0, 1, aceDrawProc, NULL, 0xFFFFFF);
    menuAddButton(&aceMenu, 0, 5, "practice payload", acePracticePayloadProc, NULL);
    menuAddButton(&aceMenu, 0, 6, "oot instruction", aceOotInstrProc, NULL);

    /*build lzs jump menu*/
    lzsMenu.selector = menuAddSubmenu(&lzsMenu, 0, 0, NULL, "return");
    menuAddStatic(&lzsMenu, 0, 1, "enabled", 0xC0C0C0);
    menuAddCheckbox(&lzsMenu, 8, 1, checkboxModProc, &fp.lzsTrainerEnabled);
    menuAddStaticCustom(&lzsMenu, 0, 2, lzsDrawProc, NULL, 0xFFFFFF);

    /*build clippy menu*/
    clippyMenu.selector = menuAddSubmenu(&clippyMenu, 0, 0, NULL, "return");
    menuAddStatic(&clippyMenu, 0, 1, "enabled", 0xC0C0C0);
    menuAddCheckbox(&clippyMenu, 8, 1, checkboxModProc, &fp.clippyTrainerEnabled);

    /*build action command menu*/
    actionCommandMenu.selector = menuAddSubmenu(&actionCommandMenu, 0, 0, NULL, "return");
    menuAddStatic(&actionCommandMenu, 0, 1, "enabled", 0xC0C0C0);
    menuAddCheckbox(&actionCommandMenu, 8, 1, checkboxModProc, &fp.actionCommandTrainerEnabled);
}
