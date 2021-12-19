#ifndef ITEM_ICONS_H
#define ITEM_ICONS_H

#include "fp.h"

typedef struct {
    u8* texture;
    u8* palette;
    u32 tex_rom;
    u32 pal_rom;
    u16 width;
    u16 height;
} IconEntry;

void item_icon_draw(u32 item_id, s32 x, s32 y, u8 alpha);

#endif