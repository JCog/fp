#include "menu.h"
#include "sys/gfx.h"
#include <stdlib.h>

struct StaticIconData {
    struct GfxTexture *texture;
    s32 textureTile;
    f32 scale;
};

struct MenuItem *menuAddStatic(struct Menu *menu, s32 x, s32 y, const char *text, u32 color) {
    struct MenuItem *item = menuItemAdd(menu, x, y, text, color);
    item->selectable = FALSE;
    return item;
}

static s32 staticIconDrawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    struct StaticIconData *data = item->data;
    s32 cw = menuGetCellWidth(item->owner, TRUE);
    s32 w = data->texture->tileWidth * data->scale;
    s32 h = data->texture->tileHeight * data->scale;
    struct GfxSprite sprite = {
        data->texture,
        data->textureTile,
        0,
        drawParams->x + (cw - w) / 2,
        drawParams->y - (gfxFontXheight(drawParams->font) + h + 1) / 2,
        data->scale,
        data->scale,
    };
    gfxModeReplace(GFX_MODE_FILTER, G_TF_BILERP);
    gfxModeReplace(GFX_MODE_DROPSHADOW, 0);
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
    gfxSpriteDraw(&sprite);
    gfxModePop(GFX_MODE_FILTER);
    gfxModePop(GFX_MODE_DROPSHADOW);
    return 1;
}

struct MenuItem *menuAddStaticIcon(struct Menu *menu, s32 x, s32 y, struct GfxTexture *texture, s32 textureTile,
                                   u32 color, f32 scale) {
    struct StaticIconData *data = malloc(sizeof(*data));
    data->texture = texture;
    data->textureTile = textureTile;
    data->scale = scale;
    struct MenuItem *item = menuItemAdd(menu, x, y, NULL, color);
    item->data = data;
    item->selectable = FALSE;
    item->drawProc = staticIconDrawProc;
    return item;
}

struct MenuItem *menuAddStaticCustom(struct Menu *menu, s32 x, s32 y,
                                     s32 (*drawProc)(struct MenuItem *item, struct MenuDrawParams *drawParams),
                                     const char *text, u32 color) {
    struct MenuItem *item = menuItemAdd(menu, x, y, text, color);
    item->selectable = FALSE;
    item->drawProc = drawProc;
    return item;
}

static s32 tooltipDrawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    struct Menu *toolMenu = item->data;
    while (toolMenu->child) {
        toolMenu = toolMenu->child;
    }
    if (toolMenu->selector && toolMenu->selector->tooltip) {
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
        gfxPrintf(drawParams->font, drawParams->x, drawParams->y, "%s", toolMenu->selector->tooltip);
    }
    return 1;
}

static s32 tooltipDestroyProc(struct MenuItem *item) {
    item->data = NULL;
    return 0;
}

struct MenuItem *menuAddTooltip(struct Menu *menu, s32 x, s32 y, struct Menu *toolMenu, u32 color) {
    struct MenuItem *item = menuItemAdd(menu, x, y, NULL, color);
    item->data = toolMenu;
    item->selectable = FALSE;
    item->drawProc = tooltipDrawProc;
    item->destroyProc = tooltipDestroyProc;
    return item;
}

static s32 imenuThinkProc(struct MenuItem *item) {
    if (item->imenu) {
        item->imenu->cxoffset = item->x;
        item->imenu->cyoffset = item->y;
        item->imenu->pxoffset = item->pxoffset;
        item->imenu->pyoffset = item->pyoffset;
    }
    return 0;
}

static s32 imenuNavigateProc(struct MenuItem *item, enum MenuNavigation nav) {
    if (item->imenu) {
        menuNavigate(item->imenu, nav);
        return 1;
    }
    return 0;
}

static s32 imenuActivateProc(struct MenuItem *item) {
    if (item->imenu) {
        menuActivate(item->imenu);
        return 1;
    }
    return 0;
}

struct MenuItem *menuAddImenu(struct Menu *menu, s32 x, s32 y, struct Menu **pImenu) {
    struct MenuItem *item = menuItemAdd(menu, x, y, NULL, 0);
    item->selectable = FALSE;
    item->thinkProc = imenuThinkProc;
    item->navigateProc = imenuNavigateProc;
    item->activateProc = imenuActivateProc;
    struct Menu *imenu = malloc(sizeof(*imenu));
    menuInit(imenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    imenu->parent = menu;
    item->imenu = imenu;
    if (pImenu) {
        *pImenu = imenu;
    }
    return item;
}

struct TabData {
    struct Menu *tabs;
    s32 nTabs;
    s32 currentTab;
};

static s32 tabDestroyProc(struct MenuItem *item) {
    item->imenu = NULL;
    return 0;
}

struct MenuItem *menuAddTab(struct Menu *menu, s32 x, s32 y, struct Menu *tabs, s32 nTabs) {
    struct TabData *data = malloc(sizeof(*data));
    data->tabs = tabs;
    data->nTabs = nTabs;
    data->currentTab = -1;
    struct MenuItem *item = menuItemAdd(menu, x, y, NULL, 0);
    item->data = data;
    item->selectable = FALSE;
    item->thinkProc = imenuThinkProc;
    item->navigateProc = imenuNavigateProc;
    item->activateProc = imenuActivateProc;
    item->destroyProc = tabDestroyProc;
    return item;
}

void menuTabGoto(struct MenuItem *item, s32 tabIndex) {
    struct TabData *data = item->data;
    if (data->tabs) {
        if (data->currentTab >= 0) {
            struct Menu *tab = &data->tabs[data->currentTab];
            struct MenuItem *selector = menuGetSelector(tab);
            if (selector) {
                menuSelectTop(item->owner, NULL);
            }
            tab->parent = NULL;
            item->imenu = NULL;
        }
        data->currentTab = tabIndex;
        if (data->currentTab >= 0) {
            struct Menu *tab = &data->tabs[data->currentTab];
            tab->parent = item->owner;
            item->imenu = tab;
        }
    }
}

void menuTabPrevious(struct MenuItem *item) {
    struct TabData *data = item->data;
    if (data->nTabs >= 0) {
        s32 tabIndex = (data->currentTab + data->nTabs - 1) % data->nTabs;
        menuTabGoto(item, tabIndex);
    }
}

void menuTabNext(struct MenuItem *item) {
    struct TabData *data = item->data;
    if (data->nTabs >= 0) {
        s32 tabIndex = (data->currentTab + 1) % data->nTabs;
        menuTabGoto(item, tabIndex);
    }
}

s32 menuTabGetCurrentTab(struct MenuItem *item) {
    struct TabData *data = item->data;
    return data->currentTab;
}
