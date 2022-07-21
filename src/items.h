#ifndef ITEMS_H
#define ITEMS_H
#include "gfx.h"
#include "menu.h"

void createBadgesMenu(struct Menu *menu, struct GfxTexture *itemTextureList[]);
void createNormalItemsMenu(struct Menu *menu, struct GfxTexture *itemTextureList[]);
void createKeyItemsMenu(struct Menu *menu, struct GfxTexture *itemTextureList[]);
void createStoredItemsMenu(struct Menu *menu, struct GfxTexture *itemTextureList[]);
void createItemSelectionMenu(struct GfxTexture *itemTextureList[]);

#endif // ITEMS_H
