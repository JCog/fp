#ifndef ITEM_ICONS_H
#define ITEM_ICONS_H

#include "fp.h"

typedef struct {
    u8 *texture; // pointer to icon texture, malloc'd on the heap
    u8 *palette; // pointer to icon palette, malloc'd on the heap
    u32 tex_rom; // rom start of the texture
    u32 pal_rom; // rom start of the palette
    u16 width;   // width of the texture
    u16 height;  // height of the texture
} IconEntry;

void item_icon_draw(u32 item_id, s32 x, s32 y, u8 alpha);

#endif
