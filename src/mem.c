#include "mem.h"
#include "common.h"
#include "fp.h"
#include "input.h"
#include "menu.h"
#include "resource.h"
#include "util.h"
#include "watchlist.h"
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector/vector.h>

#define MEM_VIEW_ROWS 16
#define MEM_VIEW_COLS 8
#define MEM_VIEW_SIZE ((MEM_VIEW_COLS) * (MEM_VIEW_ROWS))

static s32 viewDomainIndex;
static s32 viewDataSize;
static bool viewFloat;
static struct MenuItem *viewAddress;
static struct MenuItem *viewType;
static struct MenuItem *viewDomainName;
static struct MenuItem *viewPageup;
static struct MenuItem *viewPagedown;
static struct MenuItem *viewCellHeader;
static struct MenuItem *viewRows[MEM_VIEW_ROWS];
static struct MenuItem *viewCells[MEM_VIEW_SIZE];

struct MemDomain {
    u32 start;
    u32 size;
    const char *name;
    s32 viewOffset;
};

static struct vector domains;

static void addDomain(u32 start, u32 size, const char *name) {
    struct MemDomain *domain = vector_push_back(&domains, 1, NULL);
    domain->start = start;
    domain->size = size;
    domain->name = name;
    domain->viewOffset = 0;
}

static void updateView(void) {
    struct MemDomain *d = vector_at(&domains, viewDomainIndex);
    if (d->size <= MEM_VIEW_SIZE || d->viewOffset < 0) {
        d->viewOffset = 0;
    } else if (d->viewOffset + MEM_VIEW_SIZE > d->size) {
        d->viewOffset = d->size - MEM_VIEW_SIZE;
    }
    menuIntinputSet(viewAddress, d->start + d->viewOffset);
    strcpy(viewDomainName->text, d->name);
    viewPageup->enabled = viewPagedown->enabled = (d->size > MEM_VIEW_SIZE);
    for (s32 y = 0; y < MEM_VIEW_ROWS; ++y) {
        struct MenuItem *row = viewRows[y];
        row->enabled = (d->viewOffset + y * MEM_VIEW_COLS < d->size);
        if (row->enabled) {
            sprintf(viewRows[y]->text, "%08" PRIx32, d->start + d->viewOffset + y * MEM_VIEW_COLS);
        }
        for (s32 x = 0; x < MEM_VIEW_COLS; ++x) {
            s32 n = y * MEM_VIEW_COLS + x;
            if (n % viewDataSize != 0) {
                continue;
            }
            struct MenuItem *cell = viewCells[n];
            cell->enabled = (d->viewOffset + n < d->size);
            if (cell->enabled) {
                cell->thinkProc(cell);
            }
        }
    }
    s16 width = 0;
    switch (viewDataSize) {
        case 1: width = 2; break;
        case 2: width = 4; break;
        case 4: width = viewFloat ? 14 : 8; break;
    }
    char *p = viewCellHeader->text;
    for (s32 i = 0; i < MEM_VIEW_COLS; ++i) {
        if (i % viewDataSize == 0) {
            p += sprintf(p, "%-*lx", width, (d->viewOffset + i) & 0xF);
        }
    }
}

static s32 cellProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    s32 cellIndex = (s32)data;
    struct MemDomain *d = vector_at(&domains, viewDomainIndex);
    switch (viewDataSize) {
        case 1: {
            u8 *p = (void *)(d->start + d->viewOffset + cellIndex);
            if (reason == MENU_CALLBACK_THINK_INACTIVE) {
                u8 v = *p;
                if (menuIntinputGet(item) != v) {
                    menuIntinputSet(item, v);
                }
            } else if (reason == MENU_CALLBACK_CHANGED) {
                *p = menuIntinputGet(item);
            }
            break;
        }
        case 2: {
            u16 *p = (void *)(d->start + d->viewOffset + cellIndex);
            if (reason == MENU_CALLBACK_THINK_INACTIVE) {
                u16 v = *p;
                if (menuIntinputGet(item) != v) {
                    menuIntinputSet(item, v);
                }
            } else if (reason == MENU_CALLBACK_CHANGED) {
                *p = menuIntinputGet(item);
            }
            break;
        }
        case 4: {
            u32 *p = (void *)(d->start + d->viewOffset + cellIndex);
            if (reason == MENU_CALLBACK_THINK_INACTIVE) {
                u32 v = *p;
                if (menuIntinputGet(item) != v) {
                    menuIntinputSet(item, v);
                }
            } else if (reason == MENU_CALLBACK_CHANGED) {
                *p = menuIntinputGet(item);
            }
            break;
        }
    }
    return 0;
}

static s32 floatCellProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    s32 cellIndex = (s32)data;
    struct MemDomain *d = vector_at(&domains, viewDomainIndex);
    f32 *p = (void *)(d->start + d->viewOffset + cellIndex);
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        f32 v = *p;
        if (isNan(v) || !isnormal(v) || menuFloatinputGet(item) != v) {
            menuFloatinputSet(item, v);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        *p = menuFloatinputGet(item);
    }
    return 0;
}

static void makeCells(struct Menu *menu) {
    s32 n = 0;
    for (s32 y = 0; y < MEM_VIEW_ROWS; ++y) {
        for (s32 x = 0; x < MEM_VIEW_COLS; ++x) {
            if (viewCells[n]) {
                menuItemRemove(viewCells[n]);
            }
            if (n % viewDataSize == 0) {
                if (viewFloat) {
                    viewCells[n] = menuAddFloatinput(menu, 9 + x / 4 * 14, 3 + y, 7, 2, floatCellProc, (void *)n);
                } else {
                    viewCells[n] = menuAddIntinput(menu, 9 + x * 2, 3 + y, 16, viewDataSize * 2, cellProc, (void *)n);
                }
            } else {
                viewCells[n] = NULL;
            }
            ++n;
        }
    }
}

static s32 addressProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_CHANGED) {
        memGoto(menuIntinputGet(item));
    }
    return 0;
}

static s32 dataTypeProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    if (reason == MENU_CALLBACK_DEACTIVATE) {
        switch (menuOptionGet(item)) {
            case 0:
                viewDataSize = 1;
                viewFloat = FALSE;
                break;
            case 1:
                viewDataSize = 2;
                viewFloat = FALSE;
                break;
            case 2:
                viewDataSize = 4;
                viewFloat = FALSE;
                break;
            case 3:
                viewDataSize = 4;
                viewFloat = FALSE;
                break;
        }
        makeCells(item->owner);
        struct MemDomain *d = vector_at(&domains, viewDomainIndex);
        memGoto(d->start + d->viewOffset);
    }
    return 0;
}

static void gotoDomain(s32 domainIndex) {
    viewDomainIndex = domainIndex;
    updateView();
}

static void prevDomainProc(struct MenuItem *item, void *data) {
    gotoDomain((viewDomainIndex + domains.size - 1) % domains.size);
}

static void nextDomainProc(struct MenuItem *item, void *data) {
    gotoDomain((viewDomainIndex + 1) % domains.size);
}

static void pageUpProc(struct MenuItem *item, void *data) {
    struct MemDomain *d = vector_at(&domains, viewDomainIndex);
    d->viewOffset -= ((inputPad() & BUTTON_Z) ? MEM_VIEW_SIZE : MEM_VIEW_COLS);
    updateView();
}

static void pageDownProc(struct MenuItem *item, void *data) {
    struct MemDomain *d = vector_at(&domains, viewDomainIndex);
    d->viewOffset += ((inputPad() & BUTTON_Z) ? MEM_VIEW_SIZE : MEM_VIEW_COLS);
    updateView();
}

static void addWatchProc(struct MenuItem *item, void *data) {
    s32 y = (s32)data;
    struct MemDomain *d = vector_at(&domains, viewDomainIndex);
    u32 address = d->start + d->viewOffset + y * MEM_VIEW_COLS;
    enum WatchType type;
    if (viewDataSize == 1) {
        type = WATCH_TYPE_X8;
    } else if (viewDataSize == 2) {
        type = WATCH_TYPE_X16;
    } else if (viewFloat) {
        type = WATCH_TYPE_F32;
    } else {
        type = WATCH_TYPE_X32;
    }
    if (watchlistAdd(fp.menuWatchlist, address, type) >= 0) {
        menuEnterTop(menuReturnTop(item->owner), fp.menuWatches);
    }
}

void memMenuCreate(struct Menu *menu) {
    /* initialize data */
    /* this is copied from gz, and I have no idea why sp regs and si regs crash, but commenting them out for now */
    vector_init(&domains, sizeof(struct MemDomain));
    addDomain(0x80000000, 0x00C00000, "k0 rdram");
    addDomain(0xA0000000, 0x00C00000, "k1 rdram");
    addDomain(0xA3F00000, 0x00100000, "rdram regs");
    addDomain(0xA4000000, 0x00001000, "sp dmem");
    addDomain(0xA4001000, 0x00001000, "sp imem");
    // add_domain(0xA4002000, 0x000FE000, "sp regs");
    addDomain(0xA4100000, 0x00100000, "dp com");
    addDomain(0xA4200000, 0x00100000, "dp span");
    addDomain(0xA4300000, 0x00100000, "mi regs");
    addDomain(0xA4400000, 0x00100000, "vi regs");
    addDomain(0xA4500000, 0x00100000, "ai regs");
    addDomain(0xA4600000, 0x00100000, "pi regs");
    // add_domain(0xA4800000, 0x00100000, "si regs");
    addDomain(0xA8000000, 0x08000000, "cart dom2");
    addDomain(0xB0000000, 0x0FC00000, "cart dom1");
    addDomain(0xBFC00000, 0x000007C0, "pif rom");
    addDomain(0xBFC007C0, 0x00000040, "pif ram");
    /* initialize menus */
    menuInit(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu->selector = menuAddSubmenu(menu, 0, 0, NULL, "return");
    {
        viewAddress = menuAddIntinput(menu, 0, 1, 16, 8, addressProc, NULL);
        viewType = menuAddOption(menu, 9, 1,
                                 "byte\0"
                                 "halfword\0"
                                 "word\0"
                                 "float\0",
                                 dataTypeProc, NULL);
        viewDataSize = 1;
        viewFloat = FALSE;
        menuAddButton(menu, 18, 1, "<", prevDomainProc, NULL);
        menuAddButton(menu, 20, 1, ">", nextDomainProc, NULL);
        viewDomainName = menuAddStatic(menu, 22, 1, NULL, 0xC0C0C0);
        viewDomainName->text = malloc(32);
        struct GfxTexture *tArrow = resourceGet(RES_ICON_ARROW);
        viewPageup = menuAddButtonIcon(menu, 0, 2, tArrow, 0, 0, 0xFFFFFF, 1.0f, pageUpProc, NULL);
        viewPagedown = menuAddButtonIcon(menu, 2, 2, tArrow, 1, 0, 0xFFFFFF, 1.0f, pageDownProc, NULL);
        viewCellHeader = menuAddStatic(menu, 9, 2, NULL, 0xC0C0C0);
        viewCellHeader->text = malloc(32);
        for (s32 y = 0; y < MEM_VIEW_ROWS; ++y) {
            viewRows[y] = menuAddButton(menu, 0, 3 + y, NULL, addWatchProc, (void *)y);
            viewRows[y]->text = malloc(9);
        }
        makeCells(menu);
        gotoDomain(0);
    }
}

void memGoto(u32 address) {
    address &= ~(viewDataSize - 1);
    for (s32 i = 0; i < domains.size; ++i) {
        struct MemDomain *d = vector_at(&domains, i);
        if (address >= d->start && address < d->start + d->size) {
            viewDomainIndex = i;
            d->viewOffset = address - d->start;
            break;
        }
    }
    updateView();
}

void memOpenWatch(struct Menu *menu, struct Menu *menuMem, u32 address, enum WatchType type) {
    switch (type) {
        case WATCH_TYPE_U8:
        case WATCH_TYPE_S8:
        case WATCH_TYPE_X8:
            viewDataSize = 1;
            viewFloat = FALSE;
            menuOptionSet(viewType, 0);
            break;

        case WATCH_TYPE_U16:
        case WATCH_TYPE_S16:
        case WATCH_TYPE_X16:
            viewDataSize = 2;
            viewFloat = FALSE;
            menuOptionSet(viewType, 1);
            break;

        case WATCH_TYPE_U32:
        case WATCH_TYPE_S32:
        case WATCH_TYPE_X32:
            viewDataSize = 4;
            viewFloat = FALSE;
            menuOptionSet(viewType, 2);
            break;

        case WATCH_TYPE_F32:
            viewDataSize = 4;
            viewFloat = TRUE;
            menuOptionSet(viewType, 3);
            break;

        default: break;
    }
    makeCells(menuMem);
    memGoto(address);
    menuEnterTop(menuReturnTop(menu), menuMem);
    if (menuMem->selector == NULL) {
        menuSelectTop(menuMem, viewCells[0]);
    }
}
