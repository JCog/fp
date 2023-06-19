#include "adex.h"
#include "files.h"
#include "fp.h"
#include "mem.h"
#include "menu/menu.h"
#include "sys/input.h"
#include "sys/resource.h"
#include "sys/settings.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector/vector.h>

struct ItemData {
    struct Menu *menuRelease;
    struct Menu *imenu;
    struct vector members;
    struct MenuItem *addButton;
    struct MenuItem *importButton;
    struct MenuItem *visibilityCheckbox;
};

struct MemberData {
    struct ItemData *data;
    s32 index;
    struct MenuItem *member;
    struct MenuItem *anchorButton;
    struct MenuItem *positioning;
    struct MenuItem *userwatch;
    bool anchored;
    s32 anchorAnimState;
    s32 x;
    s32 y;
    bool positionSet;
};

static struct GfxTexture *listIcons = NULL;
static struct GfxTexture *wrench = NULL;

static struct MemberData *getMember(struct ItemData *data, s32 index) {
    if (index < 0 || index >= data->members.size) {
        return NULL;
    }
    struct MemberData **memberData = vector_at(&data->members, index);
    return *memberData;
}

static void releaseMember(struct MemberData *memberData) {
    if (!memberData->anchored) {
        return;
    }
    memberData->anchored = FALSE;
    menuItemEnable(memberData->positioning);
    struct MenuItem *watch = menuUserwatchWatch(memberData->userwatch);
    if (!memberData->positionSet) {
        memberData->x = menuItemScreenX(watch);
        memberData->y = menuItemScreenY(watch);
    }
    watch->x = 0;
    watch->y = 0;
    watch->pxoffset = memberData->x;
    watch->pyoffset = memberData->y;
    menuItemTransfer(watch, memberData->data->menuRelease);
}

static void anchorMember(struct MemberData *memberData) {
    if (memberData->anchored) {
        return;
    }
    memberData->anchored = TRUE;
    menuItemDisable(memberData->positioning);
    struct MenuItem *watch = menuUserwatchWatch(memberData->userwatch);
    watch->x = 13;
    watch->y = 0;
    watch->pxoffset = 0;
    watch->pyoffset = 0;
    menuItemTransfer(watch, memberData->userwatch->imenu);
}

static s32 anchorButtonEnterProc(struct MenuItem *item, enum MenuSwitchReason reason) {
    struct MemberData *memberData = item->data;
    memberData->anchorAnimState = 0;
    return 0;
}

static s32 anchorButtonDrawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    struct MemberData *memberData = item->data;
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
    static struct GfxTexture *texture = NULL;
    if (!texture) {
        texture = resourceLoadGrcTexture("anchor");
    }
    s32 cw = menuGetCellWidth(item->owner, TRUE);
    if (memberData->anchorAnimState > 0) {
        ++drawParams->x;
        ++drawParams->y;
    }
    struct GfxSprite sprite = {
        texture,
        (memberData->anchorAnimState > 0) != memberData->anchored ? 0 : 1,
        0,
        drawParams->x + (cw - texture->tileWidth) / 2,
        drawParams->y - (gfxFontXheight(drawParams->font) + texture->tileHeight + 1) / 2,
        1.f,
        1.f,
    };
    gfxSpriteDraw(&sprite);
    if (memberData->anchorAnimState > 0) {
        memberData->anchorAnimState = (memberData->anchorAnimState + 1) % 3;
    }
    return 1;
}

static s32 anchorButtonActivateProc(struct MenuItem *item) {
    struct MemberData *memberData = item->data;
    if (memberData->anchored) {
        releaseMember(memberData);
    } else {
        anchorMember(memberData);
    }
    memberData->anchorAnimState = 1;
    return 1;
}

static void editWatchInMemoryProc(struct MenuItem *item, void *data) {
    struct MemberData *memberData = data;
    struct MenuItem *watch = menuUserwatchWatch(memberData->userwatch);
    memOpenWatch(item->owner, fp.menuMem, menuWatchGetAddress(watch), menuWatchGetType(watch));
}

static s32 positionProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    struct MemberData *memberData = data;
    struct MenuItem *watch = menuUserwatchWatch(memberData->userwatch);
    if (!memberData->positionSet) {
        memberData->positionSet = TRUE;
        memberData->x = watch->pxoffset;
        memberData->y = watch->pyoffset;
    }
    s32 dist = 2;
    if (inputPad() & BUTTON_Z) {
        dist *= 2;
    }
    switch (reason) {
        case MENU_CALLBACK_ACTIVATE: inputReserve(BUTTON_Z); break;
        case MENU_CALLBACK_DEACTIVATE: inputFree(BUTTON_Z); break;
        case MENU_CALLBACK_NAV_UP: memberData->y -= dist; break;
        case MENU_CALLBACK_NAV_DOWN: memberData->y += dist; break;
        case MENU_CALLBACK_NAV_LEFT: memberData->x -= dist; break;
        case MENU_CALLBACK_NAV_RIGHT: memberData->x += dist; break;
        default: break;
    }
    watch->pxoffset = memberData->x;
    watch->pyoffset = memberData->y;
    return 0;
}

static void removeButtonProc(struct MenuItem *item, void *data);
static s32 addMember(struct ItemData *data, u32 address, enum WatchType type, s32 position, bool anchored, s32 x, s32 y,
                     bool positionSet) {
    if (data->members.size >= SETTINGS_WATCHES_MAX || position < 0 || position > data->members.size) {
        return 0;
    }
    ++data->addButton->y;
    ++data->importButton->y;
    for (s32 i = position; i < data->members.size; ++i) {
        struct MemberData *memberData = getMember(data, i);
        ++memberData->index;
        ++memberData->member->y;
    }
    struct Menu *imenu;
    struct MemberData *memberData = malloc(sizeof(*memberData));
    memberData->data = data;
    memberData->index = position;
    memberData->member = menuAddImenu(data->imenu, 0, position, &imenu);
    memberData->anchorButton = menuItemAdd(imenu, 4, 0, NULL, 0xFFFFFF);
    memberData->anchorButton->enterProc = anchorButtonEnterProc;
    memberData->anchorButton->drawProc = anchorButtonDrawProc;
    memberData->anchorButton->activateProc = anchorButtonActivateProc;
    memberData->anchorButton->data = memberData;
    memberData->positioning = menuAddPositioning(imenu, 6, 0, positionProc, memberData);
    memberData->userwatch = menuAddUserwatch(imenu, 8, 0, address, type);
    memberData->anchored = TRUE;
    memberData->anchorAnimState = 0;
    memberData->x = x;
    memberData->y = y;
    memberData->positionSet = TRUE;
    menuAddButtonIcon(imenu, 0, 0, listIcons, 1, 0, 0xFF0000, 1.0f, removeButtonProc, memberData);
    menuAddButtonIcon(imenu, 2, 0, wrench, 0, 0, 0xFFFFFF, 1.0f, editWatchInMemoryProc, memberData);

    if (!settings->bits.watchesVisible) {
        struct MenuItem *watch = menuUserwatchWatch(memberData->userwatch);
        menuItemDisable(watch);
    }

    if (anchored) {
        menuItemDisable(memberData->positioning);
    } else {
        releaseMember(memberData);
    }
    memberData->positionSet = positionSet;
    vector_insert(&data->members, position, 1, &memberData);
    return 1;
}

static s32 removeMember(struct ItemData *data, s32 position) {
    if (position < 0 || position >= data->members.size) {
        return 0;
    }
    menuNavigateTop(data->imenu, MENU_NAVIGATE_DOWN);
    --data->addButton->y;
    --data->importButton->y;
    for (s32 i = position + 1; i < data->members.size; ++i) {
        struct MemberData *memberData = getMember(data, i);
        --memberData->index;
        --memberData->member->y;
    }
    struct MemberData *memberData = getMember(data, position);
    struct MenuItem *watch = menuUserwatchWatch(memberData->userwatch);
    menuItemRemove(watch);
    memberData->anchorButton->data = NULL;
    menuItemRemove(memberData->member);
    vector_erase(&data->members, position, 1);
    free(memberData);
    return 1;
}

static void addButtonProc(struct MenuItem *item, void *data) {
    struct ItemData *itemData = data;
    u32 address = 0x80000000;
    enum WatchType type = WATCH_TYPE_U8;
    if (itemData->members.size > 0) {
        struct MemberData *memberData = getMember(itemData, itemData->members.size - 1);
        struct MenuItem *lastWatch = menuUserwatchWatch(memberData->userwatch);
        address = menuWatchGetAddress(lastWatch);
        type = menuWatchGetType(lastWatch);
    }
    addMember(itemData, address, type, itemData->members.size, TRUE, 0, 0, FALSE);
}

static s32 importCallback(const char *path, void *data);
static void importButtonProc(struct MenuItem *item, void *data) {
    struct ItemData *itemData = data;
    menuGetFile(menuGetTop(itemData->imenu), GETFILE_LOAD, NULL, ".txt", importCallback, itemData);
}

static void removeButtonProc(struct MenuItem *item, void *data) {
    struct MemberData *memberData = data;
    removeMember(memberData->data, memberData->index);
}

static s32 destroyProc(struct MenuItem *item) {
    struct ItemData *data = item->data;
    vector_destroy(&data->members);
    return 0;
}

void watchlistShow(struct MenuItem *item) {
    struct ItemData *data = item->data;
    for (s32 i = 0; i < data->members.size; i++) {
        struct MemberData *memberData = getMember(data, i);
        struct MenuItem *watch = menuUserwatchWatch(memberData->userwatch);
        menuItemEnable(watch);
    }
}

void watchlistHide(struct MenuItem *item) {
    struct ItemData *data = item->data;
    for (s32 i = 0; i < data->members.size; i++) {
        struct MemberData *memberData = getMember(data, i);
        struct MenuItem *watch = menuUserwatchWatch(memberData->userwatch);
        menuItemDisable(watch);
    }
}

static s32 toggleVisibilityProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    struct MenuItem *watchlist = data;
    if (reason == MENU_CALLBACK_CHANGED) {
        settings->bits.watchesVisible = menuCheckboxGet(item);
        if (settings->bits.watchesVisible) {
            watchlistShow(watchlist);
        } else {
            watchlistHide(watchlist);
        }
    } else if (reason == MENU_CALLBACK_THINK) {
        menuCheckboxSet(item, settings->bits.watchesVisible);
    }
    return 0;
}

struct MenuItem *watchlistCreate(struct Menu *menu, struct Menu *menuRelease, s32 x, s32 y) {
    struct Menu *imenu;
    struct MenuItem *item = menuAddImenu(menu, x, y + 3, &imenu);
    struct ItemData *data = malloc(sizeof(*data));

    menuAddButton(menu, x, y, "save settings", fpSaveSettingsProc, NULL);
    menuAddStatic(menu, x, y + 2, "visible", 0xC0C0C0);
    data->visibilityCheckbox = menuAddCheckbox(menu, x + 8, y + 2, toggleVisibilityProc, item);

    data->menuRelease = menuRelease;
    data->imenu = imenu;
    vector_init(&data->members, sizeof(struct MemberData *));
    if (!listIcons) {
        listIcons = resourceLoadGrcTexture("list_icons");
    }
    if (!wrench) {
        wrench = resourceLoadGrcTexture("wrench");
    }
    data->addButton = menuAddButtonIcon(imenu, 0, 0, listIcons, 0, 0, 0x00FF00, 1.0f, addButtonProc, data);

    struct GfxTexture *fileIcons = resourceGet(RES_ICON_FILE);
    data->importButton = menuAddButtonIcon(imenu, 2, 0, fileIcons, 1, 0, 0xFFFFFF, 1.0f, importButtonProc, data);
    item->data = data;
    item->destroyProc = destroyProc;
    return item;
}

s32 watchlistAdd(struct MenuItem *item, u32 address, enum WatchType type) {
    struct ItemData *list = item->data;
    s32 pos = list->members.size;
    if (addMember(list, address, type, pos, TRUE, 0, 0, FALSE)) {
        return pos;
    } else {
        return -1;
    }
}

void watchlistStore(struct MenuItem *item) {
    struct ItemData *data = item->data;
    settings->nWatches = data->members.size;
    for (s32 i = 0; i < data->members.size; ++i) {
        struct MemberData *memberData = getMember(data, i);
        struct MenuItem *watch = menuUserwatchWatch(memberData->userwatch);
        settings->watchAddress[i] = menuWatchGetAddress(watch);
        settings->watchX[i] = memberData->x;
        settings->watchY[i] = memberData->y;
        settings->watchInfo[i].type = menuWatchGetType(watch);
        settings->watchInfo[i].anchored = memberData->anchored;
        settings->watchInfo[i].positionSet = memberData->positionSet;
    }
}

void watchlistFetch(struct MenuItem *item) {
    struct ItemData *data = item->data;
    for (s32 i = data->members.size - 1; i >= 0; --i) {
        removeMember(data, i);
    }
    for (s32 i = 0; i < settings->nWatches; ++i) {
        addMember(data, settings->watchAddress[i], settings->watchInfo[i].type, i, settings->watchInfo[i].anchored,
                  settings->watchX[i], settings->watchY[i], settings->watchInfo[i].positionSet);
    }
}

/*
   import menu
*/

#define WATCHFILE_VIEW_ROWS 16

static struct ItemData *watchfileListData;
static struct vector watchfileEntries;
static struct Menu watchfileMenu;
static struct MenuItem *watchfileReturn;
static struct MenuItem *watchfileItems[WATCHFILE_VIEW_ROWS];
static s32 watchfileScroll;

struct WatchfileEntry {
    char *name;
    enum WatchType type;
    struct Adex adex;
    s32 animState;
};

static const char *watchTypeName[] = {
    "u8", "s8", "x8", "u16", "s16", "x16", "u32", "s32", "x32", "f32",
};

static s32 watchTypeSize[] = {
    1, 1, 1, 2, 2, 2, 4, 4, 4, 4,
};

static void watchfileDestroy(void) {
    for (s32 i = 0; i < watchfileEntries.size; ++i) {
        struct WatchfileEntry *entry = vector_at(&watchfileEntries, i);
        free(entry->name);
        adexDestroy(&entry->adex);
    }
    vector_destroy(&watchfileEntries);
}

static s32 watchfileLeaveProc(struct MenuItem *item, enum MenuSwitchReason reason) {
    if (reason == MENU_SWITCH_RETURN) {
        watchfileDestroy();
    }
    return 0;
}

static s32 entryEnterProc(struct MenuItem *item, enum MenuSwitchReason reason) {
    s32 row = (s32)item->data;
    s32 index = watchfileScroll + row;
    if (index < watchfileEntries.size) {
        struct WatchfileEntry *entry = vector_at(&watchfileEntries, index);
        entry->animState = 0;
    }
    return 0;
}

static s32 entryDrawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    s32 row = (s32)item->data;
    struct WatchfileEntry *entry = vector_at(&watchfileEntries, watchfileScroll + row);
    if (entry->animState > 0) {
        ++drawParams->x;
        ++drawParams->y;
        entry->animState = (entry->animState + 1) % 3;
    }
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
    gfxPrintf(drawParams->font, drawParams->x, drawParams->y, "%s", entry->name);
    return 1;
}

static s32 entryActivateProc(struct MenuItem *item) {
    s32 row = (s32)item->data;
    struct WatchfileEntry *entry = vector_at(&watchfileEntries, watchfileScroll + row);
    entry->animState = 1;
    u32 address;
    enum AdexError e = adexEval(&entry->adex, &address);
    if (!e && (address < 0x80000000 || address >= 0x80800000 || address % watchTypeSize[entry->type] != 0)) {
        e = ADEX_ERROR_ADDRESS;
    }
    if (e) {
        struct Menu *menuTop = menuGetTop(watchfileListData->imenu);
        menuPrompt(menuTop, adexErrorName[e], "return\0", 0, NULL, NULL);
    } else {
        addMember(watchfileListData, address, entry->type, watchfileListData->members.size, TRUE, 0, 0, FALSE);
    }
    return 1;
}

static void scrollUpProc(struct MenuItem *item, void *data) {
    --watchfileScroll;
    if (watchfileScroll < 0) {
        watchfileScroll = 0;
    }
}

static void scrollDownProc(struct MenuItem *item, void *data) {
    ++watchfileScroll;
    s32 nEntries = watchfileEntries.size;
    if (watchfileScroll + WATCHFILE_VIEW_ROWS > nEntries) {
        watchfileScroll = nEntries - WATCHFILE_VIEW_ROWS;
    }
    if (watchfileScroll < 0) {
        watchfileScroll = 0;
    }
}

static void watchfileMenuInit(void) {
    static bool ready = FALSE;
    if (!ready) {
        ready = TRUE;
        struct Menu *menu = &watchfileMenu;
        menuInit(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
        watchfileReturn = menuAddSubmenu(menu, 0, 0, NULL, "return");
        watchfileReturn->leaveProc = watchfileLeaveProc;
        struct GfxTexture *tArrow = resourceGet(RES_ICON_ARROW);
        menuAddButtonIcon(menu, 0, 1, tArrow, 0, 0, 0xFFFFFF, 1.0f, scrollUpProc, NULL);
        menuAddButtonIcon(menu, 0, 1 + WATCHFILE_VIEW_ROWS - 1, tArrow, 1, 0, 0xFFFFFF, 1.0f, scrollDownProc, NULL);
        for (s32 i = 0; i < WATCHFILE_VIEW_ROWS; ++i) {
            struct MenuItem *item = menuItemAdd(menu, 2, 1 + i, NULL, 0xFFFFFF);
            item->data = (void *)i;
            item->enterProc = entryEnterProc;
            item->drawProc = entryDrawProc;
            item->activateProc = entryActivateProc;
            watchfileItems[i] = item;
        }
    }
}

static void watchfileView(struct Menu *menu) {
    /* initialize menus */
    watchfileMenuInit();
    watchfileScroll = 0;
    /* configure menus */
    for (s32 i = 0; i < WATCHFILE_VIEW_ROWS; ++i) {
        if (i < watchfileEntries.size) {
            menuItemEnable(watchfileItems[i]);
        } else {
            menuItemDisable(watchfileItems[i]);
        }
    }
    if (watchfileEntries.size > 0) {
        menuSelect(&watchfileMenu, watchfileItems[0]);
    } else {
        menuSelect(&watchfileMenu, watchfileReturn);
    }
    menuEnter(menu, &watchfileMenu);
}

static bool parseLine(const char *line, const char **errStr) {
    const char *p = line;
    /* skip whitespace, check for comment or empty line */
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') {
        ++p;
    }
    if (*p == '#' || *p == 0) {
        return TRUE;
    }
    /* read name part */
    if (*p++ != '"') {
        goto syntax_err;
    }
    const char *nameS = p;
    const char *nameE = strchr(p, '"');
    if (!nameE || nameE == nameS) {
        goto syntax_err;
    }
    s32 nameL = nameE - nameS;
    p = nameE + 1;
    while (*p == ' ' || *p == '\t') {
        ++p;
    }
    /* read type part */
    const char *typeS = p;
    while (*p && *p != ' ' && *p != '\t') {
        ++p;
    }
    while (*p == ' ' || *p == '\t') {
        ++p;
    }
    const char *exprS = p;
    /* construct entry */
    struct WatchfileEntry entry;
    entry.type = -1;
    for (s32 i = 0; i < sizeof(watchTypeName) / sizeof(*watchTypeName); ++i) {
        s32 l = strlen(watchTypeName[i]);
        if (strncmp(typeS, watchTypeName[i], l) == 0) {
            entry.type = i;
            break;
        }
    }
    if (entry.type == -1) {
        goto syntax_err;
    }
    enum AdexError e = adexParse(&entry.adex, exprS);
    if (e) {
        *errStr = adexErrorName[e];
        goto err;
    }
    entry.name = malloc(nameL + 1);
    memcpy(entry.name, nameS, nameL);
    entry.name[nameL] = 0;
    entry.animState = 0;
    /* insert entry */
    vector_push_back(&watchfileEntries, 1, &entry);
    return TRUE;
syntax_err:
    *errStr = adexErrorName[ADEX_ERROR_SYNTAX];
    goto err;
err:
    return FALSE;
}

static s32 importCallback(const char *path, void *data) {
    /* initialize watchfile data */
    watchfileListData = data;
    vector_init(&watchfileEntries, sizeof(struct WatchfileEntry));
    /* parse lines */
    const char *errStr = NULL;
    FILE *f = fopen(path, "r");
    char *line = malloc(1024);
    if (f) {
        while (1) {
            if (fgets(line, 1024, f)) {
                if (strchr(line, '\n') || feof(f)) {
                    if (!parseLine(line, &errStr)) {
                        break;
                    }
                } else {
                    errStr = "line overflow";
                    break;
                }
            } else {
                if (!feof(f)) {
                    errStr = strerror(errno);
                }
                break;
            }
        }
    } else {
        errStr = strerror(errno);
    }
    if (f) {
        fclose(f);
    }
    if (line) {
        free(line);
    }
    /* show error message or view file */
    struct Menu *menuTop = menuGetTop(watchfileListData->imenu);
    if (errStr) {
        watchfileDestroy();
        menuPrompt(menuTop, errStr, "return\0", 0, NULL, NULL);
    } else {
        menuReturn(menuTop);
        watchfileView(menuTop);
    }
    return 1;
}
