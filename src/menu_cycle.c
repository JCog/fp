#include "gfx.h"
#include "menu.h"
#include <stdlib.h>

struct item_data {
    s32 cycle_state;
    menu_generic_callback callback_proc;
    void *callback_data;
    s32 cycle_count;
    struct gfx_texture **textures;
    s32 *texture_tiles;
    s8 *texture_palettes;
    u32 *colors;
    f32 scale;
    bool disable_shadow;
    s32 display_state;
    s32 anim_state;
};

static s32 enter_proc(struct menu_item *item, enum menu_switch_reason reason) {
    struct item_data *data = item->data;
    data->anim_state = 0;
    return 0;
}

static s32 think_proc(struct menu_item *item) {
    struct item_data *data = item->data;
    if (data->callback_proc) {
        return data->callback_proc(item, MENU_CALLBACK_THINK, data->callback_data);
    }
    return 0;
}

static s32 draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    struct item_data *data = item->data;
    if (data->anim_state > 0) {
        ++draw_params->x;
        ++draw_params->y;
    } else {
        data->display_state = data->cycle_state;
    }
    struct gfx_texture *texture = data->textures[data->display_state];
    s32 texture_tile = data->texture_tiles[data->display_state];
    s8 texture_palette = data->texture_palettes[data->display_state];
    u32 color = data->colors[data->display_state];
    s32 cw = menu_get_cell_width(item->owner, TRUE);
    s32 w = texture->tile_width * data->scale;
    s32 h = texture->tile_height * data->scale;
    s32 x = draw_params->x + (cw - w) / 2;
    s32 y = draw_params->y - (gfx_font_xheight(draw_params->font) + h) / 2;
    if (item->owner->selector == item) {
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color, draw_params->alpha * 0x80 / 0xFF));
        gfx_mode_replace(GFX_MODE_COMBINE, G_CC_MODE(G_CC_PRIMITIVE, G_CC_PRIMITIVE));
        gfx_disp(
            gsSPScisTextureRectangle(qs102(x - 1), qs102(y - 1), qs102(x + w + 1), qs102(y + h + 1), 0, 0, 0, 0, 0));
        gfx_mode_pop(GFX_MODE_COMBINE);
    }
    gfx_mode_set(GFX_MODE_COLOR,
                 GPACK_RGBA8888((color >> 16) & 0xFF, (color >> 8) & 0xFF, (color >> 0) & 0xFF, draw_params->alpha));
    if (data->scale != 1.f) {
        gfx_mode_replace(GFX_MODE_FILTER, G_TF_BILERP);
    } else {
        gfx_mode_replace(GFX_MODE_FILTER, G_TF_POINT);
    }
    if (data->disable_shadow) {
        gfx_mode_replace(GFX_MODE_DROPSHADOW, 0);
    }
    struct gfx_sprite sprite = {
        texture, texture_tile, texture_palette, x, y, data->scale, data->scale,
    };
    gfx_sprite_draw(&sprite);
    gfx_mode_pop(GFX_MODE_FILTER);
    if (data->disable_shadow) {
        gfx_mode_pop(GFX_MODE_DROPSHADOW);
    }
    if (data->anim_state > 0) {
        data->anim_state = (data->anim_state + 1) % 3;
    }
    return 1;
}

static s32 activate_proc(struct menu_item *item) {
    struct item_data *data = item->data;
    data->cycle_state = (data->cycle_state + 1) % data->cycle_count;
    data->anim_state = 1;
    if (data->callback_proc) {
        data->callback_proc(item, MENU_CALLBACK_CHANGED, data->callback_data);
    }
    return 1;
}

struct menu_item *menu_add_cycle(struct menu *menu, s32 x, s32 y, s32 cycle_count, struct gfx_texture **textures,
                                 s32 *texture_tiles, s8 *texture_palettes, u32 *colors, f32 scale, bool disable_shadow,
                                 menu_generic_callback callback_proc, void *callback_data) {
    struct menu_item *item = menu_item_add(menu, x, y, NULL, 0x808080);
    struct item_data *data = malloc(sizeof(*data));
    data->cycle_state = 0;
    data->callback_proc = callback_proc;
    data->callback_data = callback_data;
    data->cycle_count = cycle_count;
    data->textures = malloc(sizeof(textures) * cycle_count);
    data->texture_tiles = malloc(sizeof(texture_tiles) * cycle_count);
    data->texture_palettes = malloc(sizeof(texture_palettes) * cycle_count);
    data->colors = malloc(sizeof(colors) * cycle_count);
    memcpy(data->textures, textures, sizeof(textures) * cycle_count);
    memcpy(data->texture_tiles, texture_tiles, sizeof(texture_tiles) * cycle_count);
    memcpy(data->texture_palettes, texture_palettes, sizeof(texture_palettes) * cycle_count);
    memcpy(data->colors, colors, sizeof(colors) * cycle_count);
    data->scale = scale;
    data->disable_shadow = disable_shadow;
    data->anim_state = 0;
    data->display_state = 0;
    item->data = data;
    item->enter_proc = enter_proc;
    item->think_proc = think_proc;
    item->draw_proc = draw_proc;
    item->activate_proc = activate_proc;
    return item;
}

void menu_cycle_set(struct menu_item *item, s32 state) {
    struct item_data *data = item->data;
    data->cycle_state = state;
}

s32 menu_cycle_get(struct menu_item *item) {
    struct item_data *data = item->data;
    return data->cycle_state;
}
