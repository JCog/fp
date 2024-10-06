#ifndef ITEM_BUTTON_H
#define ITEM_BUTTON_H
#include "common.h"
#include "menu.h"

enum ItemType {
    ITEM_TYPE_NORMAL,
    ITEM_TYPE_KEY,
    ITEM_TYPE_STORED,
    ITEM_TYPE_BADGE,
};

struct MenuItem *menuAddItemButton(struct Menu *menu, s32 x, s32 y, const char **itemNames,
                                   struct GfxTexture **itemTextureList, enum ItemType type, s16 itemSlotIndex,
                                   f32 scale, MenuActionCallback callbackProc, void *callbackData);
s16 *menuItemButtonGetSlot(struct MenuItem *item);
enum ItemType menuItemButtonGetType(struct MenuItem *item);

#endif // ITEM_BUTTON_H
