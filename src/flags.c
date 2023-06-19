#include "flags.h"
#include "common.h"
#include "menu/menu.h"
#include "sys/gfx.h"
#include "sys/resource.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector/vector.h>

#define FLAG_LOG_LENGTH 16
#define FLAG_VIEW_ROWS  16

static s32 viewRecordIndex;
static struct MenuItem *viewRecordName;
static struct MenuItem *viewPageup;
static struct MenuItem *viewPagedown;
static struct MenuItem *viewRows[FLAG_VIEW_ROWS];
static struct MenuItem *viewCells[FLAG_VIEW_ROWS * 0x10];

struct FlagRecord {
    s32 wordSize;
    s32 length;
    void *data;
    void *comp;
    s16 indexLength;
    const char *name;
    s32 viewOffset;
};

struct FlagEvent {
    s32 recordIndex;
    s32 flagIndex;
    bool value;
    char description[32];
};

static struct vector records;
static struct vector events;

static void addRecord(size_t wordSize, size_t length, void *data, const char *name) {
    struct FlagRecord *record = vector_push_back(&records, 1, NULL);
    record->wordSize = wordSize;
    record->length = length;
    record->data = data;
    record->name = name;
    record->comp = calloc(length, wordSize);
    s32 nFlags = wordSize * 8 * length;
    record->indexLength = ((s32)(ceilf(log2f(nFlags))) + 3) / 4;
    record->viewOffset = 0;
}

static void addEvent(s32 recordIndex, s32 flagIndex, bool value) {
    if (events.size >= FLAG_LOG_LENGTH) {
        vector_erase(&events, 0, 1);
    }
    struct FlagRecord *r = vector_at(&records, recordIndex);
    struct FlagEvent *e = vector_push_back(&events, 1, NULL);
    e->recordIndex = recordIndex;
    e->flagIndex = flagIndex;
    e->value = value;
    sprintf(e->description, "%s[0x%0*lx] := %i", r->name, r->indexLength, flagIndex, value);
}

static u32 getFlagWord(void *data, size_t wordSize, s32 index) {
    if (wordSize == 1) {
        return ((u8 *)data)[index];
    } else if (wordSize == 2) {
        return ((u16 *)data)[index];
    } else if (wordSize == 4) {
        return ((u32 *)data)[index];
    }
    return 0;
}

static void modifyFlag(void *data, size_t wordSize, s32 flagIndex, bool value) {
    s32 word = flagIndex / (wordSize * 8);
    s32 bit = flagIndex % (wordSize * 8);
    if (wordSize == 1) {
        u8 *p = data;
        if (value) {
            p[word] |= (1 << bit);
        } else {
            p[word] &= ~(1 << bit);
        }
    } else if (wordSize == 2) {
        u16 *p = data;
        if (value) {
            p[word] |= (1 << bit);
        } else {
            p[word] &= ~(1 << bit);
        }
    } else if (wordSize == 4) {
        u32 *p = data;
        if (value) {
            p[word] |= (1 << bit);
        } else {
            p[word] &= ~(1 << bit);
        }
    }
}

static s32 logThinkProc(struct MenuItem *item) {
    for (s32 i = 0; i < records.size; ++i) {
        struct FlagRecord *r = vector_at(&records, i);
        for (s32 j = 0; j < r->length; ++j) {
            u32 wd = getFlagWord(r->data, r->wordSize, j);
            u32 wc = getFlagWord(r->comp, r->wordSize, j);
            u32 d = wd ^ wc;
            if (d != 0) {
                for (s32 k = 0; k < r->wordSize * 8; ++k) {
                    if ((d >> k) & 1) {
                        addEvent(i, r->wordSize * 8 * j + k, (wd >> k) & 1);
                    }
                }
            }
        }
    }
    updateFlagRecords();
    return 0;
}

static s32 logDrawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    s32 x = drawParams->x;
    s32 y = drawParams->y;
    struct GfxFont *font = drawParams->font;
    u32 color = drawParams->color;
    u8 alpha = drawParams->alpha;
    s32 ch = menuGetCellHeight(item->owner, TRUE);
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(color, alpha));
    for (s32 i = 0; i < events.size && i < FLAG_LOG_LENGTH; ++i) {
        struct FlagEvent *e = vector_at(&events, events.size - i - 1);
        gfxPrintf(font, x, y + ch * i, "%s", e->description);
    }
    return 1;
}

static void logUndoProc(struct MenuItem *item, void *data) {
    if (events.size == 0) {
        return;
    }
    struct FlagEvent *e = vector_at(&events, events.size - 1);
    struct FlagRecord *r = vector_at(&records, e->recordIndex);
    modifyFlag(r->data, r->wordSize, e->flagIndex, !e->value);
    modifyFlag(r->comp, r->wordSize, e->flagIndex, !e->value);
    vector_erase(&events, events.size - 1, 1);
}

static void logClearProc(struct MenuItem *item, void *data) {
    vector_erase(&events, 0, events.size);
}

static void updateView(void) {
    struct FlagRecord *r = vector_at(&records, viewRecordIndex);
    strcpy(viewRecordName->text, r->name);
    s32 nFlags = r->wordSize * 8 * r->length;
    viewPageup->enabled = viewPagedown->enabled = (nFlags > FLAG_VIEW_ROWS * 0x10);
    for (s32 y = 0; y < FLAG_VIEW_ROWS; ++y) {
        struct MenuItem *row = viewRows[y];
        row->enabled = (r->viewOffset + y * 0x10 < nFlags);
        if (row->enabled) {
            sprintf(viewRows[y]->text, "%04lx", r->viewOffset + y * 0x10);
        }
        for (s32 x = 0; x < 0x10; ++x) {
            s32 n = y * 0x10 + x;
            struct MenuItem *cell = viewCells[n];
            cell->enabled = (r->viewOffset + n < nFlags);
            if (cell->enabled) {
                cell->thinkProc(cell);
            }
        }
    }
}

static void gotoRecord(s32 recordIndex) {
    viewRecordIndex = recordIndex;
    updateView();
}

static void prevRecordProc(struct MenuItem *item, void *data) {
    gotoRecord((viewRecordIndex + records.size - 1) % records.size);
}

static void nextRecordProc(struct MenuItem *item, void *data) {
    gotoRecord((viewRecordIndex + 1) % records.size);
}

static void pageUpProc(struct MenuItem *item, void *data) {
    struct FlagRecord *r = vector_at(&records, viewRecordIndex);
    if (r->viewOffset > 0) {
        r->viewOffset -= 0x10;
        updateView();
    }
}

static void pageDownProc(struct MenuItem *item, void *data) {
    struct FlagRecord *r = vector_at(&records, viewRecordIndex);
    s32 nFlags = r->wordSize * 8 * r->length;
    if (r->viewOffset + FLAG_VIEW_ROWS * 0x10 < nFlags) {
        r->viewOffset += 0x10;
        updateView();
    }
}

static s32 flagProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    s32 flagIndex = (s32)data;
    struct FlagRecord *r = vector_at(&records, viewRecordIndex);
    flagIndex += r->viewOffset;
    if (reason == MENU_CALLBACK_THINK) {
        s32 word = flagIndex / (r->wordSize * 8);
        s32 bit = flagIndex % (r->wordSize * 8);
        bool v = (getFlagWord(r->data, r->wordSize, word) >> bit) & 1;
        menuSwitchSet(item, v);
    } else if (reason == MENU_CALLBACK_SWITCH_ON) {
        modifyFlag(r->data, r->wordSize, flagIndex, TRUE);
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        modifyFlag(r->data, r->wordSize, flagIndex, FALSE);
    }
    return 0;
}

void flagMenuCreate(struct Menu *menu) {
    /* initialize data */
    vector_init(&records, sizeof(struct FlagRecord));
    vector_init(&events, sizeof(struct FlagEvent));
    addRecord(4, 64, &pm_gCurrentSaveFile.globalFlags, "global flags");
    addRecord(4, 8, &pm_gCurrentSaveFile.areaFlags, "area flags");
    addRecord(4, 600, pm_gCurrentEncounter.defeatFlags, "enemy defeat flags");
    addRecord(1, 26, &pm_gCurrentSaveFile.globalBytes[0x16D], "tattle flags");
    /* initialize menus */
    menuInit(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu->selector = menuAddSubmenu(menu, 0, 0, NULL, "return");
    {
        static struct Menu log;
        menuAddSubmenu(menu, 0, 1, &log, "log");
        menuInit(&log, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
        log.selector = menuAddSubmenu(&log, 0, 0, NULL, "return");
        menuAddButton(&log, 0, 1, "undo", logUndoProc, NULL);
        menuAddButton(&log, 5, 1, "clear", logClearProc, NULL);
        struct MenuItem *logItem = menuItemAdd(&log, 0, 2, NULL, 0xC0C0C0);
        logItem->selectable = FALSE;
        logItem->thinkProc = logThinkProc;
        logItem->drawProc = logDrawProc;
    }
    {
        menuAddButton(menu, 4, 1, "<", prevRecordProc, NULL);
        menuAddButton(menu, 6, 1, ">", nextRecordProc, NULL);
        viewRecordName = menuAddStatic(menu, 8, 1, NULL, 0xC0C0C0);
        viewRecordName->text = malloc(32);
        struct GfxTexture *tArrow = resourceGet(RES_ICON_ARROW);
        viewPageup = menuAddButtonIcon(menu, 0, 2, tArrow, 0, 0, 0xFFFFFF, 1.0f, pageUpProc, NULL);
        viewPagedown = menuAddButtonIcon(menu, 2, 2, tArrow, 1, 0, 0xFFFFFF, 1.0f, pageDownProc, NULL);
        menuAddStatic(menu, 4, 2, "0123456789abcdef", 0xC0C0C0);
        static struct GfxTexture *tFlag;
        if (!tFlag) {
            tFlag = resourceLoadGrcTexture("flag_icons");
        }
        for (s32 y = 0; y < FLAG_VIEW_ROWS; ++y) {
            viewRows[y] = menuAddStatic(menu, 0, 3 + y, NULL, 0xC0C0C0);
            viewRows[y]->text = malloc(5);
            for (s32 x = 0; x < 0x10; ++x) {
                s32 n = y * 0x10 + x;
                viewCells[n] = menuAddSwitch(menu, 4 + x, 3 + y, tFlag, 1, 0, 0xFFFFFF, tFlag, 0, 0, 0xFFFFFF, 0.75f,
                                             TRUE, flagProc, (void *)n);
            }
        }
        gotoRecord(0);
    }
}

void updateFlagRecords(void) {
    for (s32 i = 0; i < records.size; ++i) {
        struct FlagRecord *r = vector_at(&records, i);
        memcpy(r->comp, r->data, r->wordSize * r->length);
    }
}
