#include "sys/gfx.h"
#include "menu.h"
#include "sys/resource.h"
#include <stdlib.h>

struct ItemData {
    MenuGenericCallback callbackProc;
    void *callbackData;
    bool active;
};

static s32 drawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    static struct GfxTexture *texture = NULL;
    if (!texture) {
        texture = resourceLoadGrcTexture("move_icon");
    }
    s32 cw = menuGetCellWidth(item->owner, TRUE);
    struct GfxSprite sprite = {
        texture,
        0,
        0,
        drawParams->x + (cw - texture->tileWidth) / 2,
        drawParams->y - (gfxFontXheight(drawParams->font) + texture->tileHeight + 1) / 2,
        1.f,
        1.f,
    };
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
    gfxSpriteDraw(&sprite);
    return 1;
}

static s32 navigateProc(struct MenuItem *item, enum MenuNavigation nav) {
    struct ItemData *data = item->data;
    if (data->active && data->callbackProc) {
        data->callbackProc(item, MENU_CALLBACK_NAV_UP + nav, data->callbackData);
    }
    return data->active;
}

static s32 activateProc(struct MenuItem *item) {
    struct ItemData *data = item->data;
    if (!data->callbackProc ||
        !data->callbackProc(item, data->active ? MENU_CALLBACK_DEACTIVATE : MENU_CALLBACK_ACTIVATE,
                            data->callbackData)) {
        data->active = !data->active;
        item->animateHighlight = data->active;
    }
    return 1;
}

struct MenuItem *menuAddPositioning(struct Menu *menu, s32 x, s32 y, MenuGenericCallback callbackProc,
                                    void *callbackData) {
    struct ItemData *data = malloc(sizeof(*data));
    data->callbackProc = callbackProc;
    data->callbackData = callbackData;
    data->active = FALSE;
    struct MenuItem *item = menuItemAdd(menu, x, y, NULL, 0xFFFFFF);
    item->data = data;
    item->drawProc = drawProc;
    item->navigateProc = navigateProc;
    item->activateProc = activateProc;
    return item;
}
