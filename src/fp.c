#include "fp.h"
#include "commands.h"
#include "common.h"
#include "crash_screen.h"
#include "geometry.h"
#include "input.h"
#include "io.h"
#include "resource.h"
#include "timer.h"
#include "watchlist.h"
#include <n64.h>
#include <startup.h>
#include <stdlib.h>
#include <string.h>

__attribute__((section(".data"))) FpCtxt fp = {
    .ready = FALSE,
};

// Initializes and uses new stack instead of using games main thread stack.
static void initStack(void (*func)(void)) {
    static _Alignas(8) __attribute__((section(".stack"))) char stack[0x2000];
    __asm__ volatile("la     $t0, %1;"
                     "sw     $sp, -0x04($t0);"
                     "sw     $ra, -0x08($t0);"
                     "addiu  $sp, $t0, -0x08;"
                     "jalr   %0;"
                     "nop;"
                     "lw     $ra, 0($sp);"
                     "lw     $sp, 4($sp);" ::"r"(func),
                     "i"(&stack[sizeof(stack)]));
}

static void mainReturnProc(struct MenuItem *item, void *data) {
    hideMenu();
}

void fpInit(void) {
    clear_bss();
    do_global_ctors();

    gfxStart();
    gfxModeConfigure(GFX_MODE_FILTER, G_TF_POINT);
    gfxModeConfigure(GFX_MODE_COMBINE, G_CC_MODE(G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM));

    fp.profile = 0;
    fp.settingsLoaded = FALSE;
    fp.versionShown = FALSE;
    fp.cpuCounter = 0;
    fp.cpuCounterFreq = 0;
    fp.menuActive = FALSE;

    for (s32 i = 0; i < SETTINGS_LOG_MAX; i++) {
        fp.log[i].msg = NULL;
    }

    fp.savedPos.x = 0;
    fp.savedPos.y = 0;
    fp.savedPos.z = 0;
    fp.savedFacingAngle = 0;
    fp.savedArea = 0x1c;
    fp.savedMap = 0;
    fp.savedEntrance = 0;
    fp.turbo = FALSE;
    fp.aceLastTimer = 0;
    fp.aceLastFlagStatus = FALSE;
    fp.aceLastJumpStatus = FALSE;
    fp.bowserBlocksEnabled = FALSE;
    fp.bowserBlock = 0;
    fp.lzsTrainerEnabled = FALSE;
    fp.prevPrevActionState = 0;
    fp.lzStored = FALSE;
    fp.recordLzsJumps = 0;
    fp.currentLzsJumps = 0;
    fp.playerLanded = FALSE;
    fp.framesSinceLand = 0;
    fp.warp = FALSE;
    fp.warpDelay = 0;
    fp.framesSinceBattle = 0;
    fp.clippyStatus = 0;
    fp.clippyTrainerEnabled = FALSE;
    fp.lastImportedSavePath = NULL;
    fp.freeCam = FALSE;
    fp.lockCam = FALSE;
    fp.camBhv = CAMBHV_MANUAL;
    fp.camDistMin = 100;
    fp.camDistMax = 400;
    fp.camYaw = 0;
    fp.camPitch = 0;
    fp.camPos.x = 0;
    fp.camPos.y = 0;
    fp.camPos.z = 0;
    fp.camEnabledBefore = FALSE;
    fp.actionCommandTrainerEnabled = FALSE;

    ioInit();

    settingsLoadDefault();

    // init menus
    static struct Menu mainMenu;
    static struct Menu watches;
    static struct Menu global;

    menuInit(&mainMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&watches, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&global, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    fp.mainMenu = &mainMenu;
    fp.global = &global;
    fp.menuWatches = &watches;

    // populate top level menus
    s32 menuIndex = 0;
    mainMenu.selector = menuAddButton(fp.mainMenu, 0, menuIndex++, "return", mainReturnProc, NULL);
    menuAddSubmenu(fp.mainMenu, 0, menuIndex++, createWarpsMenu(), "warps");
    menuAddSubmenu(fp.mainMenu, 0, menuIndex++, createCheatsMenu(), "cheats");
    menuAddSubmenu(fp.mainMenu, 0, menuIndex++, createPlayerMenu(), "player");
    menuAddSubmenu(fp.mainMenu, 0, menuIndex++, createFileMenu(), "file");
    menuAddSubmenu(fp.mainMenu, 0, menuIndex++, createPracticeMenu(), "practice");
    menuAddSubmenu(fp.mainMenu, 0, menuIndex++, createCameraMenu(), "camera");
    menuAddSubmenu(fp.mainMenu, 0, menuIndex++, &watches, "watches");
    menuAddSubmenu(fp.mainMenu, 0, menuIndex++, createDebugMenu(), "debug");
    menuAddSubmenu(fp.mainMenu, 0, menuIndex++, createSettingsMenu(), "settings");

    // populate watches menu
    watches.selector = menuAddSubmenu(&watches, 0, 0, NULL, "return");
    fp.menuWatchlist = watchlistCreate(&watches, &global, 0, 1);

    // configure menu related commands
    inputBindSetOverride(COMMAND_MENU, TRUE);
    inputBindSetOverride(COMMAND_RETURN, TRUE);

    // get menu appearance
    applyMenuSettings();

    // skip intro on boot
    pm_gGameStatus.bSkipIntro = 1;

    // calculate frame window for OoT ACE
    *(u16 *)0x807D0000 = 0;
    s32 memoryValue = 0;
    s32 *pointer = (s32 *)0x807BFFF8;

    while (memoryValue == 0) {
        pointer--;
        memoryValue = *pointer;
    }

    s32 frameWindow = (s32)pointer;
    frameWindow -= 0x80400000;

    if (frameWindow % 0x4000 == 0) {
        frameWindow /= 0x40000;
        frameWindow -= 0x10;
    } else {
        frameWindow /= 0x40000;
        frameWindow -= 0xf;
    }

    frameWindow *= -1;
    fp.aceFrameWindow = frameWindow;

#if PM64_VERSION == JP
    crashScreenInit();
#endif

    fp.ready = TRUE;
}

void fpUpdateMenu(void) {
    if (inputBindPressedRaw(COMMAND_MENU)) {
        hideMenu();
    } else if (inputBindPressed(COMMAND_RETURN)) {
        menuReturn(fp.mainMenu);
    } else {
        u16 padPressed = inputPressed();
        if (padPressed & BUTTON_D_UP) {
            menuNavigate(fp.mainMenu, MENU_NAVIGATE_UP);
        } else if (padPressed & BUTTON_D_DOWN) {
            menuNavigate(fp.mainMenu, MENU_NAVIGATE_DOWN);
        } else if (padPressed & BUTTON_D_LEFT) {
            menuNavigate(fp.mainMenu, MENU_NAVIGATE_LEFT);
        } else if (padPressed & BUTTON_D_RIGHT) {
            menuNavigate(fp.mainMenu, MENU_NAVIGATE_RIGHT);
        } else if (padPressed & BUTTON_L) {
            menuActivate(fp.mainMenu);
        }
    }
}

static void fpUpdateCpuCounter(void) {
    static u32 count = 0;
    u32 newCount;
    __asm__("mfc0    %0, $9;" : "=r"(newCount));
    fp.cpuCounterFreq = OS_CPU_COUNTER;
    fp.cpuCounter += newCount - count;
    count = newCount;
}

void fpEmergencySettingsReset(u16 padPressed) {
    if (padPressed) {
        static const u16 inputList[] = {
            BUTTON_D_UP,    BUTTON_D_UP,   BUTTON_D_DOWN,  BUTTON_D_DOWN, BUTTON_D_LEFT,
            BUTTON_D_RIGHT, BUTTON_D_LEFT, BUTTON_D_RIGHT, BUTTON_B,      BUTTON_A,
        };
        static s32 inputPos = 0;
        size_t inputListLength = sizeof(inputList) / sizeof(*inputList);
        if (padPressed == inputList[inputPos]) {
            ++inputPos;
            if (inputPos == inputListLength) {
                inputPos = 0;
                settingsLoadDefault();
                applyMenuSettings();
                fpLog("default settings restored");
            }
        } else {
            inputPos = 0;
        }
    }
}

#define STRINGIFY(S)  STRINGIFY_(S)
#define STRINGIFY_(S) #S
void fpDrawVersion(struct GfxFont *font, s32 cellWidth, s32 cellHeight, u8 menuAlpha) {
    static struct GfxTexture *fpIconTex;
    if (pm_gGameStatus.introState == 5) {
        fp.versionShown = TRUE;
    } else {
        if (fpIconTex == NULL) {
            fpIconTex = resourceLoadPmiconItem(ITEM_FP_PLUS_A, FALSE);
        }
        struct GfxSprite fpIconSprite = {
            fpIconTex, 0, 0, 15, SCREEN_HEIGHT - 65, 1.f, 1.f,
        };
        gfxModeReplace(GFX_MODE_DROPSHADOW, 0);
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0xFF, 0xFF, 0xFF));
        gfxSpriteDraw(&fpIconSprite);
        gfxModePop(GFX_MODE_DROPSHADOW);

        gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0, 0, menuAlpha));
        gfxPrintf(font, 16, SCREEN_HEIGHT - 35 + cellHeight * 1, STRINGIFY(FP_VERSION));
        gfxPrintf(font, SCREEN_WIDTH - cellWidth * 21, SCREEN_HEIGHT - 35 + cellHeight * 1, STRINGIFY(URL));
    }
}

void fpDrawInputDisplay(struct GfxFont *font, s32 cellWidth, s32 cellHeight, u8 menuAlpha) {
    u16 dPad = pm_gGameStatus.currentButtons[0].buttons;
    s8 dX = pm_gGameStatus.stickX[0];
    s8 dY = pm_gGameStatus.stickY[0];

    struct GfxTexture *texture = resourceGet(RES_ICON_BUTTONS);
    struct GfxTexture *controlStick = resourceGet(RES_TEXTURE_CONTROL_STICK);
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0xC0, 0xC0, 0xC0, menuAlpha));
    if (settings->controlStick == 1 || settings->controlStick == 2) {
        s32 imageRange = controlStick->tileWidth / 2;
        s32 imageDx, imageDy;
        if (dX > settings->controlStickRange) {
            imageDx = imageRange;
        } else if (dX < -settings->controlStickRange) {
            imageDx = -imageRange;
        } else {
            imageDx = dX * imageRange / settings->controlStickRange;
        }
        if (dY > settings->controlStickRange) {
            imageDy = -imageRange;
        } else if (dY < -settings->controlStickRange) {
            imageDy = imageRange;
        } else {
            imageDy = -dY * imageRange / settings->controlStickRange;
        }
        struct GfxSprite inBackground = {
            controlStick, 0, 0, settings->inputDisplayX, settings->inputDisplayY - controlStick->tileHeight, 1.f, 1.f,
        };
        struct GfxSprite inStick = {
            controlStick,
            1,
            0,
            settings->inputDisplayX + imageDx,
            settings->inputDisplayY - controlStick->tileHeight + imageDy,
            1.f,
            1.f,
        };
        gfxSpriteDraw(&inBackground);
        gfxSpriteDraw(&inStick);
    }
    if (settings->controlStick == 0) {
        gfxPrintf(font, settings->inputDisplayX, settings->inputDisplayY, "%4i %4i", dX, dY);
    } else if (settings->controlStick == 2) {
        gfxPrintf(font, settings->inputDisplayX + controlStick->tileWidth + cellWidth,
                  settings->inputDisplayY - cellHeight * 2, "%4i %4i", dX, dY);
    }

    static const s32 buttons[] = {
        15, 14, 12, 3, 2, 1, 0, 13, 5, 4, 11, 10, 9, 8,
    };

    for (s32 i = 0; i < sizeof(buttons) / sizeof(*buttons); ++i) {
        s32 b = buttons[i];
        if (!(dPad & (1 << b))) {
            continue;
        }
        s32 x = (cellWidth - texture->tileWidth) / 2 + i * 10;
        s32 y = -(gfxFontXheight(font) + texture->tileHeight + 1) / 2;
        s32 buttonDx;
        if (settings->controlStick == 0) {
            buttonDx = cellWidth * 10;
        } else {
            buttonDx = controlStick->tileWidth + cellWidth;
        }
        struct GfxSprite sprite = {
            texture, b, 0, settings->inputDisplayX + buttonDx + x, settings->inputDisplayY + y, 1.f, 1.f,
        };
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(inputButtonColor[b], menuAlpha));
        gfxSpriteDraw(&sprite);
    }
}

void fpDrawTimer(struct GfxFont *font, s32 cellWidth, s32 cellHeight, u8 menuAlpha) {
    s32 hundredths = timerGetTimerCount() * 100 / fp.cpuCounterFreq;
    s32 seconds = hundredths / 100;
    s32 minutes = seconds / 60;
    s32 hours = minutes / 60;

    hundredths %= 100;
    seconds %= 60;
    minutes %= 60;

    s32 x = settings->timerX;
    s32 y = settings->timerY;
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0xC0, 0xC0, 0xC0, menuAlpha));

    if (hours > 0) {
        gfxPrintf(font, x, y, "%d:%02d:%02d.%02d", hours, minutes, seconds, hundredths);
    } else if (minutes > 0) {
        gfxPrintf(font, x, y, "%d:%02d.%02d", minutes, seconds, hundredths);
    } else {
        gfxPrintf(font, x, y, "%d.%02d", seconds, hundredths);
    }

    gfxPrintf(font, x, y + cellHeight, "%d", timerGetLagFrames());
}

// this whole thing should be redone once battles are better understood - freezing rng isn't very reliable
void fpBowserBlockTrainer(void) {
    if (pm_gGameStatus.isBattle && pm_gGameStatus.areaID == 0x4 &&
        (pm_gGameStatus.mapID == 0x7 || pm_gGameStatus.mapID == 0x13) && STORY_PROGRESS != STORY_INTRO &&
        !(pm_gGameStatus.peachFlags & (1 << 0))) {

        pm_Actor *bowser = pm_gBattleStatus.enemyActors[0];

        if (bowser != NULL) {
            s32 *turn = &bowser->state.varTable[0];
            s32 *turnsSinceWave = &bowser->state.varTable[2];
            s32 *turnsSinceBeam = &bowser->state.varTable[3];
            s32 *turnsSinceClaw = &bowser->state.varTable[4];
            s32 *turnsSinceStomp = &bowser->state.varTable[5];
            s32 *turnsSinceHeal = &bowser->state.varTable[6];
            *turnsSinceHeal = 0;
            *turnsSinceBeam = 0;
            switch (fp.bowserBlock) {
                case 0: // fire
                    *turn = 3;
                    *turnsSinceClaw = 0;
                    *turnsSinceStomp = 0;
                    *turnsSinceWave = 0;
                    break;
                case 1: // butt stomp
                    *turn = 3;
                    *turnsSinceClaw = 0;
                    *turnsSinceStomp = 1;
                    *turnsSinceWave = 0;
                    pm_randSeed = 0x03D49DFF;
                    break;
                case 2: // claw
                    *turn = 3;
                    *turnsSinceStomp = 0;
                    *turnsSinceClaw = 1;
                    *turnsSinceWave = 0;
                    pm_randSeed = 0x9CB89EDA;
                    break;
                case 3: // wave
                    *turn = 4;
                    *turnsSinceWave = 6;
                    pm_randSeed = 0x77090261;
                    break;
                case 4: // lightning, still gives wave for hallway bowser
                    *turn = 4;
                    *turnsSinceWave = 6;
                    pm_randSeed = 0x72A5DCE5;
                    break;
            }
        }

        if (pm_gBattleStatus.partnerActor != NULL) {
            // if partner is KO'd by wave, never let it last more than one turn so you can keep practicing the block
            if (pm_gBattleStatus.partnerActor->koDuration > 1) {
                pm_gBattleStatus.partnerActor->koDuration = 1;
            }
        }
    }
}

void fpLzsTrainer(void) {
    // detect if loading zone is stored
    for (s32 evtIdx = 0; evtIdx < pm_gNumScripts; evtIdx++) {
        pm_Evt *script = (*pm_gCurrentScriptListPtr)[pm_gScriptIndexList[evtIdx]];
        if (script && script->ptrNextLine) {
            u32 callbackFunction = script->ptrNextLine[5];
            if (callbackFunction == (uintptr_t)pm_gotoMap) {
                fp.lzStored = TRUE;
            }
        }
    }

    // Count frames since mario landed
    if (pm_gPlayerStatus.actionState == ACTION_STATE_LAND || pm_gPlayerStatus.actionState == ACTION_STATE_WALK ||
        pm_gPlayerStatus.actionState == ACTION_STATE_RUN) {
        fp.playerLanded = TRUE;
    }
    if (fp.playerLanded) {
        fp.framesSinceLand++;
    } else {
        fp.framesSinceLand = 0;
    }
    if (pm_gPlayerStatus.actionState == ACTION_STATE_JUMP) {
        fp.playerLanded = FALSE;
    }

    // log lzs status
    if (fp.lzStored && pm_gGameStatus.pressedButtons[0].a) {
        if (fp.prevPrevActionState == ACTION_STATE_FALLING && pm_gPlayerStatus.actionState == ACTION_STATE_JUMP &&
            pm_mapChangeState == 0) {
            fpLog("control early");
        } else if (pm_gPlayerStatus.prevActionState == ACTION_STATE_JUMP ||
                   pm_gPlayerStatus.actionState == ACTION_STATE_SPIN_JUMP ||
                   pm_gPlayerStatus.actionState == ACTION_STATE_ULTRA_JUMP) {
            fpLog("jump >= 2 frames early");
            if (pm_gGameStatus.pressedButtons[0].yCardinal || fp.prevPressedY) {
                fpLog("control early");
            }
        } else if (pm_gPlayerStatus.prevActionState == ACTION_STATE_FALLING) {
            fpLog("jump 1 frame early");
            if (pm_gPlayerStatus.actionState == ACTION_STATE_RUN || pm_gPlayerStatus.actionState == ACTION_STATE_WALK) {
                fpLog("control early");
            }
        } else if (fp.prevPrevActionState == ACTION_STATE_FALLING && pm_mapChangeState == 0) {
            fpLog("jump 1 frame late");
            fpLog("control early");
        } else if (fp.framesSinceLand == 3) {
            fpLog("jump 1 frame late");
            if (pm_gGameStatus.pressedButtons[0].yCardinal) {
                fpLog("control late");
            }
        } else if (fp.framesSinceLand == 4) {
            fpLog("jump 2 frames late");
            if (pm_gGameStatus.pressedButtons[0].yCardinal || fp.prevPressedY) {
                fpLog("control late");
            }
        } else if (fp.framesSinceLand == 0 &&
                   (fp.prevPrevActionState == ACTION_STATE_RUN || fp.prevPrevActionState == ACTION_STATE_WALK)) {
            fpLog("jump >= 2 frames late");
            fpLog("control early");
        } else if (fp.framesSinceLand >= 5 && pm_mapChangeState == 0) {
            fpLog("jump > 2 frames late");
            if (pm_gGameStatus.pressedButtons[0].yCardinal || fp.prevPressedY) {
                fpLog("control late");
            }
        } else if (fp.framesSinceLand == 2) {
            fp.currentLzsJumps++;
        }
    }

    if (fp.currentLzsJumps > fp.recordLzsJumps) {
        fp.recordLzsJumps = fp.currentLzsJumps;
    }

    fp.prevPressedY = pm_gGameStatus.pressedButtons[0].yCardinal;
    fp.prevPrevActionState = pm_gPlayerStatus.prevActionState;

    if (pm_mapChangeState == 1) {
        fp.lzStored = FALSE;
        fp.playerLanded = FALSE;
        fp.framesSinceLand = 0;
        fp.currentLzsJumps = 0;
    }
}

void fpClippyTrainer(void) {
    if (pm_gGameStatus.pressedButtons[0].cr && pm_gCurrentEncounter.eFirstStrike != 2) {
        if (pm_gameState == 2 && pm_gPartnerActionStatus.partnerActionState == 1) {
            fp.clippyStatus = 1;
        } else if (fp.framesSinceBattle > 0) {
            fp.clippyStatus = 3;
        } else if (pm_gameState == 3 && fp.framesSinceBattle == 0) {
            fp.clippyStatus = 2;
        }
    }

    if (pm_gameState == 3) {
        fp.framesSinceBattle++;
        switch (fp.clippyStatus) {
            case 1: fpLog("early"); break;
            case 2: break; // Got clippy
            case 3: fpLog("late"); break;
        }
        fp.clippyStatus = 0;
    } else if (pm_gameState != 3) {
        fp.framesSinceBattle = 0;
    }
}

void fpActionCommandTrainer(void) {
    // Either goombario or mario attacking
    if ((pm_battleState2 == 3 && pm_gPlayerStatus.playerData.currentPartner == PARTNER_GOOMBARIO) ||
        pm_battleState2 == 4) {
        if (pm_gActionCommandStatus.state == 10 && pm_gGameStatus.pressedButtons[0].a) {
            fp.lastAPress = pm_gGameStatus.frameCounter;
        } else if (pm_gActionCommandStatus.state == 11) {
            if (fp.lastAPress) {
                u16 framesEarly = pm_gGameStatus.frameCounter - fp.lastAPress;
                fpLog("pressed A %d frame%s early", framesEarly, framesEarly > 1 ? "s" : "");
                fp.lastAPress = 0;
            }
            if (pm_gGameStatus.pressedButtons[0].a) {
                fpLog("pressed A frame %d out of %d",
                      pm_gBattleStatus.unk_434[pm_gActionCommandStatus.unk_50] - pm_gActionCommandStatus.unk_54,
                      pm_gBattleStatus.unk_434[pm_gActionCommandStatus.unk_50]);
            }
            fp.lastValidFrame = pm_gGameStatus.frameCounter;
            // check for a press up to 10 frames late
        } else if (pm_gActionCommandStatus.state == 12 && pm_gGameStatus.pressedButtons[0].a &&
                   pm_gGameStatus.frameCounter - fp.lastValidFrame <= 10) {
            u16 framesLate = pm_gGameStatus.frameCounter - fp.lastValidFrame;
            fpLog("pressed A %d frame%s late", framesLate, framesLate > 1 ? "s" : "");
        }
    }
}

void fpUpdateCheats(void) {
    if (CHEAT_ACTIVE(CHEAT_HP)) {
        pm_gPlayerStatus.playerData.curHP = pm_gPlayerStatus.playerData.maxHP;
    }
    if (CHEAT_ACTIVE(CHEAT_FP)) {
        pm_gPlayerStatus.playerData.curFP = pm_gPlayerStatus.playerData.curMaxFP;
    }
    if (CHEAT_ACTIVE(CHEAT_COINS)) {
        pm_gPlayerStatus.playerData.coins = 999;
    }
    if (CHEAT_ACTIVE(CHEAT_STAR_POWER)) {
        pm_gPlayerStatus.playerData.starSpiritsFullBarsFilled = pm_gPlayerStatus.playerData.starSpiritsSaved;
        pm_gPlayerStatus.playerData.starSpiritsPartialBarFilled = 0;
    }
    if (CHEAT_ACTIVE(CHEAT_STAR_PIECES)) {
        pm_gPlayerStatus.playerData.starPieces = 160;
    }
    if (CHEAT_ACTIVE(CHEAT_PERIL)) {
        pm_gPlayerStatus.playerData.curHP = 1;
    }
    if (CHEAT_ACTIVE(CHEAT_AUTO_MASH)) {
        if (pm_gGameStatus.isBattle) {
            pm_gActionCommandStatus.barFillLevel = 10000;
        }
    }
    if (CHEAT_ACTIVE(CHEAT_BRIGHTEN_ROOM)) {
        pm_set_screen_overlay_alpha(1, 0);
    }
    if (CHEAT_ACTIVE(CHEAT_AUTO_ACTION_CMD)) {
        pm_gActionCommandStatus.autoSucceed = 1;
    }
}

void fpUpdateWarps(void) {
    if (fp.warpDelay > 0) {
        PRINTF("fp.warp_delay: %d\n", fp.warpDelay);
        fp.warpDelay--;
    }

    if (fp.warp && fp.warpDelay == 0) {
        // if a popup menu is currently hidden, destroy it
        if (pm_popupMenuVar == 10) {
            PRINTF("destroying popup menu\n");
            pm_destroyPopupMenu();
        }

        pm_setMapTransitionEffect(0); // normal black fade
        PRINTF("changing game mode\n");
        pm_setGameMode(5); // start the "change map" game mode
        fp.warp = FALSE;
    }
}

void fpDrawLog(struct GfxFont *font, s32 cellWidth, s32 cellHeight, u8 menuAlpha) {
    for (s32 i = SETTINGS_LOG_MAX - 1; i >= 0; --i) {
        const s32 fadeBegin = 20;
        const s32 fadeDuration = 20;
        struct LogEntry *ent = &fp.log[i];
        u8 msgAlpha;
        if (!ent->msg) {
            continue;
        }
        ++ent->age;
        if (ent->age > (fadeBegin + fadeDuration)) {
            free(ent->msg);
            ent->msg = NULL;
            continue;
        } else if (!settings->bits.log) {
            continue;
        } else if (ent->age > fadeBegin) {
            msgAlpha = 0xFF - (ent->age - fadeBegin) * 0xFF / fadeDuration;
        } else {
            msgAlpha = 0xFF;
        }
        msgAlpha = msgAlpha * menuAlpha / 0xFF;
        s32 msgX = settings->logX - cellWidth * strlen(ent->msg);
        s32 msgY = settings->logY - cellHeight * i;
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(0xC0C0C0, msgAlpha));
        gfxPrintf(font, msgX, msgY, "%s", ent->msg);
    }
}

void fpCamUpdate(void) {
    if (fp.freeCam) {
        if (!fp.camEnabledBefore) {
            fp.camPos.x = pm_gCameras->lookAt_eye.x;
            fp.camPos.y = pm_gCameras->lookAt_eye.y;
            fp.camPos.z = pm_gCameras->lookAt_eye.z;
            fp.camEnabledBefore = TRUE;
        } else {
            fpUpdateCam();
        }
        Vec3f *cameraAt = &pm_gCameras->lookAt_obj;
        Vec3f *cameraEye = &pm_gCameras->lookAt_eye;

        *cameraEye = fp.camPos;

        Vec3f vf;
        vec3fPy(&vf, fp.camPitch, fp.camYaw);
        vec3fAdd(cameraAt, cameraEye, &vf);
    }
}

/**
 * fp's main update function
 * This runs after the base games full update loop every frame
 */
void fpUpdate(void) {
    fpUpdateCpuCounter();
    inputUpdate();

    u16 padPressed = inputPressed();

    if (!fp.versionShown) {
        pm_gGameStatus.bSkipIntro = 1;
    }

    if (!fp.settingsLoaded) {
        if (!(inputPressed() & BUTTON_START) && settingsLoad(fp.profile)) {
            applyMenuSettings();
        }
        fp.settingsLoaded = TRUE;
    }

    fpEmergencySettingsReset(padPressed);

    if (fp.menuActive) {
        fpUpdateMenu();
    } else if (inputBindPressedRaw(COMMAND_MENU)) {
        showMenu();
    }

    timerUpdate();

    if (fp.bowserBlocksEnabled) {
        fpBowserBlockTrainer();
    }

    if (fp.lzsTrainerEnabled) {
        fpLzsTrainer();
    }

    if (fp.clippyTrainerEnabled) {
        fpClippyTrainer();
    }

    if (fp.actionCommandTrainerEnabled) {
        fpActionCommandTrainer();
    }

    fpUpdateCheats();

    if (fp.turbo) {
        pm_gPlayerStatus.runSpeed = 24.0f;
    } else {
        pm_gPlayerStatus.runSpeed = 4.0f;
    }

    for (s32 i = 0; i < COMMAND_MAX; ++i) {
        bool active = FALSE;

        switch (fpCommands[i].commandType) {
            case COMMAND_HOLD: active = inputBindHeld(i); break;
            case COMMAND_PRESS: active = inputBindPressed(i); break;
            case COMMAND_PRESS_ONCE: active = inputBindPressedRaw(i); break;
        }

        if (fpCommands[i].proc && active) {
            fpCommands[i].proc();
        }
    }

    fpUpdateWarps();

    // Override updateMode so update_cameras switch always defaults
    if (fp.freeCam) {
        pm_gCameras[pm_gCurrentCameraID].updateMode = 7;
    }
    fpCamUpdate();

    while (fp.menuActive && menuThink(fp.mainMenu)) {
        // wait
    }

    while (menuThink(fp.global)) {
        // wait
    }
}

/**
 * fp's main draw function
 * This runs after the game draws the front UI every frame
 */
void fpDraw(void) {
    gfxModeInit();

    struct GfxFont *font = menuGetFont(fp.mainMenu, TRUE);
    u8 menuAlpha = menuGetAlphaI(fp.mainMenu, TRUE);
    s32 cellWidth = menuGetCellWidth(fp.mainMenu, TRUE);
    s32 cellHeight = menuGetCellHeight(fp.mainMenu, TRUE);

    if (!fp.versionShown) {
        fpDrawVersion(font, cellWidth, cellHeight, menuAlpha);
    }

    if (settings->bits.inputDisplay) {
        fpDrawInputDisplay(font, cellWidth, cellHeight, menuAlpha);
    }

    enum TimerState timerState = timerGetState();
    if (fp.timerMoving || (timerState == TIMER_STOPPED && !fp.menuActive) ||
        (settings->bits.timerShow && !fp.menuActive && timerState != TIMER_INACTIVE)) {
        fpDrawTimer(font, cellWidth, cellHeight, menuAlpha);
    }

    if (fp.menuActive) {
        menuDraw(fp.mainMenu);
    }

    menuDraw(fp.global);
    fpDrawLog(font, cellWidth, cellHeight, menuAlpha);
    gfxFlush();
}

/* ========================== HOOK ENTRY POINTS ========================== */

ENTRY void fpUpdateEntry(void) {
    init_gp();

    if (!fp.ready) {
        initStack(fpInit);
        PRINTF("\n**** fp initialized ****\n\n");
    }

    pm_step_game_loop();
    initStack(fpUpdate);
}

ENTRY void fpDrawEntry(void) {
    init_gp();
    pm_state_render_frontUI();
    initStack(fpDraw);
}

ENTRY void fpAfterDrawEntry(void) {
    crashScreenSetDrawInfoCustom(nuGfxCfb_ptr, SCREEN_WIDTH, SCREEN_HEIGHT);
}

HOOK void fpUpdateCameraMode6(pm_Camera *cam) {
    if (!fp.freeCam) {
        pm_update_camera_mode_6(cam);
    }
}

HOOK void fpUpdateInput(void) {
    pm_update_player_input();
    pm_Controller *mask = &fp.inputMask;

    pm_gPlayerStatus.currentButtons.buttons &= ~mask->buttons;
    pm_gPlayerStatus.previousButtons.buttons &= ~mask->buttons;
    pm_gPlayerStatus.heldButtons.buttons &= ~mask->buttons;

    pm_gPlayerStatus.stickAxisX &= ~mask->xCardinal;
    pm_gPlayerStatus.stickAxisY &= ~mask->yCardinal;
}

HOOK s32 fpCheckBlockInput(s32 buttonMask) {
    s32 mashWindow;
    s32 blockWindow;
    s32 block;
    s32 mash = 0;
    s32 bufferPos;
    s32 i;

    pm_gBattleStatus.blockResult = 0; // Fail

    if (pm_gBattleStatus.unk_83 == -1 && (pm_gBattleStatus.flags1 & 0x2000000)) {
        pm_gBattleStatus.blockResult = 1;
        return 1;
    }

    if (!pm_gBattleStatus.unk_83 || (pm_gGameStatus.demoState & 1)) {
        return 0;
    }

    if (pm_gPlayerStatus.playerData.hitsTaken < 9999) {
        pm_gPlayerStatus.playerData.hitsTaken += 1;
        pm_gActionCommandStatus.hitsTakenIsMax = 0;
    } else {
        pm_gActionCommandStatus.hitsTakenIsMax = 1;
    }

    block = 0;
    blockWindow = 3;
    mashWindow = 10;

    if (!(pm_gBattleStatus.flags1 & 0x80000) && pm_is_ability_active(0)) {
        blockWindow = 5;
    }

    // Pre-window mashing check
    bufferPos = pm_gBattleStatus.inputBufferPos;
    bufferPos -= mashWindow + blockWindow;

    if (bufferPos < 0) {
        bufferPos += ARRAY_LENGTH(pm_gBattleStatus.pushInputBuffer);
    }
    for (i = 0; i < mashWindow; i++) {
        if (bufferPos >= ARRAY_LENGTH(pm_gBattleStatus.pushInputBuffer)) {
            bufferPos -= ARRAY_LENGTH(pm_gBattleStatus.pushInputBuffer);
        }

        if (pm_gBattleStatus.pushInputBuffer[bufferPos] & buttonMask) {
            if (fp.actionCommandTrainerEnabled) {
                s32 frames_early = mashWindow - i;
                fpLog("blocked %d frame%s early", frames_early, frames_early > 1 ? "s" : "");
            }
            mash = 1;
            break;
        }
        bufferPos++;
    }

    // Block check
    bufferPos = pm_gBattleStatus.inputBufferPos;
    bufferPos -= blockWindow;
    if (bufferPos < 0) {
        bufferPos += ARRAY_LENGTH(pm_gBattleStatus.pushInputBuffer);
    }
    for (i = 0; i < blockWindow; i++) {
        if (bufferPos >= ARRAY_LENGTH(pm_gBattleStatus.pushInputBuffer)) {
            bufferPos -= ARRAY_LENGTH(pm_gBattleStatus.pushInputBuffer);
        }

        if (pm_gBattleStatus.pushInputBuffer[bufferPos] & buttonMask) {
            if (!mash && fp.actionCommandTrainerEnabled) {
                fpLog("blocked frame %d out of %d", i + 1, blockWindow);
            }
            pm_gBattleStatus.blockResult = 1; // Block
            block = 1;
            break;
        }
        bufferPos++;
    }

    if (mash) {
        pm_gBattleStatus.blockResult = -1; // Mash
        block = 0;
    }

    // Ignore inputs until another mash window has passed, so check_block_input() can be called in quick succession
    if (block) {
        bufferPos = pm_gBattleStatus.inputBufferPos;
        bufferPos -= mashWindow + blockWindow + 20;
        if (bufferPos < 0) {
            bufferPos += ARRAY_LENGTH(pm_gBattleStatus.pushInputBuffer);
        }

        for (i = 0; i < mashWindow + blockWindow + 20; i++) {
            if (bufferPos >= ARRAY_LENGTH(pm_gBattleStatus.pushInputBuffer)) {
                bufferPos -= ARRAY_LENGTH(pm_gBattleStatus.pushInputBuffer);
            }
            pm_gBattleStatus.pushInputBuffer[bufferPos] = 0;
            bufferPos++;
        }
    }
    if (block && !pm_gActionCommandStatus.hitsTakenIsMax) {
        pm_gPlayerStatus.playerData.hitsBlocked += 1;
    }

    return block;
}

#include <grc.c>
#include <list/list.c>
#include <set/set.c>
#include <startup.c>
#include <vector/vector.c>
