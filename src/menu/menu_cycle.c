#include "menu.h"
#include "sys/gfx.h"
#include <stdlib.h>

struct ItemData {
    s32 cycleState;
    MenuGenericCallback callbackProc;
    void *callbackData;
    s32 cycleCount;
    struct GfxTexture **textures;
    s32 *textureTiles;
    s8 *texturePalettes;
    u32 *colors;
    f32 scale;
    bool disableShadow;
    s32 displayState;
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
    if (data->animState > 0) {
        ++drawParams->x;
        ++drawParams->y;
    } else {
        data->displayState = data->cycleState;
    }
    struct GfxTexture *texture = data->textures[data->displayState];
    s32 textureTile = data->textureTiles[data->displayState];
    s8 texturePalette = data->texturePalettes[data->displayState];
    u32 color = data->colors[data->displayState];
    s32 cw = menuGetCellWidth(item->owner, TRUE);
    f32 w = texture->tileWidth * data->scale;
    f32 h = texture->tileHeight * data->scale;
    s32 x = drawParams->x + (cw - w) / 2;
    s32 y = drawParams->y - (gfxFontXheight(drawParams->font) + h) / 2;
    if (item->owner->selector == item) {
        f32 outlineScale = 1.2;
        s32 outlineWidth = w * outlineScale;
        s32 outlineHeight = h * outlineScale;
        s32 outlineX = x - (outlineWidth - w) / 2;
        s32 outlineY = y - (outlineHeight - h) / 2;

        struct GfxSprite sprite = {
            texture,
            textureTile,
            texturePalette,
            outlineX,
            outlineY,
            data->scale * outlineScale,
            data->scale * outlineScale,
        };
        gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
        gfxSpriteDraw(&sprite);
        gfxModePop(GFX_MODE_COLOR);
    }
    gfxModeSet(GFX_MODE_COLOR,
               GPACK_RGBA8888((color >> 16) & 0xFF, (color >> 8) & 0xFF, (color >> 0) & 0xFF, drawParams->alpha));
    if (data->scale != 1.f) {
        gfxModeReplace(GFX_MODE_FILTER, G_TF_BILERP);
    } else {
        gfxModeReplace(GFX_MODE_FILTER, G_TF_POINT);
    }
    if (data->disableShadow) {
        gfxModeReplace(GFX_MODE_DROPSHADOW, 0);
    }
    struct GfxSprite sprite = {
        texture, textureTile, texturePalette, x, y, data->scale, data->scale,
    };
    gfxSpriteDraw(&sprite);
    gfxModePop(GFX_MODE_FILTER);
    if (data->disableShadow) {
        gfxModePop(GFX_MODE_DROPSHADOW);
    }
    if (data->animState > 0) {
        data->animState = (data->animState + 1) % 3;
    }
    return 1;
}

static s32 activateProc(struct MenuItem *item) {
    struct ItemData *data = item->data;
    data->cycleState = (data->cycleState + 1) % data->cycleCount;
    data->animState = 1;
    if (data->callbackProc) {
        data->callbackProc(item, MENU_CALLBACK_CHANGED, data->callbackData);
    }
    return 1;
}

struct MenuItem *menuAddCycle(struct Menu *menu, s32 x, s32 y, s32 cycleCount, struct GfxTexture **textures,
                              s32 *textureTiles, s8 *texturePalettes, u32 *colors, f32 scale, bool disableShadow,
                              MenuGenericCallback callbackProc, void *callbackData) {
    struct MenuItem *item = menuItemAdd(menu, x, y, NULL, 0x808080);
    struct ItemData *data = malloc(sizeof(*data));
    data->cycleState = 0;
    data->callbackProc = callbackProc;
    data->callbackData = callbackData;
    data->cycleCount = cycleCount;
    data->textures = malloc(sizeof(textures) * cycleCount);
    data->textureTiles = malloc(sizeof(textureTiles) * cycleCount);
    data->texturePalettes = malloc(sizeof(texturePalettes) * cycleCount);
    data->colors = malloc(sizeof(colors) * cycleCount);
    memcpy(data->textures, textures, sizeof(textures) * cycleCount);
    memcpy(data->textureTiles, textureTiles, sizeof(textureTiles) * cycleCount);
    memcpy(data->texturePalettes, texturePalettes, sizeof(texturePalettes) * cycleCount);
    memcpy(data->colors, colors, sizeof(colors) * cycleCount);
    data->scale = scale;
    data->disableShadow = disableShadow;
    data->animState = 0;
    data->displayState = 0;
    item->data = data;
    item->enterProc = enterProc;
    item->thinkProc = thinkProc;
    item->drawProc = drawProc;
    item->activateProc = activateProc;
    return item;
}

void menuCycleSet(struct MenuItem *item, s32 state) {
    struct ItemData *data = item->data;
    data->cycleState = state;
}

s32 menuCycleGet(struct MenuItem *item) {
    struct ItemData *data = item->data;
    return data->cycleState;
}
