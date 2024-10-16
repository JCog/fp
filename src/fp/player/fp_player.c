#include "common.h"
#include "fp.h"
#include "items.h"
#include "menu/menu.h"
#include "sys/gfx.h"
#include "sys/resource.h"
#include <stdlib.h>

static const char *strHp = "HP";
static const char *strMaxHp = "Max HP";
static const char *strFp = "FP";
static const char *strMaxFp = "Max FP";
static const char *strBp = "BP";
static const char *strCoins = "Coins";
static const char *strStarPieces = "Star Pieces";
static const char *strLevel = "Level";
static const char *strStarPoints = "Star Points";
static const char *strActionCommands = "Action Commands";
static const char *strBootsNormal = "Normal Boots";
static const char *strBootsSuper = "Super Boots";
static const char *strBootsUltra = "Ultra Boots";
static const char *strHammerNormal = "Normal Hammer";
static const char *strHammerSuper = "Super Hammer";
static const char *strHammerUltra = "Ultra Hammer";
static const char *strPartnerNames[] = {
    "Goombario", "Kooper",     "Bombette", "Parakarry", "Bow",   "Watt",
    "Sushie",    "Lakilester", "Goompa",   "Goombaria", "Twink",
};
static const char *strStarSpiritNames[] = {
    "Eldstar", "Mamar", "Skolar", "Muskular", "Misstar", "Klevar", "Kalmar",
};
static const char *strStarPeachBeam = "Star/Peach Beam";
static const char *strSuperRank = "Super Rank";
static const char *strUltraRank = "Ultra Rank";
static const u32 partnerOrder[] = {
    0, 1, 2, 3, 4, 9, 6, 7, 8, 5, 10, 11,
};

static struct GfxTexture *itemTextureList[0x16D];

static struct GfxTexture **getItemTextureList(void) {
    static bool ready = FALSE;
    if (!ready) {
        ready = TRUE;
        for (u16 i = 0; i < 0x16D; i++) {
            itemTextureList[i] = resourceLoadPmiconItem(i, FALSE);
        }
    }
    return itemTextureList;
}

static s32 maxHpProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    s8 *curMaxHP = &pm_gPlayerStatus.playerData.curMaxHP;
    s8 *hardMaxHP = &pm_gPlayerStatus.playerData.hardMaxHP;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menuIntinputGet(item) != *curMaxHP) {
            menuIntinputSet(item, *curMaxHP);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        s8 diff = *curMaxHP - *hardMaxHP;
        s8 newHP = menuIntinputGet(item);
        if (newHP - diff < 0) {
            newHP = diff;
        }
        *curMaxHP = newHP;
        *hardMaxHP = newHP - diff;
    }
    return 0;
}

static s32 maxFpProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    s8 *curMaxFP = &pm_gPlayerStatus.playerData.curMaxFP;
    s8 *hardMaxFP = &pm_gPlayerStatus.playerData.hardMaxFP;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menuIntinputGet(item) != *curMaxFP) {
            menuIntinputSet(item, *curMaxFP);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        s8 diff = *curMaxFP - *hardMaxFP;
        s8 newFP = menuIntinputGet(item);
        if (newFP - diff < 0) {
            newFP = diff;
        }
        *curMaxFP = newFP;
        *hardMaxFP = newFP - diff;
    }
    return 0;
}

static s32 currentPartnerProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        menuOptionSet(item, partnerOrder[pm_gPlayerStatus.playerData.currentPartner]);
    } else if (reason == MENU_CALLBACK_DEACTIVATE) {
        pm_gPlayerStatus.playerData.currentPartner = partnerOrder[menuOptionGet(item)];
    }
    return 0;
}

static s32 bootsProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    u32 trackedLevel = (u32)data;
    s8 *bootsUpgrade = &pm_gPlayerStatus.playerData.bootsLevel;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        *bootsUpgrade = trackedLevel;
    } else if (reason == MENU_CALLBACK_THINK) {
        menuSwitchSet(item, *bootsUpgrade == trackedLevel);
    }
    return 0;
}

static s32 hammerProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    u32 trackedLevel = (u32)data;
    s8 *hammerUpgrade = &pm_gPlayerStatus.playerData.hammerLevel;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        *hammerUpgrade = trackedLevel;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        *hammerUpgrade = 0xFF;
    } else if (reason == MENU_CALLBACK_THINK) {
        menuSwitchSet(item, *hammerUpgrade == trackedLevel);
    }
    return 0;
}

static s32 superRankProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    pm_PartnerData *partner = (pm_PartnerData *)data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        partner->level = 1;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        if (partner->level == 2) {
            partner->level = 1;
        } else {
            partner->level = 0;
        }
    } else if (reason == MENU_CALLBACK_THINK) {
        menuSwitchSet(item, partner->level > 0);
    }
    return 0;
}

static s32 ultraRankProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    pm_PartnerData *partner = (pm_PartnerData *)data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        partner->level = 2;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        partner->level = 0;
    } else if (reason == MENU_CALLBACK_THINK) {
        menuSwitchSet(item, partner->level == 2);
    }
    return 0;
}

static s32 starSpiritSwitchProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    s8 *ssSaved = &pm_gPlayerStatus.playerData.maxStarPower;
    s16 *starPower = &pm_gPlayerStatus.playerData.starPower;
    s32 ssIndex = (u32)data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        *ssSaved = ssIndex;
        *starPower = ssIndex * 0x100;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        if (*ssSaved == ssIndex) {
            if (*starPower < ssIndex * 0x100) {
                *starPower = ssIndex * 0x100;
            } else {
                *ssSaved = 0;
                *starPower = 0;
            }
        } else {
            *ssSaved = ssIndex;
            *starPower = ssIndex * 0x100;
        }
    } else if (reason == MENU_CALLBACK_THINK) {
        menuSwitchSet(item, *ssSaved >= ssIndex);
    }
    return 0;
}

static s32 peachFlagProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    // 0 = peach or mario, 1 = transformed, 3 = parasol
    u32 flag = (u32)data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_gGameStatus.peachFlags |= (1 << flag);
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_gGameStatus.peachFlags &= ~(1 << flag);
    } else if (reason == MENU_CALLBACK_THINK) {
        menuCheckboxSet(item, pm_gGameStatus.peachFlags & (1 << flag));
    }
    return 0;
}

static void createGearMenu(struct Menu *menu) {
    menu->selector = menuAddSubmenu(menu, 0, 0, NULL, "return");
    menuAddTooltip(menu, 8, 0, fp.mainMenu, 0xC0C0C0);

    struct GfxTexture **itemTextures = getItemTextureList();

    struct GfxTexture *texBootsNormal = itemTextures[ITEM_JUMP];
    struct GfxTexture *texBootsSuper = itemTextures[ITEM_SPIN_JUMP];
    struct GfxTexture *texBootsUltra = itemTextures[ITEM_TORNADO_JUMP];
    struct GfxTexture *texHammerNormal = itemTextures[ITEM_HAMMER];
    struct GfxTexture *texHammerSuper = itemTextures[ITEM_SUPER_HAMMER];
    struct GfxTexture *texHammerUltra = itemTextures[ITEM_ULTRA_HAMMER];
    gfxAddGrayscalePalette(texBootsNormal, 0);
    gfxAddGrayscalePalette(texBootsSuper, 0);
    gfxAddGrayscalePalette(texBootsUltra, 0);
    gfxAddGrayscalePalette(texHammerNormal, 0);
    gfxAddGrayscalePalette(texHammerSuper, 0);
    gfxAddGrayscalePalette(texHammerUltra, 0);

    struct MenuItem *item;
    s32 bootsX = 1;
    s32 bootsY = 2;
    item = menuAddSwitch(menu, bootsX, bootsY, texBootsNormal, 0, 0, 0xFFFFFF, texBootsNormal, 0, 1, 0xFFFFFF, 0.7f,
                         FALSE, bootsProc, (void *)0);
    item->tooltip = strBootsNormal;
    item = menuAddSwitch(menu, bootsX + 3, bootsY, texBootsSuper, 0, 0, 0xFFFFFF, texBootsSuper, 0, 1, 0xFFFFFF, 0.7f,
                         FALSE, bootsProc, (void *)1);
    item->tooltip = strBootsSuper;
    item = menuAddSwitch(menu, bootsX + 6, bootsY, texBootsUltra, 0, 0, 0xFFFFFF, texBootsUltra, 0, 1, 0xFFFFFF, 0.7f,
                         FALSE, bootsProc, (void *)2);
    item->tooltip = strBootsUltra;

    s32 hammerX = 1;
    s32 hammerY = 5;
    item = menuAddSwitch(menu, hammerX, hammerY, texHammerNormal, 0, 0, 0xFFFFFF, texHammerNormal, 0, 1, 0xFFFFFF, 0.7f,
                         FALSE, hammerProc, (void *)0);
    item->tooltip = strHammerNormal;
    item = menuAddSwitch(menu, hammerX + 3, hammerY, texHammerSuper, 0, 0, 0xFFFFFF, texHammerSuper, 0, 1, 0xFFFFFF,
                         0.7f, FALSE, hammerProc, (void *)1);
    item->tooltip = strHammerSuper;
    item = menuAddSwitch(menu, hammerX + 6, hammerY, texHammerUltra, 0, 0, 0xFFFFFF, texHammerUltra, 0, 1, 0xFFFFFF,
                         0.7f, FALSE, hammerProc, (void *)2);
    item->tooltip = strHammerUltra;
}

static void createStatsMenu(struct Menu *menu) {
    menu->selector = menuAddSubmenu(menu, 0, 0, NULL, "return");
    menuAddTooltip(menu, 8, 0, fp.mainMenu, 0xC0C0C0);

    struct GfxTexture **itemTextures = getItemTextureList();

    struct GfxTexture *texLuckyStar = itemTextures[ITEM_LUCKY_STAR];
    struct GfxTexture *texHeart = resourceLoadPmiconGlobal(ICON_STATUS_HEART, 1);
    struct GfxTexture *texFlower = resourceLoadPmiconGlobal(ICON_STATUS_FLOWER, 1);
    struct GfxTexture *texBpIcon = resourceGet(RES_PMICON_BP);
    struct GfxTexture *texMarioHead = resourceLoadPmiconGlobal(ICON_MARIO_HEAD, 1);
    struct GfxTexture *texStarPoint = resourceLoadPmiconGlobal(ICON_STATUS_STAR_POINT, 1);
    struct GfxTexture *texStarPiece = resourceLoadPmiconGlobal(ICON_STATUS_STAR_PIECE, 1);
    struct GfxTexture *texCoin = resourceLoadPmiconGlobal(ICON_STATUS_COIN, 1);
    gfxAddGrayscalePalette(texLuckyStar, 0);

    struct MenuItem *item;

    s32 hpX = 1;
    s32 hpY = 2;
    menuAddStaticIcon(menu, hpX, hpY, texHeart, 0, 0xFFFFFF, 1.0f);
    item = menuAddIntinput(menu, hpX + 2, hpY, 10, 2, menuByteModProc, &pm_gPlayerStatus.playerData.curHP);
    item->tooltip = strHp;
    menuAddStatic(menu, hpX + 4, hpY, "/", 0xC0C0C0);
    item = menuAddIntinput(menu, hpX + 5, hpY, 10, 2, maxHpProc, NULL);
    item->tooltip = strMaxHp;

    s32 fpX = 1;
    s32 fpY = 4;
    menuAddStaticIcon(menu, fpX, fpY, texFlower, 0, 0xFFFFFF, 1.0f);
    item = menuAddIntinput(menu, fpX + 2, fpY, 10, 2, menuByteModProc, &pm_gPlayerStatus.playerData.curFP);
    item->tooltip = strFp;
    menuAddStatic(menu, fpX + 4, fpY, "/", 0xC0C0C0);
    item = menuAddIntinput(menu, fpX + 5, fpY, 10, 2, maxFpProc, NULL);
    item->tooltip = strMaxFp;

    s32 bpX = 1;
    s32 bpY = 6;
    menuAddStaticIcon(menu, bpX, bpY, texBpIcon, 0, 0xFFFFFF, 1.0f);
    item = menuAddIntinput(menu, bpX + 2, bpY, 10, 2, menuByteModProc, &pm_gPlayerStatus.playerData.maxBP);
    item->tooltip = strBp;

    s32 coinX = 10;
    s32 coinY = 2;
    menuAddStaticIcon(menu, coinX, coinY, texCoin, 0, 0xFFFFFF, 1.0f);
    item = menuAddIntinput(menu, coinX + 2, coinY, 10, 3, menuHalfwordModProc, &pm_gPlayerStatus.playerData.coins);
    item->tooltip = strCoins;

    s32 starPieceX = 10;
    s32 starPieceY = 4;
    menuAddStaticIcon(menu, starPieceX, starPieceY, texStarPiece, 0, 0xFFFFFF, 1.0f);
    item = menuAddIntinput(menu, starPieceX + 2, starPieceY, 10, 3, menuByteModProc,
                           &pm_gPlayerStatus.playerData.starPieces);
    item->tooltip = strStarPieces;

    s32 levelX = 17;
    s32 levelY = 2;
    menuAddStaticIcon(menu, levelX, levelY, texMarioHead, 0, 0xFFFFFF, 1.0f);
    item = menuAddIntinput(menu, levelX + 2, levelY, 10, 2, menuByteModProc, &pm_gPlayerStatus.playerData.level);
    item->tooltip = strLevel;

    s32 starPointX = 17;
    s32 starPointY = 4;
    menuAddStaticIcon(menu, starPointX, starPointY, texStarPoint, 0, 0xFFFFFF, 1.0f);
    item = menuAddIntinput(menu, starPointX + 2, starPointY, 10, 2, menuByteModProc,
                           &pm_gPlayerStatus.playerData.starPoints);
    item->tooltip = strStarPoints;

    s32 actionCommandX = 23;
    s32 actionCommandY = 2;
    item =
        menuAddSwitch(menu, actionCommandX, actionCommandY, texLuckyStar, 0, 0, 0xFFFFFF, texLuckyStar, 0, 1, 0xFFFFFF,
                      0.7f, FALSE, menuByteSwitchToggleProc, &pm_gPlayerStatus.playerData.hasActionCommands);
    item->tooltip = strActionCommands;
}

static void createPartyMenu(struct Menu *menu) {
    struct MenuItem *returnItem = menu->selector = menuAddSubmenu(menu, 0, 0, NULL, "return");
    menuAddTooltip(menu, 8, 0, fp.mainMenu, 0xC0C0C0);
    const s32 baseX = 1;
    const s32 baseY = 4;
    const s32 colHeight = 4;
    const s32 spacingX = 6;
    const s32 spacingY = 4;
    const f32 scale = 0.7f;

    struct MenuItem *partners[8];
    struct MenuItem *superRanks[8];
    struct MenuItem *ultraRanks[8];
    struct GfxTexture *texPartner = resourceGet(RES_PMICON_PARTNERS);
    struct GfxTexture *texRank = resourceLoadPmiconGlobal(ICON_PARTNER_RANK_1_A, 1);
    gfxAddGrayscalePalette(texRank, 0);
    gfxTextureTranslate(texRank, 0, 2, 2);

    menuAddStatic(menu, 0, 2, "active", 0xC0C0C0);
    struct MenuItem *activeItem = menuAddOption(menu, 7, 2,
                                                "none\0"
                                                "goombario\0"
                                                "kooper\0"
                                                "bombette\0"
                                                "parakarry\0"
                                                "bow\0"
                                                "watt\0"
                                                "sushie\0"
                                                "lakilester\0"
                                                "goompa\0"
                                                "goombaria\0"
                                                "twink\0",
                                                currentPartnerProc, NULL);
    menuItemAddChainLink(returnItem, activeItem, MENU_NAVIGATE_DOWN);

    for (s32 i = 0; i < 8; i++) {
        s32 partnerX = baseX + (i / colHeight) * spacingX;
        s32 partnerY = baseY + (i % colHeight) * spacingY;

        partners[i] = menuAddSwitch(menu, partnerX, partnerY, texPartner, i + 1, 0, 0xFFFFFF, texPartner, i + 1, 1,
                                    0xFFFFFF, scale, FALSE, menuByteSwitchToggleProc,
                                    &pm_gPlayerStatus.playerData.partners[partnerOrder[i + 1]].enabled);
        partners[i]->tooltip = strPartnerNames[i];

        // super tex
        superRanks[i] =
            menuAddSwitch(menu, partnerX + 2, partnerY, texRank, 0, 0, 0xFFFFFF, texRank, 0, 1, 0xFFFFFF, scale, FALSE,
                          superRankProc, &pm_gPlayerStatus.playerData.partners[partnerOrder[i + 1]]);
        superRanks[i]->tooltip = strSuperRank;

        // ultra tex
        ultraRanks[i] =
            menuAddSwitch(menu, partnerX + 3, partnerY, texRank, 0, 0, 0xFFFFFF, texRank, 0, 1, 0xFFFFFF, scale, FALSE,
                          ultraRankProc, &pm_gPlayerStatus.playerData.partners[partnerOrder[i + 1]]);
        ultraRanks[i]->tooltip = strUltraRank;
    }
    menuItemAddChainLink(activeItem, partners[0], MENU_NAVIGATE_DOWN);
    menuItemAddChainLink(partners[0], activeItem, MENU_NAVIGATE_UP);
    menuItemCreateChain(partners, 8, MENU_NAVIGATE_DOWN, FALSE, FALSE);
    menuItemCreateChain(superRanks, 8, MENU_NAVIGATE_DOWN, FALSE, FALSE);
    menuItemCreateChain(ultraRanks, 8, MENU_NAVIGATE_DOWN, FALSE, FALSE);
}

static void createStarSpiritMenu(struct Menu *menu) {
    menu->selector = menuAddSubmenu(menu, 0, 0, NULL, "return");
    menuAddTooltip(menu, 8, 0, fp.mainMenu, 0xC0C0C0);
    const s32 rowWidth = 4;
    const s32 baseX = 1;
    const s32 baseY = 2;
    const s32 spacingX = 4;
    const s32 spacingY = 5;
    const f32 scale = 0.7f;

    struct MenuItem *starSpirits[8];
    struct GfxTexture *texStarSpirits = resourceGet(RES_PMICON_STAR_SPIRITS);

    s32 ssX;
    s32 ssY;
    for (s32 i = 0; i < 7; i++) {
        ssX = baseX + (i % rowWidth) * spacingX;
        ssY = baseY + (i / rowWidth) * spacingY;
        starSpirits[i] = menuAddSwitch(menu, ssX, ssY, texStarSpirits, i, 0, 0xFFFFFF, texStarSpirits, i, 1, 0xFFFFFF,
                                       scale, FALSE, starSpiritSwitchProc, (void *)i + 1);
        starSpirits[i]->tooltip = strStarSpiritNames[i];
    }
    ssX = baseX + (7 % rowWidth) * spacingX;
    ssY = baseY + (7 / rowWidth) * spacingY;
    struct GfxTexture *beamTextures[] = {texStarSpirits, texStarSpirits, texStarSpirits};
    s32 beamTiles[] = {7, 7, 8};
    s8 beamPalettes[] = {1, 0, 0};
    u32 beamColors[] = {0xFFFFFF, 0xFFFFFF, 0xFFFFFF};
    struct MenuItem *item = menuAddCycle(menu, ssX, ssY, 3, beamTextures, beamTiles, beamPalettes, beamColors, scale,
                                         FALSE, menuByteCycleProc, &pm_gPlayerStatus.playerData.starBeamLevel);
    item->tooltip = strStarPeachBeam;
    menuItemCreateChain(starSpirits, 8, MENU_NAVIGATE_RIGHT, FALSE, FALSE);
    menuItemCreateChain(starSpirits, 8, MENU_NAVIGATE_LEFT, FALSE, TRUE);
}

static void createPeachMenu(struct Menu *menu) {
    s32 y = 0;
    menu->selector = menuAddSubmenu(menu, 0, y++, NULL, "return");

    menuAddStatic(menu, 0, y, "peach", 0xC0C0C0);
    menuAddCheckbox(menu, 12, y++, peachFlagProc, (void *)0);

    menuAddStatic(menu, 0, y, "transformed", 0xC0C0C0);
    menuAddCheckbox(menu, 12, y++, peachFlagProc, (void *)1);

    menuAddStatic(menu, 0, y, "parasol", 0xC0C0C0);
    menuAddCheckbox(menu, 12, y++, peachFlagProc, (void *)2);

    menuAddStatic(menu, 0, y, "disguise", 0xC0C0C0);
    menuAddOption(menu, 12, y++,
                  "none\0"
                  "koopatrol\0"
                  "hammer bro\0"
                  "clubba\0",
                  menuByteOptionmodProc, &pm_gGameStatus.peachDisguise);
}

static void createMerleeMenu(struct Menu *menu) {
    s32 yValue = 0;
    menu->selector = menuAddSubmenu(menu, 0, yValue++, NULL, "return");

    menuAddStatic(menu, 0, yValue, "spell type", 0xC0C0C0);
    menuAddOption(menu, 16, yValue++,
                  "none\0"
                  "+3 ATK\0"
                  "+3 DEF\0"
                  "EXP x2\0"
                  "Coins x2\0",
                  menuByteOptionmodProc, &pm_gPlayerStatus.playerData.merleeSpellType);

    menuAddStatic(menu, 0, yValue, "casts remaining", 0xC0C0C0);
    menuAddIntinput(menu, 16, yValue++, 10, 2, menuByteModProc, &pm_gPlayerStatus.playerData.merleeCastsRemaining);

    menuAddStatic(menu, 0, yValue, "turns remaining", 0xC0C0C0);
    menuAddIntinput(menu, 16, yValue++, 10, 3, menuHalfwordModProc, &pm_gPlayerStatus.playerData.merleeTurnCount);
}

struct Menu *createPlayerMenu(void) {
    static struct Menu menu;

    static struct Menu stats;

    static struct Menu gear;
    static struct Menu badges;

    static struct Menu partners;
    static struct Menu starSpirits;

    static struct Menu items;
    static struct Menu regularItems;
    static struct Menu keyItems;
    static struct Menu storedItems;

    static struct Menu misc;
    static struct Menu peach;
    static struct Menu merlee;

    /* initialize menu */
    menuInit(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    menuInit(&stats, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    menuInit(&gear, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&badges, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    menuInit(&partners, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&starSpirits, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    menuInit(&items, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&regularItems, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&keyItems, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&storedItems, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    menuInit(&misc, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&peach, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuInit(&merlee, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    /*build player menu*/
    struct GfxTexture **itemTextures = getItemTextureList();
    gfxAddGrayscalePalette(itemTextures[ITEM_KOOPA_FORTRESS_KEY], 0);
    createItemSelectionMenu(itemTextureList);

    s32 y = 0;
    menu.selector = menuAddSubmenu(&menu, 0, y++, NULL, "return");
    menuAddSubmenu(&menu, 0, y++, &stats, "stats");
    menuAddSubmenu(&menu, 0, y++, &gear, "gear");
    menuAddSubmenu(&menu, 0, y++, &partners, "partners");
    menuAddSubmenu(&menu, 0, y++, &badges, "badges");
    menuAddSubmenu(&menu, 0, y++, &items, "items");
    menuAddSubmenu(&menu, 0, y++, &starSpirits, "star spirits");
    menuAddSubmenu(&menu, 0, y++, &misc, "misc");
    createStatsMenu(&stats);
    createGearMenu(&gear);
    createPartyMenu(&partners);
    createBadgesMenu(&badges, itemTextureList);
    createStarSpiritMenu(&starSpirits);

    y = 0;
    items.selector = menuAddSubmenu(&items, 0, y++, NULL, "return");
    menuAddSubmenu(&items, 0, y++, &regularItems, "regular items");
    menuAddSubmenu(&items, 0, y++, &keyItems, "key items");
    menuAddSubmenu(&items, 0, y++, &storedItems, "stored items");
    createNormalItemsMenu(&regularItems, itemTextureList);
    createKeyItemsMenu(&keyItems, itemTextureList);
    createStoredItemsMenu(&storedItems, itemTextureList);

    y = 0;
    misc.selector = menuAddSubmenu(&misc, 0, y++, NULL, "return");
    menuAddSubmenu(&misc, 0, y++, &peach, "princess peach");
    menuAddSubmenu(&misc, 0, y++, &merlee, "merlee");
    createPeachMenu(&peach);
    createMerleeMenu(&merlee);

    return &menu;
}
