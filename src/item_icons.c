#include <stdlib.h>
#include "item_icons.h"

// note: currently space allocated for the texture and palettes are not freed.
// if memory becomes a concern we can destroy the space when leaving a menu that uses icons
IconEntry gIconEntries[337];

// parse the icon scripts to get size and address information
void item_icon_parse_script(u32 item_id, IconEntry* entry) {
    StaticItem* item = &gItemTable[item_id];
    u32* script = pm_IconScripts[item->iconID][0];
    u32 cur_cmd = *script;
    u32 prev_cmd = cur_cmd;

    while (1) {
        switch (cur_cmd) {
            case HUD_ELEMENT_OP_SetImage:
                entry->tex_rom = ITEM_ICONS_ROM_START + *(script + 2);
                entry->pal_rom = ITEM_ICONS_ROM_START + *(script + 3);
                break;

            case HUD_ELEMENT_OP_SetTileSize:
                switch(*(script + 1)) {
                    case HUD_ELEMENT_SIZE_8x8:
                        entry->width = entry->height = 8;
                        break;
                    case HUD_ELEMENT_SIZE_16x16:
                        entry->width = entry->height = 16;
                        break;
                    case HUD_ELEMENT_SIZE_24x24:
                        entry->width = entry->height = 24;
                        break;
                    case HUD_ELEMENT_SIZE_32x32:
                        entry->width = entry->height = 32;
                        break;
                    case HUD_ELEMENT_SIZE_48x48:
                        entry->width = entry->height = 48;
                        break;
                    case HUD_ELEMENT_SIZE_64x64:
                        entry->width = entry->height = 64;
                        break;
                    default:
                        break;
                }
                break;

            default:
                break;

        }
        
        prev_cmd = cur_cmd;
        script++;
        cur_cmd = *script;

        if ((cur_cmd == HUD_ELEMENT_OP_End) && (prev_cmd == HUD_ELEMENT_OP_Restart)) {
            break;
        }
    }

}

// allocate space for texture+palette and dma them
void item_icon_load(u32 item_id) {
    IconEntry* entry = &gIconEntries[item_id];

    if (entry->texture != NULL) {
        // already loaded, dont do anything
        return;
    }

    item_icon_parse_script(item_id, entry);

    u32 size = (entry->width * entry->height) / 2;

    entry->texture = malloc(size);
    entry->palette = malloc(32);

    dma_copy(entry->tex_rom, entry->tex_rom + size, entry->texture);
    dma_copy(entry->pal_rom, entry->pal_rom + 32, entry->palette);

    PRINTF("item_icon_load: texture:%8X palette:%8X  tex_rom:%8X pal_rom:%8X size:%d\n", 
            entry->texture, entry->palette, entry->tex_rom, entry->pal_rom, size);
}

void item_icon_draw(u32 item_id, s32 x, s32 y, u8 alpha) {
    IconEntry* entry = &gIconEntries[item_id];

    if (entry->texture == NULL) {
        item_icon_load(item_id);
    } else {
        draw_ci_image_with_clipping(entry->texture, entry->width, entry->height, G_IM_FMT_CI, G_IM_SIZ_4b, entry->palette, x, y, 0, 0, 
                                    SCREEN_WIDTH, SCREEN_HEIGHT, alpha);
    }
}
