#include "common.h"
#include "fp.h"
#include "menu.h"
#include <stdlib.h>

static s32 byteModProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    u8 *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menuIntinputGet(item) != *p) {
            menuIntinputSet(item, *p);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        *p = menuIntinputGet(item);
    }
    return 0;
}

static void bowserHallwayProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH8_REACHED_PEACHS_CASTLE;
    fpSetGlobalFlag(0x1fa, FALSE); // hallway not defeated
    fpWarp(AREA_PEACHS_CASTLE, 0x7, 0x0);
}

static void bowserPhase1Proc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH8_REACHED_PEACHS_CASTLE;
    fpSetGlobalFlag(0x1fc, FALSE); // bridge not broken
    fpSetGlobalFlag(0x1fd, FALSE); // not sure, but prevents a crash
    fpSetGlobalFlag(0x1fe, TRUE);  // skip camera zoom in
    fpWarp(AREA_PEACHS_CASTLE, 0x13, 0x0);
}

static void bowserPhase2Proc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH8_REACHED_PEACHS_CASTLE;
    fpSetGlobalFlag(0x1fc, TRUE); // bridge broken
    fpWarp(AREA_PEACHS_CASTLE, 0x13, 0x1);
}

static void goombaKingProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH0_DEFEATED_GOOMBA_BROS;
    fpSetGlobalFlag(0x02d, TRUE); // skip cutscene
    fpWarp(AREA_GOOMBA_VILLAGE, 0x9, 0x0);
}

static void koopaBrosProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH1_KOOPA_BROS_FIRING_BLASTERS;
    fpWarp(AREA_KOOPA_BROS_FORTRESS, 0xa, 0x0);
}

static void tutankoopaProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH2_SOLVED_ARTIFACT_PUZZLE;
    fpWarp(AREA_DRY_DRY_RUINS, 0xe, 0x0);
}

static void tubbaBlubbaProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH3_HEART_ESCAPED_WINDY_MILL;
    fpWarp(AREA_GUSTY_GULCH, 0x4, 0x0);
}

static void generalGuyProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH4_OPENED_GENERAL_GUY_ROOM;
    fpWarp(AREA_SHY_GUYS_TOY_BOX, 0xe, 0x0);
}

static void lavaPiranhaProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH5_KOLORADO_IN_TREASURE_ROOM;
    fpWarp(AREA_VOLCANO, 0xd, 0x1);
}

static void huffNPuffProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH6_GREW_MAGIC_BEANSTALK;
    fpWarp(AREA_FLOWER_FIELDS, 0xf, 0x0);
}

static void crystalKingProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH7_SOLVED_ALBINO_DINO_PUZZLE;
    fpWarp(AREA_CRYSTAL_PALACE, 0x17, 0x0);
}

static void jrPlaygroundProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH0_FOUND_HAMMER;
    u8 *partner = &pm_gPlayerStatus.playerData.currentPartner;
    if (*partner == 4 || *partner == 6 || *partner == 8 || *partner == 9) { // flying partners cause a softlock
        *partner = 1;                                                       // goombario
    }
    fpWarp(AREA_GOOMBA_VILLAGE, 0x3, 0x0);
}

static void jrPleasantPathProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH1_STAR_SPRIT_DEPARTED;
    fpWarp(AREA_KOOPA_VILLAGE_PLEASANT_PATH, 0x4, 0x1);
}

static void jrForeverForestProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH3_STAR_SPRIT_DEPARTED;
    fpSetGlobalFlag(0x39f, FALSE); // jr not defeated
    fpWarp(AREA_FOREVER_FOREST, 0x6, 0x3);
}

static void jrToadTownPortProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH5_STAR_SPIRIT_DEPARTED;
    fpSetGlobalFlag(0x4c2, FALSE); // jr not defeated
    fpWarp(AREA_TOAD_TOWN, 0x6, 0x1);
}

static void jrShiverSnowfieldProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH7_MAYOR_MURDER_SOLVED;
    fpWarp(AREA_SHIVER_REGION, 0x2, 0x0);
}

static void jrBowsersCastleProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH8_REACHED_BOWSERS_CASTLE;
    fpSetGlobalByte(0x12c, 0);
    fpWarp(AREA_BOWSERS_CASTLE, 0x1c, 0x0);
}

static void goombaBrosProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH0_SMASHED_GATE_BLOCK;
    fpWarp(AREA_GOOMBA_VILLAGE, 0x6, 0x0);
}

static void tubbasHeartProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH3_WENT_DOWN_THE_WELL;
    fpWarp(AREA_GUSTY_GULCH, 0x8, 0x0);
}

static void lanternGhostProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH4_SOLVED_COLOR_PUZZLE;
    fpWarp(AREA_SHY_GUYS_TOY_BOX, 0xb, 0x0);
}

static void fuzzipedeProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH5_ENTERED_WHALE;
    fpWarp(AREA_WHALE, 0x1, 0x0);
}

static void lakilesterProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH6_SPOKE_WITH_THE_SUN;
    fpWarp(AREA_FLOWER_FIELDS, 0x8, 0x1);
}

static void monstarProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH7_DEFEATED_JR_TROOPA;
    fpWarp(AREA_SHIVER_REGION, 0x4, 0x0);
}

static void blooperProc(struct MenuItem *item, void *data) {
    fpSetGlobalFlag(0x1ab, FALSE); // blooper not defeated
    fpSetGlobalFlag(0x1ac, FALSE); // electro blooper not defeated
    fpSetGlobalFlag(0x1af, FALSE); // ch5 pipe switch
    fpWarp(AREA_SEWERS, 0x7, 0x1);
}

static void electroBlooperProc(struct MenuItem *item, void *data) {
    fpSetGlobalFlag(0x1ab, TRUE);  // blooper defeated
    fpSetGlobalFlag(0x1ac, FALSE); // electro blooper not defeated
    fpSetGlobalFlag(0x1af, FALSE); // ch5 pipe switch
    fpWarp(AREA_SEWERS, 0x7, 0x1);
}

static void superBlooperProc(struct MenuItem *item, void *data) {
    fpSetGlobalFlag(0x1ab, TRUE);  // blooper defeated
    fpSetGlobalFlag(0x1ac, TRUE);  // electro blooper defeated
    fpSetGlobalFlag(0x1af, FALSE); // ch5 pipe switch
    fpWarp(AREA_SEWERS, 0x7, 0x1);
}

static void buzzarProc(struct MenuItem *item, void *data) {
    fpSetGlobalFlag(0x2c4, FALSE); // buzzar not defeated
    fpWarp(AREA_MT_RUGGED, 0x4, 0x1);
}

static void antiGuyProc(struct MenuItem *item, void *data) {
    fpSetGlobalFlag(0x451, FALSE); // anti guy not defeated
    fpWarp(AREA_SHY_GUYS_TOY_BOX, 0xc, 0x1);
}

static void kentCKoopaProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH6_RETURNED_TO_TOAD_TOWN;
    fpSetGlobalFlag(0x262, FALSE); // kent not defeated
    fpWarp(AREA_KOOPA_VILLAGE_PLEASANT_PATH, 0x4, 0x0);
}

static void antiGuysUnitProc(struct MenuItem *item, void *data) {
    STORY_PROGRESS = STORY_CH8_REACHED_BOWSERS_CASTLE;
    fpSetGlobalByte(0x12b, 0);
    fpWarp(AREA_BOWSERS_CASTLE, 0x1b, 0x0);
}

static void chanProc(struct MenuItem *item, void *data) {
    fpSetGlobalByte(0x1C, 0);
    if (pm_gGameStatus.areaID == 0x1 && pm_gGameStatus.mapID == 0x1 && !pm_gGameStatus.isBattle) {
        fpLog("dojo set to chan");
    } else {
        fpWarp(AREA_TOAD_TOWN, 0x1, 0x1);
    }
}

static void leeProc(struct MenuItem *item, void *data) {
    fpSetGlobalByte(0x1C, 1);
    if (pm_gGameStatus.areaID == 0x1 && pm_gGameStatus.mapID == 0x1 && !pm_gGameStatus.isBattle) {
        fpLog("dojo set to lee");
    } else {
        fpWarp(AREA_TOAD_TOWN, 0x1, 0x1);
    }
}

static void master1Proc(struct MenuItem *item, void *data) {
    fpSetGlobalByte(0x1C, 2);
    if (pm_gGameStatus.areaID == 0x1 && pm_gGameStatus.mapID == 0x1 && !pm_gGameStatus.isBattle) {
        fpLog("dojo set to master 1");
    } else {
        fpWarp(AREA_TOAD_TOWN, 0x1, 0x1);
    }
}

static void master2Proc(struct MenuItem *item, void *data) {
    fpSetGlobalByte(0x1C, 3);
    if (pm_gGameStatus.areaID == 0x1 && pm_gGameStatus.mapID == 0x1 && !pm_gGameStatus.isBattle) {
        fpLog("dojo set to master 2");
    } else {
        fpWarp(AREA_TOAD_TOWN, 0x1, 0x1);
    }
}

static void master3Proc(struct MenuItem *item, void *data) {
    fpSetGlobalByte(0x1C, 4);
    if (pm_gGameStatus.areaID == 0x1 && pm_gGameStatus.mapID == 0x1 && !pm_gGameStatus.isBattle) {
        fpLog("dojo set to master 3");
    } else {
        fpWarp(AREA_TOAD_TOWN, 0x1, 0x1);
    }
}

static void tabPrevProc(struct MenuItem *item, void *data) {
    menuTabPrevious(data);
}

static void tabNextProc(struct MenuItem *item, void *data) {
    menuTabNext(data);
}

void createBossesMenu(struct Menu *menu) {
    s32 yMain = 0;

    /* initialize menu */
    menuInit(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu->selector = menuAddSubmenu(menu, 0, yMain++, NULL, "return");

    s32 pageCount = 6;
    struct Menu *pages = malloc(sizeof(*pages) * pageCount);
    struct MenuItem *tab = menuAddTab(menu, 0, yMain++, pages, pageCount);
    for (s32 i = 0; i < pageCount; ++i) {
        struct Menu *page = &pages[i];
        menuInit(page, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    }

    /* bowser */
    s32 yTab = 0;
    struct Menu *page = &pages[0];
    menuAddStatic(page, 0, yTab++, "bowser", 0xC0C0C0);
    menuAddButton(page, 0, yTab++, "hallway", bowserHallwayProc, NULL);
    menuAddButton(page, 0, yTab++, "final phase 1", bowserPhase1Proc, NULL);
    menuAddButton(page, 0, yTab++, "final phase 2", bowserPhase2Proc, NULL);
    yTab++;
    menuAddStatic(page, 0, yTab, "phase 2 hp:", 0xC0C0C0);
    menuAddIntinput(page, 12, yTab++, 10, 2, byteModProc, &pm_gCurrentSaveFile.globalBytes[0x18a]);

    /* chapter bosses */
    yTab = 0;
    page = &pages[1];
    menuAddStatic(page, 0, yTab++, "chapter bosses", 0xC0C0C0);
    menuAddButton(page, 0, yTab++, "goomba king", goombaKingProc, NULL);
    menuAddButton(page, 0, yTab++, "koopa bros.", koopaBrosProc, NULL);
    menuAddButton(page, 0, yTab++, "tutankoopa", tutankoopaProc, NULL);
    menuAddButton(page, 0, yTab++, "tubba blubba", tubbaBlubbaProc, NULL);
    menuAddButton(page, 0, yTab++, "general guy", generalGuyProc, NULL);
    menuAddButton(page, 0, yTab++, "lava piranha", lavaPiranhaProc, NULL);
    menuAddButton(page, 0, yTab++, "huff n. puff", huffNPuffProc, NULL);
    menuAddButton(page, 0, yTab++, "crystal king", crystalKingProc, NULL);

    /* jr troopa */
    yTab = 0;
    page = &pages[2];
    menuAddStatic(page, 0, yTab++, "jr. troopa", 0xC0C0C0);
    menuAddButton(page, 0, yTab++, "playground", jrPlaygroundProc, NULL);
    menuAddButton(page, 0, yTab++, "pleasant path", jrPleasantPathProc, NULL);
    menuAddButton(page, 0, yTab++, "forever forest", jrForeverForestProc, NULL);
    menuAddButton(page, 0, yTab++, "toad town port", jrToadTownPortProc, NULL);
    menuAddButton(page, 0, yTab++, "shiver snowfield", jrShiverSnowfieldProc, NULL);
    menuAddButton(page, 0, yTab++, "bowser's castle", jrBowsersCastleProc, NULL);

    /* minor bosses */
    yTab = 0;
    page = &pages[3];
    menuAddStatic(page, 0, yTab++, "minor bosses", 0xC0C0C0);
    menuAddButton(page, 0, yTab++, "goomba bros.", goombaBrosProc, NULL);
    menuAddButton(page, 0, yTab++, "tubba's heart", tubbasHeartProc, NULL);
    menuAddButton(page, 0, yTab++, "big lantern ghost", lanternGhostProc, NULL);
    menuAddButton(page, 0, yTab++, "fuzzipede", fuzzipedeProc, NULL);
    menuAddButton(page, 0, yTab++, "lakilester", lakilesterProc, NULL);
    menuAddButton(page, 0, yTab++, "monstar", monstarProc, NULL);

    /* optional bosses */
    yTab = 0;
    page = &pages[4];
    menuAddStatic(page, 0, yTab++, "optional bosses", 0xC0C0C0);
    menuAddButton(page, 0, yTab++, "blooper", blooperProc, NULL);
    menuAddButton(page, 0, yTab++, "electro blooper", electroBlooperProc, NULL);
    menuAddButton(page, 0, yTab++, "super blooper", superBlooperProc, NULL);
    menuAddButton(page, 0, yTab++, "buzzar", buzzarProc, NULL);
    menuAddButton(page, 0, yTab++, "anti guy", antiGuyProc, NULL);
    menuAddButton(page, 0, yTab++, "kent c. koopa", kentCKoopaProc, NULL);
    menuAddButton(page, 0, yTab++, "anti guys unit", antiGuysUnitProc, NULL);

    /* dojo */
    yTab = 0;
    page = &pages[5];
    menuAddStatic(page, 0, yTab++, "dojo", 0xC0C0C0);
    menuAddButton(page, 0, yTab++, "chan", chanProc, NULL);
    menuAddButton(page, 0, yTab++, "lee", leeProc, NULL);
    menuAddButton(page, 0, yTab++, "master 1", master1Proc, NULL);
    menuAddButton(page, 0, yTab++, "master 2", master2Proc, NULL);
    menuAddButton(page, 0, yTab++, "master 3", master3Proc, NULL);

    menuTabGoto(tab, 0);
    menuAddButton(menu, 8, 0, "<", tabPrevProc, tab);
    menuAddButton(menu, 10, 0, ">", tabNextProc, tab);
}
