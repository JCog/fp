#include "item_button.h"
#include "common.h"
#include "gfx.h"
#include <list/list.h>
#include <stdlib.h>

struct ItemData {
    MenuActionCallback callbackProc;
    void *callbackData;
    enum ItemType itemType;
    u16 *itemSlot;
    u16 itemId;
    const char **itemNames;
    struct GfxTexture **itemTextureList;
    f32 scale;
    s32 animState;
};

static u16 getEmptyIcon(enum ItemType type) {
    switch (type) {
        case ITEM_TYPE_NORMAL:
        case ITEM_TYPE_STORED: return ITEM_MUSHROOM;
        case ITEM_TYPE_KEY: return ITEM_KOOPA_FORTRESS_KEY;
        case ITEM_TYPE_BADGE:
        default: return ITEM_SPEEDY_SPIN;
    }
}

static u16 *getItemSlot(enum ItemType type, u16 itemIndex) {
    switch (type) {
        case ITEM_TYPE_NORMAL: return &pm_gPlayerStatus.playerData.invItems[itemIndex];
        case ITEM_TYPE_KEY: return &pm_gPlayerStatus.playerData.keyItems[itemIndex];
        case ITEM_TYPE_STORED: return &pm_gPlayerStatus.playerData.storedItems[itemIndex];
        case ITEM_TYPE_BADGE:
        default: return &pm_gPlayerStatus.playerData.badges[itemIndex];
    }
}

static s32 enterProc(struct MenuItem *item, enum MenuSwitchReason reason) {
    struct ItemData *data = item->data;
    data->animState = 0;
    return 0;
}

static s32 thinkProc(struct MenuItem *item) {
    struct ItemData *data = item->data;
    data->itemId = *data->itemSlot;
    item->tooltip = data->itemNames[*data->itemSlot];
    return 0;
}

static s32 drawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    struct ItemData *data = item->data;
    if (data->animState > 0) {
        ++drawParams->x;
        ++drawParams->y;
        data->animState = (data->animState + 1) % 3;
    }
    s32 cw = menuGetCellWidth(item->owner, TRUE);
    struct GfxTexture *texture = data->itemTextureList[data->itemId == 0 ? getEmptyIcon(data->itemType) : data->itemId];
    struct GfxSprite sprite = {
        texture,
        0,
        data->itemId == 0 ? 1 : 0,
        drawParams->x + (cw - texture->tileWidth) / 2,
        drawParams->y - (gfxFontXheight(drawParams->font) + texture->tileHeight + 1) / 2,
        data->scale,
        data->scale,
    };
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, data->itemId == 0 ? 127 : 255));
    gfxSpriteDraw(&sprite);
    return 1;
}

static s32 activateProc(struct MenuItem *item) {
    struct ItemData *data = item->data;
    data->callbackProc(item, data->callbackData);
    data->animState = 1;
    return 1;
}

struct MenuItem *menuAddItemButton(struct Menu *menu, s32 x, s32 y, const char **itemNames,
                                   struct GfxTexture **itemTextureList, enum ItemType type, u16 itemSlotIndex,
                                   f32 scale, MenuActionCallback callbackProc, void *callbackData) {
    struct ItemData *data = malloc(sizeof(*data));
    data->callbackProc = callbackProc;
    data->callbackData = callbackData;
    data->itemType = type;
    data->itemSlot = getItemSlot(type, itemSlotIndex);
    data->itemId = 0;
    data->itemNames = itemNames;
    data->itemTextureList = itemTextureList;
    data->scale = scale;
    data->animState = 0;
    struct MenuItem *item = menuItemAdd(menu, x, y, NULL, 0xFFFFFF);
    item->data = data;
    item->enterProc = enterProc;
    item->thinkProc = thinkProc;
    item->drawProc = drawProc;
    item->activateProc = activateProc;
    return item;
}

u16 *menuItemButtonGetSlot(struct MenuItem *item) {
    struct ItemData *data = item->data;
    return data->itemSlot;
}

enum ItemType menuItemButtonGetType(struct MenuItem *item) {
    struct ItemData *data = item->data;
    return data->itemType;
}
