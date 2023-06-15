#include "gfx.h"
#include "menu.h"
#include "resource.h"
#include <stdlib.h>

struct ItemData {
    MenuGenericCallback callbackProc;
    void *callbackData;
    bool state;
    s32 animState;
};

static s32 enterProc(struct MenuItem *item, enum MenuSwitchReason reason) {
    struct ItemData *data = item->data;
    data->animState = 0;
    return 0;
}

static s32 thinkProc(struct MenuItem *item) {
    struct ItemData *data = item->data;
    if (data->callbackProc) {
        return data->callbackProc(item, MENU_CALLBACK_THINK, data->callbackData);
    }
    return 0;
}

static s32 drawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    struct ItemData *data = item->data;
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
    static struct GfxTexture *texture = NULL;
    if (!texture) {
        texture = resourceLoadGrcTexture("checkbox");
    }
    s32 cw = menuGetCellWidth(item->owner, TRUE);
    struct GfxSprite sprite = {
        texture,
        data->animState == 0 ? 0 : 1,
        0,
        drawParams->x + (cw - texture->tileWidth) / 2,
        drawParams->y - (gfxFontXheight(drawParams->font) + texture->tileHeight + 1) / 2,
        1.f,
        1.f,
    };
    gfxSpriteDraw(&sprite);
    if ((data->animState > 0) != data->state) {
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0xFF, 0xFF, drawParams->alpha));
        sprite.textureTile = 2;
        gfxSpriteDraw(&sprite);
    }
    if (data->animState > 0) {
        data->animState = (data->animState + 1) % 3;
    }
    return 1;
}

static s32 activateProc(struct MenuItem *item) {
    struct ItemData *data = item->data;
    if (!data->callbackProc ||
        !data->callbackProc(item, data->state ? MENU_CALLBACK_SWITCH_OFF : MENU_CALLBACK_SWITCH_ON,
                            data->callbackData)) {
        data->state = !data->state;
        data->animState = 1;
        if (data->callbackProc) {
            data->callbackProc(item, MENU_CALLBACK_CHANGED, data->callbackData);
        }
    }
    return 1;
}

struct MenuItem *menuAddCheckbox(struct Menu *menu, s32 x, s32 y, MenuGenericCallback callbackProc,
                                 void *callbackData) {
    struct ItemData *data = malloc(sizeof(*data));
    data->callbackProc = callbackProc;
    data->callbackData = callbackData;
    data->animState = 0;
    struct MenuItem *item = menuItemAdd(menu, x, y, NULL, 0xFFFFFF);
    item->data = data;
    item->enterProc = enterProc;
    item->thinkProc = thinkProc;
    item->drawProc = drawProc;
    item->activateProc = activateProc;
    return item;
}

bool menuCheckboxGet(struct MenuItem *item) {
    struct ItemData *data = item->data;
    return data->state;
}

void menuCheckboxSet(struct MenuItem *item, bool state) {
    struct ItemData *data = item->data;
    data->state = state;
}
