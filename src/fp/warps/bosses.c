#include "bosses.h"
#include "common.h"
#include "menu/menu.h"
#include <stdlib.h>

typedef struct BattleInfo {
    const char *name;
    s16 battleId;
    s16 stageId;
    s32 songId;
} BattleInfo;

typedef struct BattlePage {
    const char *name;
    u8 battleCount;
    BattleInfo battles[];
} BattlePage;

// clang-format off
static BattlePage pageBowser = {
    "bowser",
    3,
    {
        {"hallway", 0x2302, -1, 102},
        {"final phase 1", 0x2303, -1, 103},
        {"final phase 2", 0x2304, -1, 5},
    }
};

static BattlePage pageChapterBosses = {
    "chapter bosses",
    8,
    {
        {"goomba king", 0x101, -1, 7},
        {"koopa bros.", 0x700, -1, 9},
        {"tutankoopa", 0xc00, 1, 10},
        {"tubba blubba", 0xe10, 1, 11},
        {"general guy", 0x1100, -1, 12},
        {"lava piranha", 0x1700, 6, 13},
        {"huff n. puff", 0x1900, 7, 14},
        {"crystal king", 0x2000, -1, 15},
    }
};

static BattlePage pageJrTroopa = {
    "jr. troopa",
    6,
    {
        {"playground", 0x202, -1, 4},
        {"pleasant path", 0x203, -1, 4},
        {"forever forest", 0x204, -1, 4},
        {"toad town port", 0x205, -1, 4},
        {"shiver snowfield", 0x206, -1, 4},
        {"bowser's castle", 0x207, -1, 4},
    }
};

static BattlePage pageMinorBosses = {
    "minor bosses",
    6,
    {
        {"goomba bros.", 0x100, 1, 3},
        {"tubba's heart", 0xe0f, -1, 11},
        {"big lantern ghost", 0x1200, -1, 3},
        {"fuzzipede", 0x1300, 0, 3},
        {"lakilester", 0x1905, 1, 3},
        {"monstar", 0x1e00, 5, 3},
    }
};

static BattlePage pageOptionalBosses = {
    "optional bosses",
    7,
    {
        {"blooper", 0x1b00, 0, 3},
        {"electro blooper", 0x1b01, 0, 3},
        {"super blooper", 0x1b02, 0, 3},
        {"buzzar", 0x90c, 2, 3},
        {"anti guy", 0x102c, 0, 3},
        {"kent c. koopa", 0x518, 0, 3},
        {"anti guys unit", 0x2400, 0, 3},
    }
};

static BattlePage pageDojo = {
    "dojo",
    5,
    {
        {"chan", 0x300, -1, -1},
        {"lee", 0x301, -1, -1},
        {"master 1", 0x302, -1, 44},
        {"master 2", 0x303, -1, 44},
        {"master 3", 0x304, -1, 44},
    }
};
// clang-format on

static BattlePage *pageList[] = {&pageBowser,      &pageChapterBosses,  &pageJrTroopa,
                                 &pageMinorBosses, &pageOptionalBosses, &pageDojo};

pm_Npc bossesDummyNpc = {0};

static pm_Enemy dummyEnemy = {.npcID = BOSSES_DUMMY_ID};
static pm_Encounter dummyEncounter = {.enemy[0] = &dummyEnemy};
static s8 warpCountdown = 0;
static bool leavingBattle = FALSE;
static u8 page = 0;
static u8 battle = 0;

static void bossWarp() {
    BattleInfo info = pageList[page]->battles[battle];

    dummyEncounter.battle = info.battleId;
    dummyEncounter.stage = info.stageId;

    pm_EncounterStatus *es = &pm_gCurrentEncounter;
    es->curEncounter = &dummyEncounter;
    es->curEnemy = &dummyEnemy;
    es->hitType = 1;
    es->firstStrikeType = 0;
    es->forbidFleeing = FALSE;
    es->scriptedBattle = TRUE;
    es->songID = info.songId;

    pm_gEncounterState = 3;    // ENCOUNTER_STATE_PRE_BATTLE
    pm_gEncounterSubState = 0; // ENCOUNTER_SUBSTATE_PRE_BATTLE_INIT
    warpCountdown = 31;
    // may want to try and clear speech bubbles, but it doesn't seem to crash, so not worried for now
}

static void bossWarpProc(struct MenuItem *item, void *data) {
    if (warpCountdown > 0) {
        // prevent warp spamming
        return;
    }
    page = (u8)((s32)data >> 8);
    battle = (u8)((s32)data & 0xFF);
    pm_clear_windows();
    pm_clear_printers();
    // context is also true when paused, so checking game mode
    if (pm_gGameStatus.context != CONTEXT_WORLD && pm_CurGameMode != 10 && pm_CurGameMode != 11) {
        // end battle cleanly so next fight can start fresh
        pm_gBattleState = 32;   // BATTLE_STATE_END_BATTLE
        pm_gBattleSubState = 2; // BTL_SUBSTATE_END_BATTLE_EXEC_STAGE_SCRIPT
        pm_bgm_pop_battle_song();
        leavingBattle = TRUE;
        return;
    }
    bossWarp();
}

void bossesUpdateWarps() {
    if (leavingBattle && pm_gGameStatus.context == CONTEXT_WORLD) {
        leavingBattle = FALSE;
        bossWarp();
    } else if (warpCountdown == 30) {
        // speed up warp fadeout
        pm_gCurrentEncounter.fadeOutAmount = 0xFF;
        pm_gCurrentEncounter.battleStartCountdown = 0;
        warpCountdown--;
    } else if (warpCountdown > 30) {
        if (pm_CurGameMode == 10) {
            // unpause
            pm_set_game_mode(11);
        } else if (pm_CurGameMode != 11) {
            // delay countdown until unpaused
            warpCountdown--;
        }
    } else if (warpCountdown > 0) {
        warpCountdown--;
    }
}

void createBossesMenu(struct Menu *menu) {
    s32 yMain = 0;

    /* initialize menu */
    menuInit(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu->selector = menuAddSubmenu(menu, 0, yMain++, NULL, "return");

    s32 pageCount = 6;
    struct Menu *pages = malloc(sizeof(*pages) * ARRAY_LENGTH(pageList));
    struct MenuItem *tab = menuAddTab(menu, 0, yMain++, pages, pageCount);
    for (u8 pageIdx = 0; pageIdx < pageCount; ++pageIdx) {
        struct Menu *page = &pages[pageIdx];
        menuInit(page, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

        s32 yTab = 0;
        BattlePage *battlePage = pageList[pageIdx];
        menuAddStatic(page, 0, yTab++, battlePage->name, 0xC0C0C0);
        for (u8 battleIdx = 0; battleIdx < battlePage->battleCount; battleIdx++) {
            s32 data = pageIdx << 8;
            data |= battleIdx;
            menuAddButton(page, 0, yTab++, battlePage->battles[battleIdx].name, bossWarpProc, (void *)data);
        }
        if (pageIdx == 0) {
            yTab++;
            menuAddStatic(page, 0, yTab, "phase 2 hp:", 0xC0C0C0);
            menuAddIntinput(page, 12, yTab++, 10, 2, menuByteModProc, &pm_gCurrentSaveFile.globalBytes[0x18a]);
        }
    }

    menuTabGoto(tab, 0);
    menuAddButton(menu, 8, 0, "<", menuTabPrevProc, tab);
    menuAddButton(menu, 10, 0, ">", menuTabNextProc, tab);
}
