#include "trainer.h"
#include "fp.h"
#include "menu/menu.h"
#include "sys/input.h"
#include "sys/resource.h"
#include "sys/settings.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

enum BowserVariant {
    BOWSER_VARIANT_HALLWAY = 0xC1,
    BOWSER_VARIANT_FINAL_1 = 0xC3,
    BOWSER_VARIANT_FINAL_2 = 0xC5,
};

enum BlockResult {
    BLOCK_EARLY = -1,
    BLOCK_LATE = 0,
    BLOCK_SUCCESS = 1,
    BLOCK_NONE = 127,
};

enum ClippyStatus {
    CLIPPY_NONE,
    CLIPPY_EARLY,
    CLIPPY_SUCCESS,
    CLIPPY_LATE,
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
static u8 bowserAttack = 0;
// clang-format off
static u32 bowserCustomScript[] = {
    EVT_OP_CALL, 3, (uintptr_t)&pm_UseIdleAnimation, 0xFFFFFF81, FALSE,
    EVT_OP_EXEC_WAIT, 1, 0, // replaced with attack script
    EVT_OP_CALL, 3, (uintptr_t)&pm_UseIdleAnimation, 0xFFFFFF81, TRUE,
    EVT_OP_RETURN, 0,
    EVT_OP_END, 0,
};
// clang-format on

// LZS trainer vars
static u8 lzsPrevPressedAnalog = 0;
static u8 lzsPrevPrevActionState = 0;
static bool lzsLzStored = NULL;
static bool lzsPlayerLanded = NULL;
static u16 lzsFramesSinceLand = 0;
static u16 lzsCurrentJumps = 0;
static u16 lzsRecordJumps = 0;

// action command trainer vars
static bool acWaitingForMissedBlock = FALSE;
static s32 acPushInputBuffer[64];
static s8 acInputBufferPos = 0;
static s8 acBlockFramesLate = 0;
static u16 acLastAPress = 0;
static u16 acLastValidFrame = 0;

// clippy trainer vars
static u16 clippyFramesSinceBattle = 0;
static u8 clippyStatus = 0;

// spin trainer vars
static enum ActionStates spinPrevActionState = ACTION_STATE_IDLE;
static bool spinPrevCanSpin = FALSE;
static s8 spinCanceled = FALSE;
static s8 spinEndTiming = 0;
static bool spinIsJumping = FALSE;
static u16 spinJumpStart = 0;
static s16 spinJumpLast = 1;
static u16 spinDelayStart = 0;
static u16 spinDelayLast = 0;
static struct GfxTexture *spinJumpTex = NULL;
static struct GfxTexture *spinDelayTex = NULL;

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
                pm_remove_effect(pm_gEffectInstances[i]);
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

static void unpinProc(struct MenuItem *item, void *data) {
    settings->trainerDisplayPinned = FALSE;
}

static s32 pinnedTrainerProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    enum PinnedTrainer p = (u32)data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        settings->pinnedTrainer = p;
        settings->trainerDisplayPinned = TRUE;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        settings->trainerDisplayPinned = FALSE;
    } else if (reason == MENU_CALLBACK_THINK) {
        menuCheckboxSet(item, settings->trainerDisplayPinned && settings->pinnedTrainer == p);
    }
    return 0;
}

static void lzsDraw(s32 x, s32 y, struct GfxFont *font, s32 chWidth, s32 chHeight, u32 color, u8 alpha) {
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(color, alpha));
    s32 menuY = 0;
    gfxPrintf(font, x, y + chHeight * menuY++, "current lzs jumps: %d", lzsCurrentJumps);
    gfxPrintf(font, x, y + chHeight * menuY++, "record lzs jumps:  %d", lzsRecordJumps);
}

static s32 lzsDrawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    lzsDraw(drawParams->x, drawParams->y, drawParams->font, menuGetCellWidth(item->owner, TRUE),
            menuGetCellHeight(item->owner, TRUE), drawParams->color, drawParams->alpha);
    return 1;
}

static void spinDraw(s32 x, s32 y, struct GfxFont *font, s32 chWidth, s32 chHeight, u32 color, u8 alpha) {
    enum ActionStates actionState = pm_gPlayerStatus.actionState;
    pm_PlayerSpinState *spinState = &pm_gPlayerSpinState;
    bool isSpeedy = pm_is_ability_active(ABILITY_SPEEDY_SPIN);

    // end timing and jump length
    if (spinPrevActionState == ACTION_STATE_SPIN) {
        if (actionState == ACTION_STATE_JUMP || actionState == ACTION_STATE_HAMMER) {
            spinCanceled = FALSE;
            s8 perfectEndTiming = 7;
            if (isSpeedy) {
                perfectEndTiming = 8;
            }
            spinEndTiming = spinState->spinCountdown - perfectEndTiming;

            if (pm_gPlayerStatus.actionState == ACTION_STATE_JUMP) {
                spinIsJumping = TRUE;
                spinJumpStart = pm_gGameStatus.frameCounter;
            }
        } else if (actionState != ACTION_STATE_SPIN) {
            spinCanceled = TRUE;
        }
    } else if (spinIsJumping && (!pm_gGameStatus.currentButtons[0].a || actionState != ACTION_STATE_JUMP)) {
        spinIsJumping = FALSE;
        spinJumpLast = pm_gGameStatus.frameCounter - spinJumpStart;
    }

    // delay between spins
    bool canSpin = !(pm_gPlayerStatus.flags & (PS_FLAG_NO_STATIC_COLLISION | PS_FLAG_CUTSCENE_MOVEMENT |
                                               PS_FLAG_INPUT_DISABLED | PS_FLAG_PAUSED) ||
                     pm_gPlayerStatus.animFlags & (PA_FLAG_USING_WATT | PA_FLAG_SPINNING)) &&
                   actionState < ACTION_STATE_JUMP;
    if (canSpin && !spinPrevCanSpin) {
        spinDelayStart = pm_gGameStatus.frameCounter;
        if (!(actionState == ACTION_STATE_RUN && pm_gPlayerStatus.prevActionState == ACTION_STATE_WALK &&
              spinPrevActionState == ACTION_STATE_IDLE)) {
            // note that there are a lot of rare edge cases here that just aren't worth handling
            spinDelayStart += 1;
        }
    }
    if (actionState == ACTION_STATE_SPIN && spinState->initialSpinTime - spinState->spinCountdown == 1) {
        // start of new spin
        if (!spinPrevCanSpin) {
            spinDelayLast = 0;
        } else {
            spinDelayLast = pm_gGameStatus.frameCounter - spinDelayStart;
        }
    }
    spinPrevActionState = actionState;
    spinPrevCanSpin = canSpin;

    // begin drawing
    u32 colorWhite = GPACK_RGB24A8(0xFFFFFF, 0xFF);
    u32 colorBlackT = GPACK_RGB24A8(0x000000, 0x60);
    u32 colorGreen = GPACK_RGB24A8(0x00FF00, 0xFF);
    u32 colorYellow = GPACK_RGB24A8(0xFFFF00, 0xFF);
    u32 colorRed = GPACK_RGB24A8(0xFF0000, 0xFF);
    u32 colorBackground = GPACK_RGB24A8(0x000000, alpha);
    u32 colorEmpty = GPACK_RGB24A8(0x000095, alpha);
    u32 colorFull = GPACK_RGB24A8(0x00E486, alpha);

    u8 barMult = 3;
    s8 spinFrame = 25 - pm_gPlayerSpinState.spinCountdown;
    u16 barWidth = 25;
    u16 barGoal = 18;
    if (isSpeedy) {
        spinFrame = 30 - pm_gPlayerSpinState.spinCountdown;
        barWidth = 30;
        barGoal = 22;
    }
    barWidth *= barMult;
    barGoal *= barMult;
    u16 barX = x - barWidth / 2;
    u16 barGoalX = barX + barGoal;
    u16 barFullX = barX + spinFrame * barMult;
    u16 barYBottom = y + 10;

    s32 iconY = y + 12;
    s32 textY = iconY + 11;
    s32 jumpIconX = x - 30;
    s32 jumpTextX = jumpIconX + 17;
    s32 delayIconX = x + 2;
    s32 delayTextX = delayIconX + 17;

    // draw background
    gfxModeSet(GFX_MODE_COLOR, colorBlackT);
    gfxModeReplace(GFX_MODE_COMBINE, G_CC_MODE(G_CC_PRIMITIVE, G_CC_PRIMITIVE));
    gfxDisp(gsSPScisTextureRectangle(qs102(barX - 4), qs102(y - 4), qs102(barX + barWidth + 4), qs102(y + 30), 0, 0, 0,
                                     0, 0));

    // draw bar
    // background
    gfxModeSet(GFX_MODE_COLOR, colorBackground);
    gfxDisp(gsSPScisTextureRectangle(qs102(barX - 1), qs102(y - 1), qs102(barX + barWidth + 1), qs102(barYBottom + 1),
                                     0, 0, 0, 0, 0));
    // empty bar
    gfxModeSet(GFX_MODE_COLOR, colorEmpty);
    gfxDisp(gsSPScisTextureRectangle(qs102(barX), qs102(y), qs102(barX + barWidth), qs102(barYBottom), 0, 0, 0, 0, 0));
    // full bar
    gfxModeSet(GFX_MODE_COLOR, colorFull);
    gfxDisp(gsSPScisTextureRectangle(qs102(barX), qs102(y), qs102(barFullX), qs102(barYBottom), 0, 0, 0, 0, 0));
    // goal marker
    gfxModeSet(GFX_MODE_COLOR, colorYellow);
    gfxDisp(
        gsSPScisTextureRectangle(qs102(barGoalX - 1), qs102(y), qs102(barGoalX + 1), qs102(barYBottom), 0, 0, 0, 0, 0));
    gfxModePop(GFX_MODE_COMBINE);

    gfxModeSet(GFX_MODE_COLOR, colorWhite);
    gfxModeReplace(GFX_MODE_DROPSHADOW, 0);
    struct GfxSprite jumpSprite = {spinJumpTex, 0, 0, jumpIconX, iconY, 0.5f, 0.5f};
    struct GfxSprite delaySprite = {spinDelayTex, 0, 0, delayIconX, iconY, 1.f, 1.f};
    gfxSpriteDraw(&jumpSprite);
    gfxSpriteDraw(&delaySprite);
    gfxModePop(GFX_MODE_DROPSHADOW);

    // end timing
    // if (spinCanceled) {
    //     gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(color, alpha));
    //     gfxPrintf(font, timingX, textY, " -");
    // } else if (spinEndTiming == 0) {
    //     gfxModeSet(GFX_MODE_COLOR, colorGreen);
    //     gfxPrintf(font, timingX, textY, " 0");
    // } else if (spinEndTiming > 0) {
    //     if (spinEndTiming == 1) {
    //         gfxModeSet(GFX_MODE_COLOR, colorYellow);
    //     } else {
    //         gfxModeSet(GFX_MODE_COLOR, colorRed);
    //     }
    //     gfxPrintf(font, timingX, textY, "-%d", spinEndTiming);
    // } else if (spinEndTiming < 0) {
    //     if (spinEndTiming == -1) {
    //         gfxModeSet(GFX_MODE_COLOR, colorYellow);
    //     } else {
    //         gfxModeSet(GFX_MODE_COLOR, colorRed);
    //     }
    //     gfxPrintf(font, timingX, textY, "+%d", abs(spinEndTiming));
    // }

    // jump frames
    if (spinCanceled) {
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(color, alpha));
        gfxPrintf(font, jumpTextX, textY, "-");
    } else {
        if (spinJumpLast == 1) {
            gfxModeSet(GFX_MODE_COLOR, colorGreen);
        } else if (spinJumpLast == 2) {
            gfxModeSet(GFX_MODE_COLOR, colorYellow);
        } else {
            gfxModeSet(GFX_MODE_COLOR, colorRed);
        }
        gfxPrintf(font, jumpTextX, textY, "%d", spinJumpLast);
    }

    // start delay
    if (spinDelayLast == 0) {
        gfxModeSet(GFX_MODE_COLOR, colorGreen);
    } else if (spinDelayLast == 1) {
        gfxModeSet(GFX_MODE_COLOR, colorYellow);
    } else {
        gfxModeSet(GFX_MODE_COLOR, colorRed);
    }
    if (spinDelayLast < 100) {
        gfxPrintf(font, delayTextX, textY, "%d", spinDelayLast);
    } else {
        gfxPrintf(font, delayTextX, textY, "99");
    }
}

static s32 spinDrawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    spinDraw(drawParams->x, drawParams->y, drawParams->font, menuGetCellWidth(item->owner, TRUE),
             menuGetCellHeight(item->owner, TRUE), drawParams->color, drawParams->alpha);
    return 1;
}

static void issDraw(s32 x, s32 y, struct GfxFont *font, s32 chWidth, s32 chHeight, u32 color, u8 alpha) {
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(color, alpha));

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
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(color, alpha));
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
}

static s32 issDrawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    issDraw(drawParams->x, drawParams->y, drawParams->font, menuGetCellWidth(item->owner, TRUE),
            menuGetCellHeight(item->owner, TRUE), drawParams->color, drawParams->alpha);
    return 1;
}

static void aceDraw(s32 x, s32 y, struct GfxFont *font, s32 chWidth, s32 chHeight, u32 color, u8 alpha) {
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(color, alpha));
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
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(color, alpha));
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
}

static s32 aceDrawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    aceDraw(drawParams->x, drawParams->y, drawParams->font, menuGetCellWidth(item->owner, TRUE),
            menuGetCellHeight(item->owner, TRUE), drawParams->color, drawParams->alpha);
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
                if (settings->trainerBowserEnabled) {
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
    if (settings->trainerLzsEnabled) {
        // detect if loading zone is stored
        for (s32 evtIdx = 0; evtIdx < pm_gNumScripts; evtIdx++) {
            if (pm_gScriptIndexList[evtIdx] >= 128) {
                continue;
            }
            pm_Evt *script = (*pm_gCurrentScriptListPtr)[pm_gScriptIndexList[evtIdx]];
            if (script && script->ptrNextLine) {
                u32 callbackFunction = script->ptrNextLine[5];
                if (callbackFunction == (uintptr_t)pm_GotoMap) {
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
                pm_gMapTransitionState == 0) {
                fpLog("control early");
            } else if (pm_gPlayerStatus.prevActionState == ACTION_STATE_JUMP ||
                       pm_gPlayerStatus.actionState == ACTION_STATE_SPIN_JUMP ||
                       pm_gPlayerStatus.actionState == ACTION_STATE_ULTRA_JUMP) {
                fpLog("jump >= 2 frames early");
                if (pm_gGameStatus.pressedButtons[0].analog || lzsPrevPressedAnalog) {
                    fpLog("control early");
                }
            } else if (pm_gPlayerStatus.prevActionState == ACTION_STATE_FALLING) {
                fpLog("jump 1 frame early");
                if (pm_gPlayerStatus.actionState == ACTION_STATE_RUN ||
                    pm_gPlayerStatus.actionState == ACTION_STATE_WALK) {
                    fpLog("control early");
                }
            } else if (lzsPrevPrevActionState == ACTION_STATE_FALLING && pm_gMapTransitionState == 0) {
                fpLog("jump 1 frame late");
                fpLog("control early");
            } else if (lzsFramesSinceLand == 3) {
                fpLog("jump 1 frame late");
                if (pm_gGameStatus.pressedButtons[0].analog) {
                    fpLog("control late");
                }
            } else if (lzsFramesSinceLand == 4) {
                fpLog("jump 2 frames late");
                if (pm_gGameStatus.pressedButtons[0].analog || lzsPrevPressedAnalog) {
                    fpLog("control late");
                }
            } else if (lzsFramesSinceLand == 0 &&
                       (lzsPrevPrevActionState == ACTION_STATE_RUN || lzsPrevPrevActionState == ACTION_STATE_WALK)) {
                fpLog("jump >= 2 frames late");
                fpLog("control early");
            } else if (lzsFramesSinceLand >= 5 && pm_gMapTransitionState == 0) {
                fpLog("jump > 2 frames late");
                if (pm_gGameStatus.pressedButtons[0].analog || lzsPrevPressedAnalog) {
                    fpLog("control late");
                }
            } else if (lzsFramesSinceLand == 2) {
                lzsCurrentJumps++;
            }
        }

        if (lzsCurrentJumps > lzsRecordJumps) {
            lzsRecordJumps = lzsCurrentJumps;
        }

        lzsPrevPressedAnalog = pm_gGameStatus.pressedButtons[0].analog;
        lzsPrevPrevActionState = pm_gPlayerStatus.prevActionState;

        if (pm_gMapTransitionState == 1) {
            lzsLzStored = FALSE;
            lzsPlayerLanded = FALSE;
            lzsFramesSinceLand = 0;
            lzsCurrentJumps = 0;
        }
    }
}

static void blockCheckSuccessOrEarly(void) {
    const u32 mashWindow = 10;
    u32 blockWindow = 3;
    if (!(pm_gBattleStatus.flags1 & 0x80000) && pm_is_ability_active(0)) { // Dodge Master
        blockWindow = 5;
    }
    s32 window;
    s32 bufferPos = acInputBufferPos - blockWindow;
    if (pm_gBattleStatus.blockResult == BLOCK_EARLY) {
        bufferPos -= mashWindow;
        window = mashWindow;
    } else { // success
        window = blockWindow;
    }

    if (bufferPos < 0) {
        bufferPos += ARRAY_LENGTH(acPushInputBuffer);
    }
    for (s32 i = 0; i < window; i++) {
        if (bufferPos >= ARRAY_LENGTH(acPushInputBuffer)) {
            bufferPos -= ARRAY_LENGTH(acPushInputBuffer);
        }
        if (acPushInputBuffer[bufferPos] & 0x8000) { // A button
            if (pm_gBattleStatus.blockResult == -1) {
                s32 framesEarly = window - i;
                fpLog("blocked %d frame%s early", framesEarly, framesEarly > 1 ? "s" : "");
            } else {
                fpLog("blocked frame %d out of %d", i + 1, window);
            }
            break;
        }
        bufferPos++;
    }
}

static void updateBlockTrainer(void) {
    if (settings->trainerAcEnabled && pm_gGameStatus.isBattle) {
        // blocks
        switch (pm_gBattleStatus.blockResult) {
            case BLOCK_EARLY:
            case BLOCK_SUCCESS:
                acWaitingForMissedBlock = FALSE;
                blockCheckSuccessOrEarly();
                // this value doesn't appear to be checked beyond the first frame, so we can change it with no issue
                pm_gBattleStatus.blockResult = BLOCK_NONE;
                break;
            case BLOCK_LATE:
                acWaitingForMissedBlock = TRUE;
                acBlockFramesLate = 0;
                pm_gBattleStatus.blockResult = BLOCK_NONE;
                break;
        }
        if (acWaitingForMissedBlock) {
            acBlockFramesLate++;
            if (pm_gGameStatus.currentButtons[0].buttons & 0x8000) { // A button
                fpLog("blocked %d frame%s late", acBlockFramesLate, acBlockFramesLate > 1 ? "s" : "");
                acWaitingForMissedBlock = FALSE;
            } else if (acBlockFramesLate == 10) { // stop checking for late block after 10 frames
                acWaitingForMissedBlock = FALSE;
            }
        }

        acPushInputBuffer[acInputBufferPos++] =
            pm_gGameStatus.currentButtons[0].buttons & pm_gBattleStatus.inputBitmask;
        if (acInputBufferPos >= ARRAY_LENGTH(acPushInputBuffer)) {
            acInputBufferPos = 0;
        }

        // Either goombario or mario attacking
        if ((pm_gBattleSubState == 3 && pm_gPlayerData.currentPartner == PARTNER_GOOMBARIO) ||
            pm_gBattleSubState == 4) {
            if (pm_gActionCommandStatus.state == 10 && pm_gGameStatus.pressedButtons[0].a) {
                acLastAPress = pm_gGameStatus.frameCounter;
            } else if (pm_gActionCommandStatus.state == 11) {
                if (acLastAPress) {
                    u16 framesEarly = pm_gGameStatus.frameCounter - acLastAPress;
                    fpLog("pressed A %d frame%s early", framesEarly, framesEarly > 1 ? "s" : "");
                    acLastAPress = 0;
                }
                if (pm_gGameStatus.pressedButtons[0].a) {
                    fpLog("pressed A frame %d out of %d",
                          pm_gBattleStatus.unk_434[pm_gActionCommandStatus.unk_50] - pm_gActionCommandStatus.unk_54,
                          pm_gBattleStatus.unk_434[pm_gActionCommandStatus.unk_50]);
                }
                acLastValidFrame = pm_gGameStatus.frameCounter;
                // check for a press up to 10 frames late
            } else if (pm_gActionCommandStatus.state == 12 && pm_gGameStatus.pressedButtons[0].a &&
                       pm_gGameStatus.frameCounter - acLastValidFrame <= 10) {
                u16 framesLate = pm_gGameStatus.frameCounter - acLastValidFrame;
                fpLog("pressed A %d frame%s late", framesLate, framesLate > 1 ? "s" : "");
            }
        }
    }
}

static void updateClippyTrainer(void) {
    if (settings->trainerClippyEnabled) {
        if (pm_gGameStatus.pressedButtons[0].cRight && pm_gCurrentEncounter.firstStrikeType != 2) {
            if (pm_gEncounterState == 2 && pm_gPartnerStatus.partnerActionState == 1) {
                clippyStatus = CLIPPY_EARLY;
            } else if (clippyFramesSinceBattle > 0) {
                clippyStatus = CLIPPY_LATE;
            } else if (pm_gEncounterState == 3 && clippyFramesSinceBattle == 0) {
                clippyStatus = CLIPPY_SUCCESS;
            }
        }

        if (pm_gEncounterState == 3) {
            clippyFramesSinceBattle++;
            switch (clippyStatus) {
                case CLIPPY_EARLY: fpLog("early"); break;
                case CLIPPY_LATE: fpLog("late"); break;
            }
            clippyStatus = CLIPPY_NONE;
        } else if (pm_gEncounterState != 3) {
            clippyFramesSinceBattle = 0;
        }
    }
}

static void updateQuickJumpTrainer(void) {
    static s8 jumpFrame;
    static u8 crouchFrame;
    static u8 frame;
    static bool waitForNextTurn;
    static u8 frameWindow;

    if (settings->trainerQuickJumpsEnabled && pm_gGameStatus.isBattle && pm_gBattleStatus.playerActor) {
        if (pm_gBattleStatus.playerActor->partsTable->curAnimation == 0x10004) {
            waitForNextTurn = FALSE;
            jumpFrame = -1;
            crouchFrame = 0;
            frame = 0;
            frameWindow = 0;
            return;
        }

        if (waitForNextTurn) {
            return;
        }

        if (pm_gBattleStatus.playerActor->partsTable->curAnimation == 0x10005 && frame <= 13) {
            if (frameWindow == 0) {
                if (pm_gBattleStatus.playerActor->state.moveTime < 13) {
                    frameWindow = 5;
                } else {
                    frameWindow = pm_gBattleStatus.playerActor->state.moveTime - 9;
                }
            }
            if (inputPressed().a && jumpFrame == -1) {
                jumpFrame = frame;
            }
            frame++;
        } else if (pm_gBattleStatus.playerActor->partsTable->curAnimation == 0x10008) {
            waitForNextTurn = TRUE;
        } else if (frame > 0) {
            if (jumpFrame != -1) {
                u8 framesSinceJump = frame - jumpFrame;
                if (framesSinceJump == 1) {
                    fpLog("jumped frame 1 out of %d", frameWindow);
                } else {
                    fpLog("jumped %d frame%s early", framesSinceJump - 1, framesSinceJump - 1 == 1 ? "" : "s");
                }
                waitForNextTurn = TRUE;
                return;
            }

            if (crouchFrame == 0) {
                crouchFrame = frame;
            }

            if (inputPressed().a) {
                u8 framesAfterCrouch = frame - crouchFrame + 2;
                if (framesAfterCrouch <= frameWindow) {
                    fpLog("jumped frame %d out of %d", framesAfterCrouch, frameWindow);
                } else {
                    fpLog("jumped %d frame%s late", framesAfterCrouch - frameWindow,
                          framesAfterCrouch - frameWindow == 1 ? "" : "s");
                }
                waitForNextTurn = TRUE;
            }

            frame++;
        }
    }
}

static void updateHammerCancelTrainer(void) {
    static bool canCancel;

    if (settings->trainerHammerCancelsEnabled) {
        if (pm_HammerHit.timer == 3) {
            canCancel = TRUE;
        }

        if (pm_HammerHit.timer > 10) {
            canCancel = FALSE;
        }

        if (inputPressed().b && canCancel) {
            canCancel = FALSE;

            if (pm_gPlayerStatus.flags & PS_FLAG_ENTERING_BATTLE) {
                return;
            }

            if (pm_HammerHit.timer < 7) {
                fpLog("hammered %d frame%s early", 7 - pm_HammerHit.timer, 7 - pm_HammerHit.timer == 1 ? "" : "s");
            } else if (pm_HammerHit.timer > 7) {
                fpLog("hammered %d frame%s late", pm_HammerHit.timer - 7, pm_HammerHit.timer - 7 == 1 ? "" : "s");
            } else {
                fpLog("hammered correct frame");
            }
        }
    }
}

void trainerUpdate(void) {
    updateBowserBlockTrainer();
    updateLzsTrainer();
    updateBlockTrainer();
    updateClippyTrainer();
    updateQuickJumpTrainer();
    updateHammerCancelTrainer();
}

void trainerDrawPinned(s32 x, s32 y, struct GfxFont *font, s32 chWidth, s32 chHeight, u32 color, u8 alpha) {
    switch (settings->pinnedTrainer) {
        case TRAINER_LZS: lzsDraw(x, y, font, chWidth, chHeight, color, alpha); break;
        case TRAINER_SPIN: spinDraw(SCREEN_WIDTH / 2, SCREEN_HEIGHT - 65, font, chWidth, chHeight, color, alpha); break;
        case TRAINER_ISS: issDraw(x, y, font, chWidth, chHeight, color, alpha); break;
        case TRAINER_ACE: aceDraw(x, y, font, chWidth, chHeight, color, alpha); break;
    }
}

void createTrainerMenu(struct Menu *menu) {
    static struct Menu lzsMenu;
    static struct Menu spinMenu;
    static struct Menu issMenu;
    static struct Menu aceMenu;

    /* initialize menu */
    menuInit(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&lzsMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&spinMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
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
    struct MenuItem *firstOption =
        menuAddCheckbox(menu, xOffset, y, menuByteCheckboxProc, &settings->trainerBowserEnabled);
    menuAddOption(menu, xOffset + 2, y++,
                  "fire\0"
                  "butt stomp\0"
                  "claw\0"
                  "wave\0"
                  "lightning\0",
                  menuByteOptionmodProc, &bowserAttack);

    menuAddStatic(menu, 0, y, "lzs jumps", 0xC0C0C0);
    menuAddCheckbox(menu, xOffset, y, menuByteCheckboxProc, &settings->trainerLzsEnabled);
    menuAddSubmenuIcon(menu, xOffset + 2, y++, &lzsMenu, wrench, 0, 0, 1.0f);

    menuAddStatic(menu, 0, y, "action commands", 0xC0C0C0);
    menuAddCheckbox(menu, xOffset, y++, menuByteCheckboxProc, &settings->trainerAcEnabled);

    menuAddStatic(menu, 0, y, "quick jumps", 0xC0C0C0);
    menuAddCheckbox(menu, xOffset, y++, menuByteCheckboxProc, &settings->trainerQuickJumpsEnabled);

    menuAddStatic(menu, 0, y, "hammer cancels", 0xC0C0C0);
    menuAddCheckbox(menu, xOffset, y++, menuByteCheckboxProc, &settings->trainerHammerCancelsEnabled);

    menuAddStatic(menu, 0, y, "clippy", 0xC0C0C0);
    menuAddCheckbox(menu, xOffset, y++, menuByteCheckboxProc, &settings->trainerClippyEnabled);

    menuAddStatic(menu, 0, y, "spin", 0xC0C0C0);
    struct MenuItem *lastOption = menuAddSubmenuIcon(menu, xOffset, y++, &spinMenu, wrench, 0, 0, 1.0f);
#if PM64_VERSION == JP
    menuAddStatic(menu, 0, y, "ice staircase skip", 0xC0C0C0);
    menuAddSubmenuIcon(menu, xOffset, y++, &issMenu, wrench, 0, 0, 1.0f);

    menuAddStatic(menu, 0, y, "oot ace", 0xC0C0C0);
    lastOption = menuAddSubmenuIcon(menu, xOffset, y++, &aceMenu, wrench, 0, 0, 1.0f);
#endif
    y++;
    struct MenuItem *unpinButton = menuAddButton(menu, 0, y++, "unpin trainer", unpinProc, NULL);
    menuAddButton(menu, 0, y++, "save settings", fpSaveSettingsProc, NULL);

    menuItemAddChainLink(menu->selector, firstOption, MENU_NAVIGATE_DOWN);
    menuItemAddChainLink(unpinButton, lastOption, MENU_NAVIGATE_UP);

    /*build lzs jump menu*/
    y = 0;
    lzsMenu.selector = menuAddSubmenu(&lzsMenu, 0, y++, NULL, "return");
    menuAddStatic(&lzsMenu, 0, y, "pin", 0xC0C0C0);
    menuAddCheckbox(&lzsMenu, 4, y++, pinnedTrainerProc, (void *)TRAINER_LZS);
    y++;
    menuAddStaticCustom(&lzsMenu, 0, y++, lzsDrawProc, NULL, 0xC0C0C0);

    /*build spin menu*/
    y = 0;
    spinMenu.selector = menuAddSubmenu(&spinMenu, 0, y++, NULL, "return");
    menuAddStatic(&spinMenu, 0, y, "pin", 0xC0C0C0);
    menuAddCheckbox(&spinMenu, 4, y++, pinnedTrainerProc, (void *)TRAINER_SPIN);
    y++;
    menuAddStaticCustom(&spinMenu, 0, y++, spinDrawProc, NULL, 0xC0C0C0);
    spinJumpTex = resourceLoadPmiconGlobal(ICON_MENU_BOOTS_1, 1);
    spinDelayTex = resourceGet(RES_PMICON_CLOCK);

    /*build iss menu*/
    y = 0;
    issMenu.selector = menuAddSubmenu(&issMenu, 0, y++, NULL, "return");
    menuAddStatic(&issMenu, 0, y, "pin", 0xC0C0C0);
    menuAddCheckbox(&issMenu, 4, y++, pinnedTrainerProc, (void *)TRAINER_ISS);
    y++;
    menuAddStaticCustom(&issMenu, 0, y++, issDrawProc, NULL, 0xC0C0C0);

    /*build ace menu*/
    y = 0;
    aceMenu.selector = menuAddSubmenu(&aceMenu, 0, y++, NULL, "return");
    menuAddStatic(&aceMenu, 0, y, "pin", 0xC0C0C0);
    struct MenuItem *pinCheckbox = menuAddCheckbox(&aceMenu, 4, y++, pinnedTrainerProc, (void *)TRAINER_ACE);
    y++;
    menuAddStaticCustom(&aceMenu, 0, y, aceDrawProc, NULL, 0xC0C0C0);
    y += 4;
    struct MenuItem *payloadOption = menuAddButton(&aceMenu, 0, y++, "practice payload", acePracticePayloadProc, NULL);
    menuAddButton(&aceMenu, 0, y++, "oot instruction", aceOotInstrProc, NULL);

    menuItemAddChainLink(aceMenu.selector, pinCheckbox, MENU_NAVIGATE_DOWN);
    menuItemAddChainLink(payloadOption, pinCheckbox, MENU_NAVIGATE_UP);
}
