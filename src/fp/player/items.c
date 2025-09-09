#include "items.h"
#include "common.h"
#include "fp.h"
#include "menu/menu_item_button.h"
#include "sys/resource.h"
#include <stdio.h>
#include <stdlib.h>

const char *strItemNames[] = {
    "empty",
    "Jump",
    "Spin Jump",
    "Tornado Jump",
    "Hammer",
    "Super Hammer",
    "Ultra Hammer",
    "Lucky Star",
    "Map",
    "Big Map",
    "First-Degree Card",
    "Second-Degree Card",
    "Third-Degree Card",
    "Fourth-Degree Card",
    "Diploma",
    "Ultra Stone",
    "Fortress Key",
    "Ruins Key",
    "Pulse Stone",
    "Tubba's Castle Key",
    "Palace Key",
    "Lunar Stone",
    "Pyramid Stone",
    "Diamond Stone",
    "Golden Jar",
    "Kooper's Shell",
    "Bowser's Castle Key",
    "Forest Pass",
    "Weight",
    "Boo's Portrait",
    "Crystal Berry",
    "Mystical Key",
    "Storeroom Key",
    "Toy Train",
    "Record",
    "Frying Pan",
    "Dictionary",
    "Mystery Note",
    "Suspicious Note",
    "Crystal Ball",
    "Screwdriver",
    "Cookbook",
    "Jade Raven",
    "Magical Seed 1",
    "Magical Seed 2",
    "Magical Seed 3",
    "Magical Seed 4",
    "Toad Doll",
    "Calculator",
    "Bucket",
    "Scarf",
    "Red Key",
    "Blue Key",
    "Unused Letter 1",
    "Letter to Merlon",
    "Letter to Goompa",
    "Letter to Mort T.",
    "Letter to Russ T.",
    "Letter to Mayor Penguin",
    "Letter to Merlow",
    "Letter to Fice T.",
    "Letter to Nomadimouse",
    "Letter to Minh T.",
    "Unused Letter 2",
    "Letter to Goompapa 1",
    "Letter to Igor",
    "Unused Letter 3",
    "Unused Letter 4",
    "Unused Letter 5",
    "Letter to Franky",
    "Letter to Muss T.",
    "Letter to Koover 1",
    "Letter to Fishmael",
    "Letter to Koover 2",
    "Letter to Mr. E.",
    "Letter to Miss T.",
    "Letter to Little Mouser",
    "Unused Letter 6",
    "Letter to Dane T. 1",
    "Letter to Red Yoshi Kid",
    "Letter to Dane T. 2",
    "Letter to Frost T.",
    "Letter to Goompapa 2",
    "Artifact",
    "Letter to Kolorado",
    "Unused Letter 7",
    "Dolly",
    "Water Stone",
    "Magical Bean",
    "Fertile Soil",
    "Miracle Water",
    "Volcano Vase",
    "Tape",
    "Baking Sugar",
    "Baking Salt",
    "Baking Egg 1",
    "Baking Cream",
    "Baking Strawberry",
    "Baking Butter",
    "Baking Cleanser",
    "Baking Water",
    "Baking Flour",
    "Baking Milk",
    "Lyrics",
    "Melody",
    "Mailbag",
    "Peach's Castle Key",
    "Odd Key",
    "Star Stone",
    "Sneaky Parasol",
    "Koopa Legends",
    "Luigi's Autograph",
    "Empty Wallet",
    "Merluvlee's Autograph",
    "Koopa Shell",
    "Old Photo",
    "Glasses",
    "Unused Old Photo",
    "Package",
    "Red Jar",
    "Peach's Castle Key 2",
    "Warehouse Key",
    "Prison Key",
    "Silver Credit",
    "Gold Credit",
    "Prison Key 2",
    "Prison Key 3",
    "Prison Key 4",
    "Fire Flower",
    "Snowman Doll",
    "Thunder Rage",
    "Shooting Star",
    "Thunder Bolt",
    "Pebble",
    "Dusty Hammer",
    "Insecticide Herb",
    "Stone Cap",
    "Tasty Tonic",
    "Mushroom",
    "Volt Shroom",
    "Super Shroom",
    "Dried Shroom",
    "Ultra Shroom",
    "Sleepy Sheep",
    "POW Block",
    "Hustle Drink",
    "Stop Watch",
    "Whacka's Bump",
    "Apple",
    "Life Shroom",
    "Mystery?",
    "Repel Gel",
    "Fright Jar",
    "Please Come Back",
    "Dizzy Dial",
    "Super Soda",
    "Lemon",
    "Lime",
    "Blue Berry",
    "Red Berry",
    "Yellow Berry",
    "Bubble Berry",
    "Jammin' Jelly",
    "Maple Syrup",
    "Honey Syrup",
    "Goomnut",
    "Koopa Leaf",
    "Dried Pasta",
    "Dried Fruit",
    "Strange Leaf",
    "Cake Mix",
    "Egg",
    "Coconut",
    "Melon",
    "Stinky Herb",
    "Iced Potato",
    "Spicy Soup",
    "Apple Pie",
    "Honey Ultra",
    "Maple Ultra",
    "Jelly Ultra",
    "Koopasta",
    "Fried Shroom",
    "Shroom Cake",
    "Shroom Steak",
    "Hot Shroom",
    "Sweet Shroom",
    "Yummy Meal",
    "Healthy Juice",
    "Bland Meal",
    "Deluxe Feast",
    "Special Shake",
    "Big Cookie",
    "Cake",
    "Mistake",
    "Koopa Tea",
    "Honey Super",
    "Maple Super",
    "Jelly Super",
    "Spaghetti",
    "Egg Missile",
    "Fried Egg",
    "Honey Shroom",
    "Honey Candy",
    "Electro Pop",
    "Fire Pop",
    "Lime Candy",
    "Coco Pop",
    "Lemon Candy",
    "Jelly Pop",
    "Strange Cake",
    "Kooky Cookie",
    "Frozen Fries",
    "Potato Salad",
    "Nutty Cake",
    "Maple Shroom",
    "Boiled Egg",
    "Yoshi Cookie",
    "Jelly Shroom",
    "Unused Jelly Shroom 1",
    "Unused Jelly Shroom 2",
    "Unused Jelly Shroom 3",
    "Unused Jelly Shroom 4",
    "Unused Jelly Shroom 5",
    "Spin Smash",
    "Multibounce",
    "Power Plus 1",
    "Dodge Master",
    "Power Bounce",
    "Spike Shield",
    "First Attack",
    "HP Plus 1",
    "Quake Hammer",
    "Double Dip",
    "Mystery Scroll",
    "Sleep Stomp",
    "Fire Shield",
    "Quick Change",
    "D-Down Pound",
    "Dizzy Stomp",
    "Smash Charge 0",
    "Pretty Lucky",
    "Feeling Fine",
    "Attack FX A",
    "All or Nothing",
    "HP Drain",
    "Jump Charge 0",
    "Slow Go",
    "FP Plus 1",
    "Mega Rush",
    "Ice Power",
    "Defend Plus",
    "Pay-Off",
    "Money Money",
    "Chill Out",
    "Happy Heart 1",
    "Zap Tap",
    "Berserker",
    "Right On",
    "Runaway Pay",
    "Refund",
    "Flower Saver 1",
    "Triple Dip",
    "Hammer Throw",
    "Mega Quake",
    "Smash Charge",
    "Jump Charge",
    "S. Smash Chg.",
    "S. Jump Chg.",
    "Power Rush",
    "Auto Jump",
    "Auto Smash",
    "Crazy Heart",
    "Last Stand",
    "Close Call",
    "P-Up D-Down",
    "Lucky Day",
    "Mega HP Drain",
    "P-Down D-Up",
    "Power Quake",
    "Auto Multibounce",
    "Flower Fanatic",
    "Heart Finder",
    "Flower Finder",
    "Spin Attack",
    "Dizzy Attack",
    "I Spy",
    "Speedy Spin",
    "Bump Attack",
    "Power Jump",
    "Super Jump",
    "Mega Jump",
    "Power Smash",
    "Super Smash",
    "Mega Smash",
    "Power Smash 2",
    "Power Smash 3",
    "Deep Focus 1",
    "Super Focus",
    "Shrink Smash",
    "Shell Crack",
    "Kaiden",
    "D-Down Jump",
    "Shrink Stomp",
    "Damage Dodge 1",
    "Earthquake Jump",
    "Deep Focus 2",
    "Deep Focus 3",
    "HP Plus 2",
    "FP Plus 2",
    "Happy Heart 2",
    "Happy Heart 3",
    "Flower Saver 2",
    "Flower Saver 3",
    "Damage Dodge 2",
    "Damage Dodge 3",
    "Power Plus 2",
    "Power Plus 3",
    "Defend Plus 2",
    "Defend Plus 3",
    "Happy Flower 1",
    "Happy Flower 2",
    "Happy Flower 3",
    "Group Focus",
    "Peekaboo",
    "Attack FX D",
    "Attack FX B",
    "Attack FX E",
    "Attack FX C",
    "Attack FX F 1",
    "HP Plus 3",
    "HP Plus 4",
    "HP Plus 5",
    "FP Plus 3",
    "FP Plus 4",
    "FP Plus 5",
    "Healthy Healthy",
    "Attack FX F 2",
    "Attack FX F 3",
    "Attack FX F 4",
    "Attack FX F 5",
    "Partner Attack",
    "Heart",
    "Coin",
    "Heart Piece",
    "Star Point",
    "Full Heal",
    "Flower",
    "Star Piece",
    "Present",
    "Complete Cake",
    "Bare Cake",
    "Empty Cake Pan",
    "Full Cake Pan",
    "Empty Mixing Bowl",
    "Full Mixing Bowl",
    "Cake With Icing",
    "Cake With Berries",
    "Hammer 1 Icon",
    "Hammer 2 Icon",
    "Hammer 3 Icon",
    "Boots 1 Icon",
    "Boots 2 Icon",
    "Boots 3 Icon",
    "Items Icon",
};

static const s16 itemsNormalFood[] = {
    0x8A, 0x8B, 0x8C, 0x8E, 0x95, 0x8D, 0xA4, 0xA3, 0xA2, 0x89, 0x93, 0x94, 0x9B, 0x9C, 0x9D,
    0x9E, 0x9F, 0xA0, 0xA1, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAD, 0xAE, 0xAF,
};

static const s16 itemsNormalOther[] = {
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x88, 0x8F, 0x90, 0x92, 0x96, 0x97, 0x98, 0x9A, 0xAC,
};

static const s16 itemsNormalTayce[] = {
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE,
    0xBF, 0xC0, 0xC1, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE,
    0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xC2,
};

static const s16 itemsNormalUnused[] = {
    0x87, 0x91, 0x99, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
};

static const s16 itemsKeyRequired[] = {
    0x7,  0x19, 0x10, 0x12, 0x11, 0x16, 0x17, 0x15, 0x22, 0x1C, 0x1D, 0x13, 0x1F, 0x20,
    0x21, 0x23, 0x24, 0x25, 0x2A, 0xF,  0x5B, 0x2B, 0x2C, 0x2D, 0x2E, 0x1E, 0x57, 0x58,
    0x59, 0x5A, 0x79, 0x32, 0x31, 0x6C, 0x34, 0x33, 0x14, 0x1A, 0x6A, 0x6D,
};

static const s16 itemsKeyOther[] = {
    0x56, 0x67, 0x68, 0x53, 0x30, 0x69, 0x29, 0x6B, 0x7A, 0x6E, 0x27, 0x5C, 0x6F,
    0x70, 0x71, 0x73, 0x74, 0x76, 0x77, 0x7B, 0x7C, 0xA,  0xB,  0xC,  0xD,  0xE,
};

static const s16 itemsKeyLetters[] = {
    0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x40, 0x41, 0x45, 0x46,
    0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x54,
};

static const s16 itemsKeyUnused[] = {
    0x8,  0x9,  0x18, 0x1B, 0x26, 0x28, 0x2F, 0x35, 0x3F, 0x42,
    0x43, 0x44, 0x4D, 0x55, 0x72, 0x75, 0x78, 0x7D, 0x7E, 0x7F,
};

static const s16 itemsBadges[] = {
    0x121, 0x123, 0x0E1, 0x10A, 0x10C, 0x12F, 0x0EB, 0x0EF, 0x12E, 0x0E4, 0x124, 0x126, 0x109, 0x10B, 0x0E0, 0x0E8,
    0x117, 0x108, 0x107, 0x0EE, 0x0E9, 0x106, 0x143, 0x0ED, 0x144, 0x0E3, 0x0FF, 0x136, 0x140, 0x141, 0x129, 0x132,
    0x133, 0x0E7, 0x134, 0x14A, 0x0F8, 0x135, 0x14D, 0x105, 0x138, 0x0E2, 0x13C, 0x0FB, 0x130, 0x13A, 0x116, 0x113,
    0x0F5, 0x0F4, 0x0F9, 0x111, 0x10D, 0x112, 0x0F1, 0x114, 0x0FA, 0x0EC, 0x0E5, 0x0F2, 0x100, 0x11A, 0x11B, 0x103,
    0x104, 0x0FC, 0x0FD, 0x11E, 0x0FE, 0x11F, 0x11D, 0x11C, 0x0E6, 0x120, 0x0F7, 0x0F3, 0x146, 0x148, 0x145, 0x147,
};

static const s16 itemsBadgesUnused[] = {
    0xF0,  0xF6,  0x101, 0x102, 0x10E, 0x10F, 0x110, 0x115, 0x118, 0x119, 0x122, 0x125, 0x12A,
    0x12B, 0x12D, 0x131, 0x137, 0x139, 0x13B, 0x13D, 0x13E, 0x13F, 0x142, 0x14B, 0x14C, 0x14E,
    0x14F, 0x150, 0x149, 0x151, 0x152, 0x153, 0x154, 0xEA,  0x127, 0x128, 0x12C,
};

static const s16 itemsMisc[] = {
    0x1,   0x2,   0x3,   0x4,   0x5,   0x6,   0x5D,  0x5E,  0x5F,  0x60,  0x61,  0x62,  0x63,  0x64,
    0x65,  0x66,  0x155, 0x156, 0x157, 0x158, 0x159, 0x15A, 0x15B, 0x15C, 0x15D, 0x15E, 0x15F, 0x160,
    0x161, 0x162, 0x163, 0x164, 0x165, 0x166, 0x167, 0x168, 0x169, 0x16A, 0x16B, 0x16C,
};

static struct Menu itemSelectionMenu;
static struct MenuItem *itemSelectionMenuReturn;
static struct MenuItem *itemSelectionMenuTab;

static s16 *itemSlotToUpdate;

static s32 badgeProcSwitch(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    u32 badgeId = (u32)data;
    s16 *badgeList = pm_gPlayerData.badges;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        for (u16 i = 0; i < 128; i++) {
            if (badgeList[i] == 0) {
                badgeList[i] = badgeId;
                break;
            }
        }
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        for (u16 i = 0; i < 128; i++) {
            if (badgeList[i] == badgeId) {
                badgeList[i] = 0;
            }
        }
        for (u16 i = 0; i < 64; i++) {
            if (pm_gPlayerData.equippedBadges[i] == badgeId) {
                pm_gPlayerData.equippedBadges[i] = 0;
            }
        }
    } else if (reason == MENU_CALLBACK_THINK) {
        bool hasBadge = FALSE;
        for (u16 i = 0; i < 128; i++) {
            if (badgeList[i] == badgeId) {
                hasBadge = TRUE;
                break;
            }
        }
        menuSwitchSet(item, hasBadge);
    }
    return 0;
}

static void itemListButtonProc(struct MenuItem *item, void *data) {
    itemSlotToUpdate = menuItemButtonGetSlot(item);
    enum ItemType itemType = menuItemButtonGetType(item);
    s32 lastPage = menuTabGetCurrentTab(itemSelectionMenuTab);
    menuEnter(fp.mainMenu, &itemSelectionMenu);
    switch (itemType) {
        case ITEM_TYPE_NORMAL:
        case ITEM_TYPE_STORED:
            if (lastPage != 0 && lastPage != 1) {
                menuSelect(&itemSelectionMenu, itemSelectionMenuReturn);
                menuTabGoto(itemSelectionMenuTab, 0);
            }
            break;
        case ITEM_TYPE_KEY:
            if (lastPage != 2 && lastPage != 3) {
                menuSelect(&itemSelectionMenu, itemSelectionMenuReturn);
                menuTabGoto(itemSelectionMenuTab, 2);
            }
            break;
        case ITEM_TYPE_BADGE:
            if (lastPage != 4 && lastPage != 5) {
                menuSelect(&itemSelectionMenu, itemSelectionMenuReturn);
                menuTabGoto(itemSelectionMenuTab, 4);
            }
            break;
    }
    return;
}

static void itemSelectionButtonProc(struct MenuItem *item, void *data) {
    s16 itemId = (u32)data;
    *itemSlotToUpdate = itemId;
    menuReturn(&itemSelectionMenu);
    return;
}

static void itemDeleteButtonProc(struct MenuItem *item, void *data) {
    *itemSlotToUpdate = 0;
    menuReturn(&itemSelectionMenu);
    return;
}

static void createItemSelectionPage(struct Menu *page, const char *title, struct GfxTexture *itemTextureList[],
                                    const s16 *itemLists[], const s16 itemListSizes[], s16 itemListCount) {
    menuInit(page, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menuAddStatic(page, 12, 0, title, 0xC0C0C0);
    const s32 rowSize = 12;
    const s32 baseX = 1;
    const s32 baseY = 5;
    const s32 spacing = 2;
    const f32 scale = 0.6f;

    s32 totalItemCount = 0;
    for (s32 i = 0; i < itemListCount; i++) {
        totalItemCount += itemListSizes[i];
    }
    struct MenuItem **itemButtons = malloc(sizeof(*itemButtons) * totalItemCount);
    s32 itemButtonIdx = 0;
    s32 startY = baseY;
    for (s32 iLists = 0; iLists < itemListCount; iLists++) {
        s32 finalItemY = baseY;
        for (s32 iPos = 0; iPos < itemListSizes[iLists]; iPos++) {
            const s32 itemX = baseX + (iPos % rowSize) * spacing;
            const s32 itemY = startY + (iPos / rowSize) * spacing;
            finalItemY = itemY;

            u16 itemId = itemLists[iLists][iPos];
            struct GfxTexture *icon = itemTextureList[itemId];
            struct MenuItem *button = menuAddButtonIcon(page, itemX, itemY, icon, 0, 0, 0xFFFFFF, scale,
                                                        itemSelectionButtonProc, (void *)(s32)itemId);
            button->tooltip = strItemNames[itemId];
            itemButtons[itemButtonIdx++] = button;
        }
        startY = finalItemY + spacing + 1;
    }
    menuItemCreateChain(itemButtons, totalItemCount, MENU_NAVIGATE_RIGHT, TRUE, FALSE);
    menuItemCreateChain(itemButtons, totalItemCount, MENU_NAVIGATE_LEFT, TRUE, TRUE);
    free(itemButtons);
}

void createItemSelectionMenu(struct GfxTexture *itemTextureList[]) {
    const s32 pageCount = 7;

    struct Menu *menu = &itemSelectionMenu;
    menuInit(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    itemSelectionMenuReturn = menu->selector = menuAddSubmenu(menu, 0, 0, NULL, "return");
    menuAddButton(menu, 0, 2, "delete", itemDeleteButtonProc, NULL);
    menuAddTooltip(menu, 8, 2, fp.mainMenu, 0xC0C0C0);

    struct Menu *pages = malloc(sizeof(*pages) * pageCount);
    itemSelectionMenuTab = menuAddTab(menu, 0, 0, pages, pageCount);

    s32 pageIndex = 0;

    const s16 *listItems1[] = {itemsNormalFood, itemsNormalOther};
    const s16 *listItems2[] = {itemsNormalTayce, itemsNormalUnused};
    const s16 *listKey1[] = {itemsKeyRequired, itemsKeyOther};
    const s16 *listKey2[] = {itemsKeyLetters, itemsKeyUnused};
    const s16 *listBadges1[] = {itemsBadges};
    const s16 *listBadges2[] = {itemsBadgesUnused};
    const s16 *listMisc[] = {itemsMisc};

    const s16 sizesItems1[] = {ARRAY_LENGTH(itemsNormalFood), ARRAY_LENGTH(itemsNormalOther)};
    const s16 sizesItems2[] = {ARRAY_LENGTH(itemsNormalTayce), ARRAY_LENGTH(itemsNormalUnused)};
    const s16 sizesKey1[] = {ARRAY_LENGTH(itemsKeyRequired), ARRAY_LENGTH(itemsKeyOther)};
    const s16 sizesKey2[] = {ARRAY_LENGTH(itemsKeyLetters), ARRAY_LENGTH(itemsKeyUnused)};
    const s16 sizesBadges1[] = {ARRAY_LENGTH(itemsBadges)};
    const s16 sizesBadges2[] = {ARRAY_LENGTH(itemsBadgesUnused)};
    const s16 sizesMisc[] = {ARRAY_LENGTH(itemsMisc)};

    createItemSelectionPage(&pages[pageIndex++], "items (1/2)", itemTextureList, listItems1, sizesItems1,
                            ARRAY_LENGTH(listItems1));
    createItemSelectionPage(&pages[pageIndex++], "items (2/2)", itemTextureList, listItems2, sizesItems2,
                            ARRAY_LENGTH(listItems2));
    createItemSelectionPage(&pages[pageIndex++], "key items (1/2)", itemTextureList, listKey1, sizesKey1,
                            ARRAY_LENGTH(listKey1));
    createItemSelectionPage(&pages[pageIndex++], "key items (2/2)", itemTextureList, listKey2, sizesKey2,
                            ARRAY_LENGTH(listKey2));
    createItemSelectionPage(&pages[pageIndex++], "badges (1/2)", itemTextureList, listBadges1, sizesBadges1,
                            ARRAY_LENGTH(listBadges1));
    createItemSelectionPage(&pages[pageIndex++], "badges (2/2)", itemTextureList, listBadges2, sizesBadges2,
                            ARRAY_LENGTH(listBadges2));
    createItemSelectionPage(&pages[pageIndex++], "misc (1/1)", itemTextureList, listMisc, sizesMisc,
                            ARRAY_LENGTH(listMisc));

    menuTabGoto(itemSelectionMenuTab, 0);
    menuAddButton(menu, 8, 0, "<", menuTabPrevProc, itemSelectionMenuTab);
    menuAddButton(menu, 10, 0, ">", menuTabNextProc, itemSelectionMenuTab);
}

static void createItemsMenu(struct Menu *menu, enum ItemType itemType, struct GfxTexture *itemTextureList[],
                            f32 iconScale, s32 itemCount, s32 rowSize, s32 pageSize, s32 spacingX, s32 spacingY) {
    menu->selector = menuAddSubmenu(menu, 0, 0, NULL, "return");
    const s32 baseX = 2;
    const s32 baseY = 3;
    const s32 pageCount = (itemCount + (pageSize - 1)) / pageSize;
    const s32 tooltipX = pageCount > 1 ? 18 : 8;

    menuAddTooltip(menu, tooltipX, 0, fp.mainMenu, 0xC0C0C0);

    struct Menu *pages = malloc(sizeof(*pages) * pageCount);
    struct MenuItem *tab = menuAddTab(menu, 0, 0, pages, pageCount);
    for (s32 iPage = 0; iPage < pageCount; iPage++) {
        struct MenuItem *itemButtons[pageSize];
        struct Menu *page = &pages[iPage];
        menuInit(page, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
        if (pageCount > 1) {
            char buffer[8];
            snprintf(buffer, sizeof(buffer), "(%ld/%ld)", iPage + 1, pageCount);
            menuAddStatic(page, 12, 0, buffer, 0xC0C0C0);
        }

        for (s32 iPos = 0; iPos < pageSize; iPos++) {
            const s32 itemX = baseX + (iPos % rowSize) * spacingX;
            const s32 itemY = baseY + (iPos / rowSize) * spacingY;
            const s32 itemIdx = (iPage * pageSize) + iPos;

            itemButtons[iPos] = menuAddItemButton(page, itemX, itemY, strItemNames, itemTextureList, itemType, itemIdx,
                                                  iconScale, itemListButtonProc, menu);
        }
        menuItemCreateChain(itemButtons, pageSize, MENU_NAVIGATE_RIGHT, TRUE, FALSE);
        menuItemCreateChain(itemButtons, pageSize, MENU_NAVIGATE_LEFT, TRUE, TRUE);
    }
    menuTabGoto(tab, 0);
    if (pageCount > 1) {
        menuAddButton(menu, 8, 0, "<", menuTabPrevProc, tab);
        menuAddButton(menu, 10, 0, ">", menuTabNextProc, tab);
    }
}

void createNormalItemsMenu(struct Menu *menu, struct GfxTexture *itemTextureList[]) {
    createItemsMenu(menu, ITEM_TYPE_NORMAL, itemTextureList, 0.8f, 10, 5, 10, 4, 4);
}

void createKeyItemsMenu(struct Menu *menu, struct GfxTexture *itemTextureList[]) {
    createItemsMenu(menu, ITEM_TYPE_KEY, itemTextureList, 0.7f, 32, 8, 32, 3, 3);
}

void createStoredItemsMenu(struct Menu *menu, struct GfxTexture *itemTextureList[]) {
    createItemsMenu(menu, ITEM_TYPE_STORED, itemTextureList, 0.7f, 32, 8, 32, 3, 3);
}

void createBadgesMenu(struct Menu *menu, struct GfxTexture *itemTextureList[]) {
    struct MenuItem *returnItem = menu->selector = menuAddSubmenu(menu, 0, 0, NULL, "return");
    menuAddTooltip(menu, 8, 0, fp.mainMenu, 0xC0C0C0);
    const u8 baseX = 1;
    const u8 baseY = 2;
    const u8 rowWidth = 10;
    const u8 spacingX = 2;
    const u8 spacingY = 2;
    struct MenuItem *badgeItems[80];

    for (s32 i = 0; i < 80; i++) {
        u8 badgeX = baseX + (i % rowWidth) * spacingX;
        u8 badgeY = baseY + (i / rowWidth) * spacingY;
        s32 itemId = itemsBadges[i];
        struct GfxTexture *badgeIcon = resourceLoadPmiconItem(itemId, FALSE);
        badgeItems[i] = menuAddSwitch(menu, badgeX, badgeY, badgeIcon, 0, 0, 0xFFFFFF, badgeIcon, 0, 1, 0xFFFFFF, 0.6f,
                                      FALSE, badgeProcSwitch, (void *)itemId);
        badgeItems[i]->tooltip = strItemNames[itemId];
    }
    menuItemAddChainLink(returnItem, badgeItems[0], MENU_NAVIGATE_DOWN);
    menuItemAddChainLink(badgeItems[0], returnItem, MENU_NAVIGATE_UP);
    menuItemCreateChain(badgeItems, 80, MENU_NAVIGATE_RIGHT, TRUE, FALSE);
    menuItemCreateChain(badgeItems, 80, MENU_NAVIGATE_LEFT, TRUE, TRUE);

    static struct Menu fullBadgeList;
    menuInit(&fullBadgeList, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    struct MenuItem *fullListItem = menuAddSubmenu(menu, 0, 18, &fullBadgeList, "full list");
    createItemsMenu(&fullBadgeList, ITEM_TYPE_BADGE, itemTextureList, 0.7f, 128, 8, 32, 3, 3);

    menuItemAddChainLink(fullListItem, badgeItems[70], MENU_NAVIGATE_UP);
    menuItemAddChainLink(badgeItems[70], fullListItem, MENU_NAVIGATE_DOWN);
}
