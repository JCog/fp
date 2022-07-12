#include <stdlib.h>
#include "gfx.h"
#include "menu.h"

struct item_data {
    menu_action_callback callback_proc;
    void *callback_data;
    struct gfx_texture *texture;
    game_icon *icon;
    s32 texture_tile;
    s32 anim_state;
};

static s32 enter_proc(struct menu_item *item, enum menu_switch_reason reason) {
    struct item_data *data = item->data;
    data->anim_state = 0;
    return 0;
}

static s32 draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    struct item_data *data = item->data;
    if (data->anim_state > 0) {
        ++draw_params->x;
        ++draw_params->y;
        data->anim_state = (data->anim_state + 1) % 3;
    }
    s32 cw = menu_get_cell_width(item->owner, 1);
    if (data->texture) {
        struct gfx_sprite sprite = {
            data->texture,
            data->texture_tile,
            draw_params->x + (cw - data->texture->tile_width) / 2,
            draw_params->y - (gfx_font_xheight(draw_params->font) + data->texture->tile_height + 1) / 2,
            1.f,
            1.f,
        };
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color, draw_params->alpha));
        gfx_sprite_draw(&sprite);
        return 1;
    } else if (data->icon) {
        game_icon *icon = data->icon;
        s32 w = game_icons_get_width(icon);
        s32 h = game_icons_get_height(icon);
        s32 x = draw_params->x + (cw - w) / 2;
        s32 y = draw_params->y - (gfx_font_xheight(draw_params->font) + h + 1) / 2;
        game_icons_set_alpha(data->icon, draw_params->alpha);
        game_icons_set_pos(icon, x + icon->render_pos_offset.x, y + icon->render_pos_offset.y);
        if (item->owner->selector == item) {
            gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color, draw_params->alpha * 0x80 / 0xFF));
            gfx_mode_replace(GFX_MODE_COMBINE, G_CC_MODE(G_CC_PRIMITIVE, G_CC_PRIMITIVE));
            gfx_disp(gsSPScisTextureRectangle(qs102(x), qs102(y), qs102(x + w), qs102(y + h), 0, 0, 0, 0, 0));
            gfx_mode_pop(GFX_MODE_COMBINE);
            // game_icons_set_drop_shadow(data->icon, 1);
            // game_icons_set_pos(icon, x + icon->render_pos_offset.x + 1, y + icon->render_pos_offset.y + 1);
        } else {
            // game_icons_set_drop_shadow(data->icon, 0);
        }
        game_icons_draw(data->icon);
        return 1;
    }
    return 0;
}

static s32 activate_proc(struct menu_item *item) {
    struct item_data *data = item->data;
    data->callback_proc(item, data->callback_data);
    data->anim_state = 1;
    return 1;
}

struct menu_item *menu_add_button(struct menu *menu, s32 x, s32 y, const char *name, menu_action_callback callback_proc,
                                  void *callback_data) {
    struct item_data *data = malloc(sizeof(*data));
    data->callback_proc = callback_proc;
    data->callback_data = callback_data;
    data->texture = NULL;
    data->icon = NULL;
    data->anim_state = 0;
    struct menu_item *item = menu_item_add(menu, x, y, name, 0xFFFFFF);
    item->data = data;
    item->enter_proc = enter_proc;
    item->draw_proc = draw_proc;
    item->activate_proc = activate_proc;
    return item;
}

struct menu_item *menu_add_button_icon(struct menu *menu, s32 x, s32 y, struct gfx_texture *texture, s32 texture_tile,
                                       u32 color, menu_action_callback callback_proc, void *callback_data) {
    struct item_data *data = malloc(sizeof(*data));
    data->callback_proc = callback_proc;
    data->callback_data = callback_data;
    data->texture = texture;
    data->texture_tile = texture_tile;
    data->icon = NULL;
    data->anim_state = 0;
    struct menu_item *item = menu_item_add(menu, x, y, NULL, color);
    item->data = data;
    item->enter_proc = enter_proc;
    item->draw_proc = draw_proc;
    item->activate_proc = activate_proc;
    return item;
}

struct menu_item *menu_add_button_game_icon(struct menu *menu, s32 x, s32 y, game_icon *icon,
                                            menu_action_callback callback_proc, void *callback_data) {
    struct item_data *data = malloc(sizeof(*data));
    data->icon = icon;
    data->texture = NULL;
    data->callback_proc = callback_proc;
    data->callback_data = callback_data;
    data->anim_state = 0;
    struct menu_item *item = menu_item_add(menu, x, y, NULL, 0xFFFFFF);
    item->data = data;
    item->enter_proc = enter_proc;
    item->draw_proc = draw_proc;
    item->activate_proc = activate_proc;
    return item;
}
