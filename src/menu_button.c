#include "gfx.h"
#include "menu.h"
#include <stdlib.h>

struct ItemData {
    MenuActionCallback callbackProc;
    void *callbackData;
    struct GfxTexture *texture;
    s32 textureTile;
    s8 texturePalette;
    f32 scale;
    s32 animState;
};

static s32 enterProc(struct MenuItem *item, enum MenuSwitchReason reason) {
    struct ItemData *data = item->data;
    data->animState = 0;
    return 0;
}

static s32 drawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    struct ItemData *data = item->data;
    if (data->animState > 0) {
        ++drawParams->x;
        ++drawParams->y;
        data->animState = (data->animState + 1) % 3;
    }
    if (data->texture) {
        s32 cw = menuGetCellWidth(item->owner, TRUE);
        struct GfxSprite sprite = {
            data->texture,
            data->textureTile,
            data->texturePalette,
            drawParams->x + (cw - data->texture->tileWidth) / 2,
            drawParams->y - (gfxFontXheight(drawParams->font) + data->texture->tileHeight + 1) / 2,
            data->scale,
            data->scale,
        };
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
        gfxSpriteDraw(&sprite);
        return 1;
    }
    return 0;
}

static s32 activateProc(struct MenuItem *item) {
    struct ItemData *data = item->data;
    data->callbackProc(item, data->callbackData);
    data->animState = 1;
    return 1;
}

struct MenuItem *menuAddButton(struct Menu *menu, s32 x, s32 y, const char *name, MenuActionCallback callbackProc,
                               void *callbackData) {
    struct ItemData *data = malloc(sizeof(*data));
    data->callbackProc = callbackProc;
    data->callbackData = callbackData;
    data->texture = NULL;
    data->animState = 0;
    struct MenuItem *item = menuItemAdd(menu, x, y, name, 0xFFFFFF);
    item->data = data;
    item->enterProc = enterProc;
    item->drawProc = drawProc;
    item->activateProc = activateProc;
    return item;
}

struct MenuItem *menuAddButtonIcon(struct Menu *menu, s32 x, s32 y, struct GfxTexture *texture, s32 textureTile,
                                   s8 texturePalette, u32 color, f32 scale, MenuActionCallback callbackProc,
                                   void *callbackData) {
    struct ItemData *data = malloc(sizeof(*data));
    data->callbackProc = callbackProc;
    data->callbackData = callbackData;
    data->texture = texture;
    data->textureTile = textureTile;
    data->texturePalette = texturePalette;
    data->scale = scale;
    data->animState = 0;
    struct MenuItem *item = menuItemAdd(menu, x, y, NULL, color);
    item->data = data;
    item->enterProc = enterProc;
    item->drawProc = drawProc;
    item->activateProc = activateProc;
    return item;
}
