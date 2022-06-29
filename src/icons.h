#ifndef ITEM_ICONS_H
#define ITEM_ICONS_H

#include "gfx.h"

typedef HudElement Icon;

Icon *icons_create_item(Item item, s32 x, s32 y, u8 alpha, f32 scale, _Bool grayscale);
Icon *icons_create_partner(u8 partner, s32 x, s32 y, u8 alpha, f32 scale, _Bool grayscale);

void icons_init();
void icons_update();

void icons_draw(Icon *icon);
void icons_delete(Icon *icon);

#endif
