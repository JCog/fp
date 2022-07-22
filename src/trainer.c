#include "trainer.h"
#include "fp.h"
#include "menu.h"
#include "resource.h"
#include "settings.h"
#include <math.h>

enum BowserVariant {
    BOWSER_VARIANT_HALLWAY = 0xC1,
    BOWSER_VARIANT_FINAL_1 = 0xC3,
    BOWSER_VARIANT_FINAL_2 = 0xC5,
};

const char messageForASM[] = "Success";

const static u32 bowserAttacksHallway[] = {
    SCRIPT_BOWSER_HALLWAY_FIRE, SCRIPT_BOWSER_HALLWAY_STOMP, SCRIPT_BOWSER_HALLWAY_CLAW,
    SCRIPT_BOWSER_HALLWAY_WAVE, SCRIPT_BOWSER_HALLWAY_WAVE,
};

const static u32 bowserAttacksFinal1[] = {
    SCRIPT_BOWSER_FINAL_1_FIRE, SCRIPT_BOWSER_FINAL_1_STOMP,     SCRIPT_BOWSER_FINAL_1_CLAW,
    SCRIPT_BOWSER_FINAL_1_WAVE, SCRIPT_BOWSER_FINAL_1_LIGHTNING,
};

const static u32 bowserAttacksFinal2[] = {
    SCRIPT_BOWSER_FINAL_2_FIRE, SCRIPT_BOWSER_FINAL_2_STOMP,     SCRIPT_BOWSER_FINAL_2_CLAW,
    SCRIPT_BOWSER_FINAL_2_WAVE, SCRIPT_BOWSER_FINAL_2_LIGHTNING,
};

// bowser block trainer vars
static bool bowserBlocksEnabled = FALSE;
static u8 bowserAttack = 0;
// clang-format off
static u32 bowserCustomScript[] = {
    EVT_OP_CALL, 3, (uintptr_t)&pm_useIdleAnimation, 0xFFFFFF81, FALSE,
    EVT_OP_EXEC_WAIT, 1, 0, // replaced with attack script
    EVT_OP_CALL, 3, (uintptr_t)&pm_useIdleAnimation, 0xFFFFFF81, TRUE,
    EVT_OP_RETURN, 0,
    EVT_OP_END, 0,
};
// clang-format on

// LZS trainer vars
static bool lzsTrainerEnabled = NULL;
static s8 lzsPrevPressedY = 0;
static u8 lzsPrevPrevActionState = 0;
static bool lzsLzStored = NULL;
static bool lzsPlayerLanded = NULL;
static u16 lzsFramesSinceLand = 0;
static u16 lzsCurrentJumps = 0;
static u16 lzsRecordJumps = 0;

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
    bool goodPos = FALSE;
    bool willClip = FALSE;

    if (pm_gPlayerStatus.position.z >= -26.3686f) {
        // check if in a known position that will clip and respawn OoB
        if ((zPos == -24 && xPos == -184) || (zPos == -25 && (xPos >= -186 && xPos <= -183)) ||
            (zPos == -26 && (xPos >= -186 && xPos <= -182))) {
            goodPos = TRUE;
        }

        // check if in a known position that will clip
        if ((xPos == -186 && (zPos >= -26 && zPos <= -21)) || (xPos == -185 && (zPos >= -26 && zPos <= -22)) ||
            (xPos == -184 && (zPos >= -26 && zPos <= -23)) || (xPos == -183 && (zPos >= -26 && zPos <= -24)) ||
            (xPos == -182 && (zPos >= -26 && zPos <= -25)) || (xPos == -181 && zPos == -26)) {
            willClip = TRUE;
        }
    }

    u32 colorGreen = GPACK_RGB24A8(0x00FF00, 0xFF);
    u32 colorYellow = GPACK_RGB24A8(0xFFFF00, 0xFF);
    u32 colorRed = GPACK_RGB24A8(0xFF0000, 0xFF);
    u32 colorWhite = GPACK_RGB24A8(0xFFFFFF, 0xFF);
    s32 menuY = 0;

    gfxPrintf(font, x, y + chHeight * menuY++, "x:     %.4f", pm_gPlayerStatus.position.x);
    gfxPrintf(font, x, y + chHeight * menuY++, "z:     %.4f", pm_gPlayerStatus.position.z);
    gfxPrintf(font, x, y + chHeight * menuY, "angle:");
    if (pm_gPlayerStatus.currentYaw >= 43.9f && pm_gPlayerStatus.currentYaw <= 46.15f) {
        gfxModeSet(GFX_MODE_COLOR, colorGreen);
    } else {
        gfxModeSet(GFX_MODE_COLOR, colorWhite);
    }
    gfxPrintf(font, x + chWidth * 7, y + chHeight * menuY++, "%.2f", pm_gPlayerStatus.currentYaw);
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
    gfxPrintf(font, x, y + chHeight * menuY, "pos:");
    if (goodPos) {
        gfxModeSet(GFX_MODE_COLOR, colorGreen);
        gfxPrintf(font, x + chWidth * 7, y + chHeight * menuY, "good");
    } else if (willClip) {
        gfxModeSet(GFX_MODE_COLOR, colorYellow);
        gfxPrintf(font, x + chWidth * 7, y + chHeight * menuY, "inconsistent");
    } else {
        gfxModeSet(GFX_MODE_COLOR, colorRed);
        gfxPrintf(font, x + chWidth * 7, y + chHeight * menuY, "bad");
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

    u32 colorGreen = GPACK_RGB24A8(0x00FF00, 0xFF);
    u32 colorRed = GPACK_RGB24A8(0xFF0000, 0xFF);
    u32 colorWhite = GPACK_RGB24A8(0xFFFFFF, 0xFF);

    gfxPrintf(font, x + chWidth * 0, y + chHeight * 0, "effects:");
    gfxPrintf(font, x + chWidth * 0, y + chHeight * 1, "flags:");
    gfxPrintf(font, x + chWidth * 0, y + chHeight * 2, "frame window:");

    if (effectCount == 81) {
        gfxModeSet(GFX_MODE_COLOR, colorGreen);
    } else {
        gfxModeSet(GFX_MODE_COLOR, colorWhite);
    }
    gfxPrintf(font, x + chWidth * 14, y + chHeight * 0, "%d", effectCount);
    gfxModeSet(GFX_MODE_COLOR, colorWhite);
    if (pm_gPlayerStatus.animFlags == 0x01000000) {
        gfxModeSet(GFX_MODE_COLOR, colorGreen);
        gfxPrintf(font, x + chWidth * 14, y + chHeight * 1, "good");
    } else {
        gfxModeSet(GFX_MODE_COLOR, colorRed);
        gfxPrintf(font, x + chWidth * 14, y + chHeight * 1, "bad");
    }
    gfxModeSet(GFX_MODE_COLOR, colorWhite);
    gfxPrintf(font, x + chWidth * 14, y + chHeight * 2, "%d", fp.aceFrameWindow);

    if (fp.aceLastTimer != 0) {
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
        gfxPrintf(font, x + chWidth * 0, y + chHeight * 7, "last attempt status:");
        gfxPrintf(font, x + chWidth * 0, y + chHeight * 8, "timer:");
        gfxPrintf(font, x + chWidth * 0, y + chHeight * 9, "flags:");
        gfxPrintf(font, x + chWidth * 0, y + chHeight * 10, "jump:");

        if (fp.aceLastTimer <= 0x81f && fp.aceLastTimer > 0x81f - fp.aceFrameWindow) {
            gfxModeSet(GFX_MODE_COLOR, colorGreen);
        } else {
            gfxModeSet(GFX_MODE_COLOR, colorRed);
        }
        gfxPrintf(font, x + chWidth * 7, y + chHeight * 8, "0x%x", fp.aceLastTimer);

        if (fp.aceLastFlagStatus) {
            gfxModeSet(GFX_MODE_COLOR, colorGreen);
            gfxPrintf(font, x + chWidth * 7, y + chHeight * 9, "good");
        } else {
            gfxModeSet(GFX_MODE_COLOR, colorRed);
            gfxPrintf(font, x + chWidth * 7, y + chHeight * 9, "bad");
        }

        if (fp.aceLastJumpStatus) {
            gfxModeSet(GFX_MODE_COLOR, colorGreen);
            gfxPrintf(font, x + chWidth * 7, y + chHeight * 10, "good");
        } else {
            gfxModeSet(GFX_MODE_COLOR, colorRed);
            gfxPrintf(font, x + chWidth * 7, y + chHeight * 10, "bad");
        }

        if (fp.aceLastFlagStatus && fp.aceLastJumpStatus && fp.aceLastTimer <= 0x81f &&
            fp.aceLastTimer > 0x81f - fp.aceFrameWindow) {
            gfxModeSet(GFX_MODE_COLOR, colorGreen);
            gfxPrintf(font, x + chWidth * 0, y + chHeight * 11, "success");
        } else {
            gfxModeSet(GFX_MODE_COLOR, colorRed);
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

static void updateBowserBlockTrainer(void) {
    if (pm_gGameStatus.isBattle) {
        pm_Actor *enemy0 = pm_gBattleStatus.enemyActors[0];
        if (enemy0) {
            bool isBowser = FALSE;
            u32 vanillaScript;
            switch (enemy0->actorType) {
                case BOWSER_VARIANT_HALLWAY:
                    isBowser = TRUE;
                    bowserCustomScript[7] = bowserAttacksHallway[bowserAttack];
                    vanillaScript = SCRIPT_BOWSER_HALLWAY_TAKE_TURN;
                    break;
                case BOWSER_VARIANT_FINAL_1:
                    isBowser = TRUE;
                    bowserCustomScript[7] = bowserAttacksFinal1[bowserAttack];
                    vanillaScript = SCRIPT_BOWSER_FINAL_1_TAKE_TURN;
                    break;
                case BOWSER_VARIANT_FINAL_2:
                    isBowser = TRUE;
                    bowserCustomScript[7] = bowserAttacksFinal2[bowserAttack];
                    vanillaScript = SCRIPT_BOWSER_FINAL_2_TAKE_TURN;
                    break;
            }

            if (isBowser) {
                if (bowserBlocksEnabled) {
                    enemy0->state.varTable[0] = 2; // total turns, to make bowser stop talking
                    enemy0->takeTurnScriptSource = (void *)&bowserCustomScript;

                    // never let wave KO last more than one turn so you can keep practicing the block
                    if (pm_gBattleStatus.partnerActor && pm_gBattleStatus.partnerActor->koDuration > 1) {
                        pm_gBattleStatus.partnerActor->koDuration = 1;
                    }
                } else {
                    enemy0->takeTurnScriptSource = (void *)vanillaScript;
                }
            }
        }
    }
}

static void updateLzsTrainer(void) {
    // detect if loading zone is stored
    for (s32 evtIdx = 0; evtIdx < pm_gNumScripts; evtIdx++) {
        pm_Evt *script = (*pm_gCurrentScriptListPtr)[pm_gScriptIndexList[evtIdx]];
        if (script && script->ptrNextLine) {
            u32 callbackFunction = script->ptrNextLine[5];
            if (callbackFunction == (uintptr_t)pm_gotoMap) {
                lzsLzStored = TRUE;
            }
        }
    }

    // Count frames since mario landed
    if (pm_gPlayerStatus.actionState == ACTION_STATE_LAND || pm_gPlayerStatus.actionState == ACTION_STATE_WALK ||
        pm_gPlayerStatus.actionState == ACTION_STATE_RUN) {
        lzsPlayerLanded = TRUE;
    }
    if (lzsPlayerLanded) {
        lzsFramesSinceLand++;
    } else {
        lzsFramesSinceLand = 0;
    }
    if (pm_gPlayerStatus.actionState == ACTION_STATE_JUMP) {
        lzsPlayerLanded = FALSE;
    }

    // log lzs status
    if (lzsLzStored && pm_gGameStatus.pressedButtons[0].a) {
        if (lzsPrevPrevActionState == ACTION_STATE_FALLING && pm_gPlayerStatus.actionState == ACTION_STATE_JUMP &&
            pm_mapChangeState == 0) {
            fpLog("control early");
        } else if (pm_gPlayerStatus.prevActionState == ACTION_STATE_JUMP ||
                   pm_gPlayerStatus.actionState == ACTION_STATE_SPIN_JUMP ||
                   pm_gPlayerStatus.actionState == ACTION_STATE_ULTRA_JUMP) {
            fpLog("jump >= 2 frames early");
            if (pm_gGameStatus.pressedButtons[0].yCardinal || lzsPrevPressedY) {
                fpLog("control early");
            }
        } else if (pm_gPlayerStatus.prevActionState == ACTION_STATE_FALLING) {
            fpLog("jump 1 frame early");
            if (pm_gPlayerStatus.actionState == ACTION_STATE_RUN || pm_gPlayerStatus.actionState == ACTION_STATE_WALK) {
                fpLog("control early");
            }
        } else if (lzsPrevPrevActionState == ACTION_STATE_FALLING && pm_mapChangeState == 0) {
            fpLog("jump 1 frame late");
            fpLog("control early");
        } else if (lzsFramesSinceLand == 3) {
            fpLog("jump 1 frame late");
            if (pm_gGameStatus.pressedButtons[0].yCardinal) {
                fpLog("control late");
            }
        } else if (lzsFramesSinceLand == 4) {
            fpLog("jump 2 frames late");
            if (pm_gGameStatus.pressedButtons[0].yCardinal || lzsPrevPressedY) {
                fpLog("control late");
            }
        } else if (lzsFramesSinceLand == 0 &&
                   (lzsPrevPrevActionState == ACTION_STATE_RUN || lzsPrevPrevActionState == ACTION_STATE_WALK)) {
            fpLog("jump >= 2 frames late");
            fpLog("control early");
        } else if (lzsFramesSinceLand >= 5 && pm_mapChangeState == 0) {
            fpLog("jump > 2 frames late");
            if (pm_gGameStatus.pressedButtons[0].yCardinal || lzsPrevPressedY) {
                fpLog("control late");
            }
        } else if (lzsFramesSinceLand == 2) {
            lzsCurrentJumps++;
        }
    }

    if (lzsCurrentJumps > lzsRecordJumps) {
        lzsRecordJumps = lzsCurrentJumps;
    }

    lzsPrevPressedY = pm_gGameStatus.pressedButtons[0].yCardinal;
    lzsPrevPrevActionState = pm_gPlayerStatus.prevActionState;

    if (pm_mapChangeState == 1) {
        lzsLzStored = FALSE;
        lzsPlayerLanded = FALSE;
        lzsFramesSinceLand = 0;
        lzsCurrentJumps = 0;
    }
}

void trainerUpdate(void) {
    updateBowserBlockTrainer();
    updateLzsTrainer();
}

void createTrainerMenu(struct Menu *menu) {
    static struct Menu lzsMenu;
    static struct Menu issMenu;
    static struct Menu aceMenu;

    /* initialize menu */
    menuInit(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&lzsMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&issMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&aceMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    /*build menu*/
    struct GfxTexture *wrench = resourceLoadGrcTexture("wrench");
    s32 y = 0;
#if PM64_VERSION == JP
    s32 xOffset = 19;
#else
    s32 xOffset = 16;
#endif
    menu->selector = menuAddSubmenu(menu, 0, y++, NULL, "return");

    menuAddStatic(menu, 0, y, "bowser blocks", 0xC0C0C0);
    menuAddCheckbox(menu, xOffset, y, checkboxModProc, &bowserBlocksEnabled);
    menuAddOption(menu, xOffset + 2, y++,
                  "fire\0"
                  "butt stomp\0"
                  "claw\0"
                  "wave\0"
                  "lightning\0",
                  byteOptionmodProc, &bowserAttack);

    menuAddStatic(menu, 0, y, "lzs jumps", 0xC0C0C0);
    menuAddCheckbox(menu, xOffset, y, checkboxModProc, &lzsTrainerEnabled);
    menuAddSubmenuIcon(menu, xOffset + 2, y++, &lzsMenu, wrench, 0, 0, 1.0f);

    menuAddStatic(menu, 0, y, "action commands", 0xC0C0C0);
    menuAddCheckbox(menu, xOffset, y++, checkboxModProc, &fp.actionCommandTrainerEnabled);

    menuAddStatic(menu, 0, y, "clippy", 0xC0C0C0);
    menuAddCheckbox(menu, xOffset, y++, checkboxModProc, &fp.clippyTrainerEnabled);
#if PM64_VERSION == JP
    menuAddStatic(menu, 0, y, "ice staircase skip", 0xC0C0C0);
    menuAddSubmenuIcon(menu, xOffset, y++, &issMenu, wrench, 0, 0, 1.0f);

    menuAddStatic(menu, 0, y, "oot ace", 0xC0C0C0);
    menuAddSubmenuIcon(menu, xOffset, y++, &aceMenu, wrench, 0, 0, 1.0f);
#endif

    /*build lzs jump menu*/
    lzsMenu.selector = menuAddSubmenu(&lzsMenu, 0, 0, NULL, "return");
    menuAddStatic(&lzsMenu, 0, 1, "current lzs jumps: ", 0xC0C0C0);
    menuAddWatch(&lzsMenu, 20, 1, (u32)&lzsCurrentJumps, WATCH_TYPE_U16);
    menuAddStatic(&lzsMenu, 0, 2, "record lzs jumps: ", 0xC0C0C0);
    menuAddWatch(&lzsMenu, 20, 2, (u32)&lzsRecordJumps, WATCH_TYPE_U16);

    /*build iss menu*/
    issMenu.selector = menuAddSubmenu(&issMenu, 0, 0, NULL, "return");
    menuAddStaticCustom(&issMenu, 0, 1, issDrawProc, NULL, 0xC0C0C0);

    /*build ace menu*/
    aceMenu.selector = menuAddSubmenu(&aceMenu, 0, 0, NULL, "return");
    menuAddStaticCustom(&aceMenu, 0, 1, aceDrawProc, NULL, 0xC0C0C0);
    menuAddButton(&aceMenu, 0, 5, "practice payload", acePracticePayloadProc, NULL);
    menuAddButton(&aceMenu, 0, 6, "oot instruction", aceOotInstrProc, NULL);
}
