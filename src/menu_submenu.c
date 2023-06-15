#include "menu.h"
#include <malloc.h>

struct ItemData {
    struct Menu *submenu;
    struct GfxTexture *texture;
    s32 textureTile;
    s8 texturePalette;
    f32 scale;
};

static s32 drawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    struct ItemData *data = item->data;
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
    if (data->submenu) {
        menuEnterTop(item->owner, data->submenu);
    } else {
        menuReturnTop(item->owner);
    }
    return 1;
}

struct MenuItem *menuAddSubmenu(struct Menu *menu, s32 x, s32 y, struct Menu *submenu, const char *name) {
    struct ItemData *data = malloc(sizeof(*data));
    data->submenu = submenu;
    data->texture = NULL;
    struct MenuItem *item = menuItemAdd(menu, x, y, name, 0xFFFFFF);
    item->data = data;
    item->activateProc = activateProc;
    return item;
}

struct MenuItem *menuAddSubmenuIcon(struct Menu *menu, s32 x, s32 y, struct Menu *submenu, struct GfxTexture *texture,
                                    s32 textureTile, s8 texturePalette, f32 scale) {
    struct ItemData *data = malloc(sizeof(*data));
    data->submenu = submenu;
    data->texture = texture;
    data->textureTile = textureTile;
    data->texturePalette = texturePalette;
    data->scale = scale;
    struct MenuItem *item = menuItemAdd(menu, x, y, NULL, 0xFFFFFF);
    item->data = data;
    item->activateProc = activateProc;
    item->drawProc = drawProc;
    return item;
}
