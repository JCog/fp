#include "menu.h"
#include "common.h"
#include "sys/gfx.h"
#include <list/list.h>
#include <stdlib.h>
#include <string.h>

void menuInit(struct Menu *menu, s32 cellWidth, s32 cellHeight, struct GfxFont *font) {
    menu->cxoffset = MENU_NOVALUE;
    menu->cyoffset = MENU_NOVALUE;
    menu->pxoffset = MENU_NOVALUE;
    menu->pyoffset = MENU_NOVALUE;
    menu->cellWidth = cellWidth;
    menu->cellHeight = cellHeight;
    menu->font = font;
    menu->alpha = 1.f;
    list_init(&menu->items, sizeof(struct MenuItem));
    menu->selector = NULL;
    menu->parent = NULL;
    menu->child = NULL;
    menu->highlightColorAnimated = 0x000000;
    menu->highlightColorStatic = 0x8080FF;
    menu->highlightState[0] = 17;
    menu->highlightState[1] = 19;
    menu->highlightState[2] = 23;
}

void menuImitate(struct Menu *dest, struct Menu *src) {
    dest->cellWidth = src->cellWidth;
    dest->cellHeight = src->cellHeight;
    dest->font = src->font;
    dest->alpha = src->alpha;
    dest->highlightColorStatic = src->highlightColorStatic;
}

void menuDestroy(struct Menu *menu) {
    while (menu->items.first) {
        menuItemRemove(menu->items.first);
    }
}

s32 menuGetCxoffset(struct Menu *menu, bool inherit) {
    s32 cxoffset = menu->cxoffset;
    if (inherit && menu->parent) {
        cxoffset += menuGetCxoffset(menu->parent, TRUE);
    }
    return cxoffset;
}

void menuSetCxoffset(struct Menu *menu, s32 cxoffset) {
    menu->cxoffset = cxoffset;
}

s32 menuGetCyoffset(struct Menu *menu, bool inherit) {
    s32 cyoffset = menu->cyoffset;
    if (inherit && menu->parent) {
        cyoffset += menuGetCyoffset(menu->parent, TRUE);
    }
    return cyoffset;
}

void menuSetCyoffset(struct Menu *menu, s32 cyoffset) {
    menu->cyoffset = cyoffset;
}

s32 menuGetPxoffset(struct Menu *menu, bool inherit) {
    s32 pxoffset = menu->pxoffset;
    if (inherit && menu->parent) {
        pxoffset += menuGetPxoffset(menu->parent, TRUE);
    }
    return pxoffset;
}

void menuSetPxoffset(struct Menu *menu, s32 pxoffset) {
    menu->pxoffset = pxoffset;
}

s32 menuGetPyoffset(struct Menu *menu, bool inherit) {
    s32 pyoffset = menu->pyoffset;
    if (inherit && menu->parent) {
        pyoffset += menuGetPyoffset(menu->parent, TRUE);
    }
    return pyoffset;
}

void menuSetPyoffset(struct Menu *menu, s32 pyoffset) {
    menu->pyoffset = pyoffset;
}

s32 menuGetCellWidth(struct Menu *menu, bool inherit) {
    if (inherit && menu->parent && menu->cellWidth == MENU_NOVALUE) {
        return menuGetCellWidth(menu->parent, TRUE);
    }
    return menu->cellWidth;
}

void menuSetCellWidth(struct Menu *menu, s32 cellWidth) {
    menu->cellWidth = cellWidth;
}

s32 menuGetCellHeight(struct Menu *menu, bool inherit) {
    if (inherit && menu->parent && menu->cellHeight == MENU_NOVALUE) {
        return menuGetCellHeight(menu->parent, TRUE);
    }
    return menu->cellHeight;
}

void menuSetCellHeight(struct Menu *menu, s32 cellHeight) {
    menu->cellHeight = cellHeight;
}

struct GfxFont *menuGetFont(struct Menu *menu, bool inherit) {
    if (inherit && menu->parent && menu->font == MENU_NOVALUE) {
        return menuGetFont(menu->parent, TRUE);
    }
    return menu->font;
}

void menuSetFont(struct Menu *menu, struct GfxFont *font) {
    menu->font = font;
}

f32 menuGetAlpha(struct Menu *menu, bool inherit) {
    f32 alpha = menu->alpha;
    if (inherit && menu->parent) {
        alpha *= menuGetAlpha(menu->parent, TRUE);
    }
    return alpha;
}

u8 menuGetAlphaI(struct Menu *menu, bool inherit) {
    f32 alpha = menuGetAlpha(menu, inherit);
    if (alpha < 0.f) {
        alpha = 0.f;
    } else if (alpha > 1.f) {
        alpha = 1.f;
    }
    return alpha * 0xFF;
}

void menuSetAlpha(struct Menu *menu, f32 alpha) {
    menu->alpha = alpha;
}

s32 menuCellScreenX(struct Menu *menu, s32 cellX) {
    return (cellX + menuGetCxoffset(menu, TRUE)) * menuGetCellWidth(menu, TRUE) + menuGetPxoffset(menu, TRUE);
}

s32 menuCellScreenY(struct Menu *menu, s32 cellY) {
    return (cellY + menuGetCyoffset(menu, TRUE)) * menuGetCellHeight(menu, TRUE) + menuGetPyoffset(menu, TRUE);
}

struct MenuItem *menuGetSelector(struct Menu *menu) {
    if (menu->child) {
        return menuGetSelector(menu->child);
    }
    return menu->selector;
}

struct Menu *menuGetTop(struct Menu *menu) {
    if (menu->parent) {
        return menuGetTop(menu->parent);
    }
    return menu;
}

struct Menu *menuGetFront(struct Menu *menu) {
    if (menu->child) {
        return menuGetFront(menu->child);
    }
    return menu;
}

s32 menuThink(struct Menu *menu) {
    if (menu->child) {
        return menuThink(menu->child);
    }
    for (struct MenuItem *item = menu->items.first; item; item = list_next(item)) {
        if (!item->enabled) {
            continue;
        }
        if (item->thinkProc && item->thinkProc(item)) {
            return 1;
        }
        if (item->imenu && menuThink(item->imenu)) {
            return 1;
        }
    }
    return 0;
}

void menuDraw(struct Menu *menu) {
    if (menu->child) {
        return menuDraw(menu->child);
    }
    for (s32 i = 0; i < 3; ++i) {
        s32 shift = i * 8;
        u32 mask = 0xFF << shift;
        s32 v = (menu->highlightColorAnimated & mask) >> shift;
        v += menu->highlightState[i];
        if (v < 0x00 || v > 0xFF) {
            v = -v + (v > 0xFF ? 2 * 0xFF : 0);
            menu->highlightState[i] = -menu->highlightState[i];
        }
        menu->highlightColorAnimated &= ~mask;
        menu->highlightColorAnimated |= (u32)v << shift;
    }
    struct GfxFont *font = menuGetFont(menu, TRUE);
    u8 alpha = menuGetAlphaI(menu, TRUE);
    for (struct MenuItem *item = menu->items.first; item; item = list_next(item)) {
        if (!item->enabled) {
            continue;
        }
        struct MenuDrawParams drawParams = {
            menuItemScreenX(item),
            menuItemScreenY(item),
            item->text,
            font,
            (item == menu->selector
                 ? (item->animateHighlight ? menu->highlightColorAnimated : menu->highlightColorStatic)
                 : item->color),
            alpha,
        };
        if (item->drawProc && item->drawProc(item, &drawParams)) {
            continue;
        }
        if (item->imenu) {
            menuDraw(item->imenu);
        }
        if (!drawParams.text || !drawParams.font) {
            continue;
        }
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams.color, drawParams.alpha));
        gfxPrintf(drawParams.font, drawParams.x, drawParams.y, "%s", drawParams.text);
    }
}

static void menuNavCompare(struct Menu *menu, struct MenuItem *selector, enum MenuNavigation nav, s32 navX, s32 navY,
                           struct MenuItem **nearItem, struct MenuItem **farItem, s32 *npa, s32 *fpa, s32 *npe,
                           s32 *fpe) {
    if (menu->child) {
        return menuNavCompare(menu->child, selector, nav, navX, navY, nearItem, farItem, npa, fpa, npe, fpe);
    }
    s32 selX = selector ? menuItemScreenX(selector) : 0;
    s32 selY = selector ? menuItemScreenY(selector) : 0;
    for (struct MenuItem *item = menu->items.first; item; item = list_next(item)) {
        if (!item->enabled) {
            continue;
        }
        if (item->imenu) {
            menuNavCompare(item->imenu, selector, nav, navX, navY, nearItem, farItem, npa, fpa, npe, fpe);
        }
        if (item == menu->selector || !item->selectable) {
            continue;
        }

        if (selector && selector->chainLinks[nav] == item) {
            *nearItem = item;
            *npe = 0;
            return;
        }
        s32 distanceX = menuItemScreenX(item) - selX;
        s32 distanceY = menuItemScreenY(item) - selY;
        s32 distancePa = (navX ? distanceX * navX : distanceY * navY);
        s32 distancePe = (navY ? distanceX : distanceY);
        if (distancePe < 0) {
            distancePe = -distancePe;
        }
        if (distancePa > 0 && (*nearItem == NULL || distancePe < *npe || (distancePe == *npe && distancePa < *npa))) {
            *npa = distancePa;
            *npe = distancePe;
            *nearItem = item;
        }
        if (distancePa < 0 && (*farItem == NULL || -distancePa > *fpa || (-distancePa == *fpa && distancePe < *fpe))) {
            *fpa = -distancePa;
            *fpe = distancePe;
            *farItem = item;
        }
    }
}

void menuNavigate(struct Menu *menu, enum MenuNavigation nav) {
    if (menu->child) {
        return menuNavigate(menu->child, nav);
    }
    if (menu->selector && menu->selector->navigateProc && menu->selector->navigateProc(menu->selector, nav)) {
        return;
    }
    s32 navX = (nav == MENU_NAVIGATE_LEFT ? -1 : (nav == MENU_NAVIGATE_RIGHT ? 1 : 0));
    s32 navY = (nav == MENU_NAVIGATE_UP ? -1 : (nav == MENU_NAVIGATE_DOWN ? 1 : 0));
    if (navX == 0 && navY == 0) {
        return;
    }
    s32 npa = 0;
    s32 fpa = 0;
    s32 npe = 0;
    s32 fpe = 0;
    struct MenuItem *nearItem = NULL;
    struct MenuItem *farItem = NULL;
    menuNavCompare(menu, menu->selector, nav, navX, navY, &nearItem, &farItem, &npa, &fpa, &npe, &fpe);
    if (nearItem) {
        menuSelect(menu, nearItem);
    } else if (farItem) {
        menuSelect(menu, farItem);
    }
}

void menuActivate(struct Menu *menu) {
    if (menu->child) {
        return menuActivate(menu->child);
    }
    if (menu->selector && menu->selector->activateProc) {
        menu->selector->activateProc(menu->selector);
    }
}

void menuEnter(struct Menu *menu, struct Menu *submenu) {
    if (menu->child) {
        return menuEnter(menu->child, submenu);
    }
    menuSignalLeave(menu, MENU_SWITCH_ENTER);
    menu->child = submenu;
    submenu->parent = menu;
    menuSignalEnter(submenu, MENU_SWITCH_ENTER);
}

struct Menu *menuReturn(struct Menu *menu) {
    if (menu->child) {
        return menuReturn(menu->child);
    }
    struct Menu *parent = menu->parent;
    if (!parent || parent->child != menu) {
        return NULL;
    }
    menuSignalLeave(menu, MENU_SWITCH_RETURN);
    menu->parent = NULL;
    parent->child = NULL;
    menuSignalEnter(parent, MENU_SWITCH_RETURN);
    return parent;
}

void menuSelect(struct Menu *menu, struct MenuItem *item) {
    if (menu->child) {
        return menuSelect(menu->child, item);
    }
    if (menu->selector) {
        menu->selector->owner->selector = NULL;
    }
    menu->selector = item;
    item->owner->selector = item;
}

void menuSignalEnter(struct Menu *menu, enum MenuSwitchReason reason) {
    if (menu->child) {
        return menuSignalEnter(menu->child, reason);
    }
    for (struct MenuItem *item = menu->items.first; item; item = list_next(item)) {
        if (item->enterProc && item->enterProc(item, reason)) {
            continue;
        }
        if (item->imenu) {
            menuSignalEnter(item->imenu, reason);
        }
    }
}

void menuSignalLeave(struct Menu *menu, enum MenuSwitchReason reason) {
    if (menu->child) {
        return menuSignalLeave(menu->child, reason);
    }
    for (struct MenuItem *item = menu->items.first; item; item = list_next(item)) {
        if (item->leaveProc && item->leaveProc(item, reason)) {
            continue;
        }
        if (item->imenu) {
            menuSignalLeave(item->imenu, reason);
        }
    }
}

void menuNavigateTop(struct Menu *menu, enum MenuNavigation nav) {
    if (menu->parent) {
        return menuNavigateTop(menu->parent, nav);
    }
    return menuNavigate(menu, nav);
}

void menuActivateTop(struct Menu *menu) {
    if (menu->parent) {
        return menuActivateTop(menu->parent);
    }
    return menuActivate(menu);
}

void menuEnterTop(struct Menu *menu, struct Menu *submenu) {
    if (menu->parent) {
        return menuEnterTop(menu->parent, submenu);
    }
    return menuEnter(menu, submenu);
}

struct Menu *menuReturnTop(struct Menu *menu) {
    if (menu->parent) {
        return menuReturnTop(menu->parent);
    }
    return menuReturn(menu);
}

void menuSelectTop(struct Menu *menu, struct MenuItem *item) {
    if (menu->parent) {
        return menuSelectTop(menu->parent, item);
    }
    return menuSelect(menu, item);
}

struct MenuItem *menuItemAdd(struct Menu *menu, s32 x, s32 y, const char *text, u32 color) {
    struct MenuItem *item = list_push_back(&menu->items, NULL);
    if (item) {
        item->owner = menu;
        item->enabled = TRUE;
        item->x = x;
        item->y = y;
        item->pxoffset = 0;
        item->pyoffset = 0;
        if (text) {
            item->text = malloc(strlen(text) + 1);
            strcpy(item->text, text);
        } else {
            item->text = NULL;
        }
        item->tooltip = NULL;
        item->color = color;
        item->animateHighlight = FALSE;
        item->data = NULL;
        item->selectable = TRUE;
        item->imenu = NULL;
        item->enterProc = NULL;
        item->leaveProc = NULL;
        item->thinkProc = NULL;
        item->drawProc = NULL;
        item->navigateProc = NULL;
        item->activateProc = NULL;
        item->destroyProc = NULL;
    }
    return item;
}

static void menuDeselect(struct Menu *menu, struct MenuItem *item) {
    if (menu->selector == item) {
        menu->selector = NULL;
    }
    if (menu->parent && menu->parent->child != menu) {
        menuDeselect(menu->parent, item);
    }
}

void menuItemEnable(struct MenuItem *item) {
    item->enabled = TRUE;
}

void menuItemDisable(struct MenuItem *item) {
    item->enabled = FALSE;
    menuDeselect(item->owner, item);
}

void menuItemTransfer(struct MenuItem *item, struct Menu *menu) {
    if (menu == item->owner) {
        return;
    }
    menuDeselect(item->owner, item);
    list_transfer(&menu->items, NULL, &item->owner->items, item);
    item->owner = menu;
}

void menuItemRemove(struct MenuItem *item) {
    if (!item->destroyProc || !item->destroyProc(item)) {
        if (item->imenu) {
            menuDestroy(item->imenu);
            free(item->imenu);
        }
        if (item->text) {
            free(item->text);
        }
        if (item->data) {
            free(item->data);
        }
    }
    menuDeselect(item->owner, item);
    list_erase(&item->owner->items, item);
}

void menuItemAddChainLink(struct MenuItem *fromItem, struct MenuItem *toItem, enum MenuNavigation direction) {
    fromItem->chainLinks[direction] = toItem;
}

void menuItemCreateChain(struct MenuItem *items[], s32 itemsSize, enum MenuNavigation navDirection, bool loop,
                         bool reverseChain) {
    if (!reverseChain) {
        for (s32 i = 0; i < itemsSize - 1; i++) {
            menuItemAddChainLink(items[i], items[i + 1], navDirection);
        }
        if (loop) {
            menuItemAddChainLink(items[itemsSize - 1], items[0], navDirection);
        }
    } else {
        for (s32 i = itemsSize - 1; i > 0; i--) {
            menuItemAddChainLink(items[i], items[i - 1], navDirection);
        }
        if (loop) {
            menuItemAddChainLink(items[0], items[itemsSize - 1], navDirection);
        }
    }
}

s32 menuItemScreenX(struct MenuItem *item) {
    return menuCellScreenX(item->owner, item->x) + item->pxoffset;
}

s32 menuItemScreenY(struct MenuItem *item) {
    return menuCellScreenY(item->owner, item->y) + item->pyoffset;
}
