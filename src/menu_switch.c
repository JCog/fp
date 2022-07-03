#include <stdlib.h>
#include "gfx.h"
#include "menu.h"

struct item_data {
    _Bool state;
    menu_generic_callback callback_proc;
    void *callback_data;
    struct gfx_texture *texture_on;
    s32 texture_tile_on;
    u32 color_on;
    struct gfx_texture *texture_off;
    s32 texture_tile_off;
    u32 color_off;
    f32 scale;
    _Bool disable_shadow;
    s32 anim_state;
    game_icon *icon_on;
    game_icon *icon_off;
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
    }
    if (data->icon_on) {
        s32 cw = menu_get_cell_width(item->owner, 1);
        draw_params->x += cw / 2;
        draw_params->y -= (gfx_font_xheight(draw_params->font) + 1) / 2;

        game_icon *icon;
        if ((data->anim_state > 0) != data->state) {
            icon = data->icon_on;
        } else {
            icon = data->icon_off;
        }
        if (item->owner->selector == item) {
            game_icons_set_render_pos(icon, draw_params->x - 1, draw_params->y - 1);
            game_icons_set_drop_shadow(icon, 1);
        } else {
            game_icons_set_render_pos(icon, draw_params->x, draw_params->y);
            game_icons_set_drop_shadow(icon, 0);
        }
        game_icons_set_alpha(icon, draw_params->alpha);
        game_icons_draw(icon);
    } else {
        struct gfx_texture *texture;
        s32 texture_tile;
        u32 color;
        if ((data->anim_state > 0) != data->state) {
            texture = data->texture_on;
            texture_tile = data->texture_tile_on;
            color = data->color_on;
        } else {
            texture = data->texture_off;
            texture_tile = data->texture_tile_off;
            color = data->color_off;
        }
        s32 cw = menu_get_cell_width(item->owner, 1);
        s32 w = texture->tile_width * data->scale;
        s32 h = texture->tile_height * data->scale;
        s32 x = draw_params->x + (cw - w) / 2;
        s32 y = draw_params->y - (gfx_font_xheight(draw_params->font) + h) / 2;
        if (item->owner->selector == item) {
            gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color, draw_params->alpha * 0x80 / 0xFF));
            gfx_mode_replace(GFX_MODE_COMBINE, G_CC_MODE(G_CC_PRIMITIVE, G_CC_PRIMITIVE));
            gfx_disp(gsSPScisTextureRectangle(qs102(x - 1), qs102(y - 1), qs102(x + w + 1), qs102(y + h + 1), 0, 0, 0,
                                              0, 0));
            gfx_mode_pop(GFX_MODE_COMBINE);
        }
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888((color >> 16) & 0xFF, (color >> 8) & 0xFF, (color >> 0) & 0xFF,
                                                    draw_params->alpha));
        if (data->scale != 1.f) {
            gfx_mode_replace(GFX_MODE_FILTER, G_TF_BILERP);
        } else {
            gfx_mode_replace(GFX_MODE_FILTER, G_TF_POINT);
        }
        if (data->disable_shadow) {
            gfx_mode_replace(GFX_MODE_DROPSHADOW, 0);
        }
        struct gfx_sprite sprite = {
            texture, texture_tile, x, y, data->scale, data->scale,
        };
        gfx_sprite_draw(&sprite);
        gfx_mode_pop(GFX_MODE_FILTER);
        if (data->disable_shadow) {
            gfx_mode_pop(GFX_MODE_DROPSHADOW);
        }
    }
    if (data->anim_state > 0) {
        data->anim_state = (data->anim_state + 1) % 3;
    }
    return 1;
}

static s32 activate_proc(struct menu_item *item) {
    struct item_data *data = item->data;
    if (!data->callback_proc ||
        !data->callback_proc(item, data->state ? MENU_CALLBACK_SWITCH_OFF : MENU_CALLBACK_SWITCH_ON,
                             data->callback_data)) {
        data->state = !data->state;
        data->anim_state = 1;
        if (data->callback_proc) {
            data->callback_proc(item, MENU_CALLBACK_CHANGED, data->callback_data);
        }
    }
    return 1;
}

struct menu_item *menu_add_switch(struct menu *menu, s32 x, s32 y, struct gfx_texture *texture_on, s32 texture_tile_on,
                                  u32 color_on, struct gfx_texture *texture_off, s32 texture_tile_off, u32 color_off,
                                  f32 scale, _Bool disable_shadow, menu_generic_callback callback_proc,
                                  void *callback_data) {
    struct menu_item *item = menu_item_add(menu, x, y, NULL, 0x808080);
    struct item_data *data = malloc(sizeof(*data));
    data->state = 0;
    data->callback_proc = callback_proc;
    data->callback_data = callback_data;
    data->texture_on = texture_on;
    data->texture_tile_on = texture_tile_on;
    data->color_on = color_on;
    data->texture_off = texture_off;
    data->texture_tile_off = texture_tile_off;
    data->color_off = color_off;
    data->scale = scale;
    data->disable_shadow = disable_shadow;
    data->anim_state = 0;
    data->icon_on = NULL;
    item->data = data;
    item->enter_proc = enter_proc;
    item->think_proc = think_proc;
    item->draw_proc = draw_proc;
    item->activate_proc = activate_proc;
    return item;
}

struct menu_item *menu_add_switch_game_icon(struct menu *menu, s32 x, s32 y, game_icon *icon_on, game_icon *icon_off,
                                            menu_generic_callback callback_proc, void *callback_data) {
    struct menu_item *item = menu_item_add(menu, x, y, NULL, 0x808080);
    struct item_data *data = malloc(sizeof(*data));
    data->state = 0;
    data->callback_proc = callback_proc;
    data->callback_data = callback_data;
    data->texture_on = NULL;
    data->texture_tile_on = 0;
    data->color_on = 0;
    data->texture_off = NULL;
    data->texture_tile_off = 0;
    data->color_off = 0;
    data->scale = 1.f;
    data->disable_shadow = 1;
    data->anim_state = 0;
    data->icon_on = icon_on;
    data->icon_off = icon_off;
    item->data = data;
    item->enter_proc = enter_proc;
    item->think_proc = think_proc;
    item->draw_proc = draw_proc;
    item->activate_proc = activate_proc;
    return item;
}

void menu_switch_set(struct menu_item *item, _Bool state) {
    struct item_data *data = item->data;
    data->state = state;
}

_Bool menu_switch_get(struct menu_item *item) {
    struct item_data *data = item->data;
    return data->state;
}

void menu_switch_toggle(struct menu_item *item) {
    struct item_data *data = item->data;
    data->state = !data->state;
}
