#include "menu.h"
#include "sys/gfx.h"
#include <stdlib.h>

struct ItemData {
    bool state;
    bool displayState;
    MenuGenericCallback callbackProc;
    void *callbackData;
    struct GfxTexture *textureOn;
    s32 textureTileOn;
    s8 texturePaletteOn;
    u32 colorOn;
    struct GfxTexture *textureOff;
    s32 textureTileOff;
    s8 texturePaletteOff;
    u32 colorOff;
    f32 scale;
    bool disableShadow;
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
        data->displayState = data->state;
    }
    struct GfxTexture *texture;
    s32 textureTile;
    s8 texturePalette;
    u32 color;
    if (data->displayState) {
        texture = data->textureOn;
        textureTile = data->textureTileOn;
        texturePalette = data->texturePaletteOn;
        color = data->colorOn;
    } else {
        texture = data->textureOff;
        textureTile = data->textureTileOff;
        texturePalette = data->texturePaletteOff;
        color = data->colorOff;
    }
    s32 cw = menuGetCellWidth(item->owner, TRUE);
    f32 w = texture->tileWidth * data->scale;
    f32 h = texture->tileHeight * data->scale;
    s32 x = drawParams->x + (cw - w) / 2;
    s32 y = drawParams->y - (gfxFontXheight(drawParams->font) + h) / 2;
    if (item->owner->selector == item) {
        f32 outlineScale = 1.25;
        s32 outlineWidth = w * outlineScale;
        s32 outlineHeight = h * outlineScale;
        s32 outlineX = x - (outlineWidth - w) / 2;
        s32 outlineY = y - (outlineHeight - h) / 2;

        struct GfxSprite sprite = {
            data->textureOff, data->textureTileOff,       data->texturePaletteOff,    outlineX,
            outlineY,         data->scale * outlineScale, data->scale * outlineScale,
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

struct MenuItem *menuAddSwitch(struct Menu *menu, s32 x, s32 y, struct GfxTexture *textureOn, s32 textureTileOn,
                               s8 texturePaletteOn, u32 colorOn, struct GfxTexture *textureOff, s32 textureTileOff,
                               s8 texturePaletteOff, u32 colorOff, f32 scale, bool disableShadow,
                               MenuGenericCallback callbackProc, void *callbackData) {
    struct MenuItem *item = menuItemAdd(menu, x, y, NULL, 0x808080);
    struct ItemData *data = malloc(sizeof(*data));
    data->state = FALSE;
    data->callbackProc = callbackProc;
    data->callbackData = callbackData;
    data->textureOn = textureOn;
    data->textureTileOn = textureTileOn;
    data->texturePaletteOn = texturePaletteOn;
    data->colorOn = colorOn;
    data->textureOff = textureOff;
    data->textureTileOff = textureTileOff;
    data->texturePaletteOff = texturePaletteOff;
    data->colorOff = colorOff;
    data->scale = scale;
    data->disableShadow = disableShadow;
    data->animState = 0;
    item->data = data;
    item->enterProc = enterProc;
    item->thinkProc = thinkProc;
    item->drawProc = drawProc;
    item->activateProc = activateProc;
    return item;
}

void menuSwitchSet(struct MenuItem *item, bool state) {
    struct ItemData *data = item->data;
    data->state = state;
}

bool menuSwitchGet(struct MenuItem *item) {
    struct ItemData *data = item->data;
    return data->state;
}

void menuSwitchToggle(struct MenuItem *item) {
    struct ItemData *data = item->data;
    data->state = !data->state;
}
