#include "item_button.h"
#include "common.h"
#include "gfx.h"
#include <list/list.h>
#include <stdlib.h>

struct item_data {
    menu_action_callback callback_proc;
    void *callback_data;
    enum item_type item_type;
    u16 *item_slot;
    u16 item_id;
    const char **item_names;
    struct gfx_texture **item_texture_list;
    f32 scale;
    s32 anim_state;
};

static u16 get_empty_icon(enum item_type type) {
    switch (type) {
        case ITEM_TYPE_NORMAL:
        case ITEM_TYPE_STORED: return ITEM_MUSHROOM;
        case ITEM_TYPE_KEY: return ITEM_KOOPA_FORTRESS_KEY;
        case ITEM_TYPE_BADGE:
        default: return ITEM_SPEEDY_SPIN;
    }
}

static u16 *get_item_slot(enum item_type type, u16 item_index) {
    switch (type) {
        case ITEM_TYPE_NORMAL: return &pm_player.player_data.items[item_index];
        case ITEM_TYPE_KEY: return &pm_player.player_data.key_items[item_index];
        case ITEM_TYPE_STORED: return &pm_player.player_data.stored_items[item_index];
        case ITEM_TYPE_BADGE:
        default: return &pm_player.player_data.badges[item_index];
    }
}

static s32 enter_proc(struct menu_item *item, enum menu_switch_reason reason) {
    struct item_data *data = item->data;
    data->anim_state = 0;
    return 0;
}

static s32 think_proc(struct menu_item *item) {
    struct item_data *data = item->data;
    data->item_id = *data->item_slot;
    item->tooltip = data->item_names[*data->item_slot];
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
    struct gfx_texture *texture =
        data->item_texture_list[data->item_id == 0 ? get_empty_icon(data->item_type) : data->item_id];
    struct gfx_sprite sprite = {
        texture,
        0,
        data->item_id == 0 ? 1 : 0,
        draw_params->x + (cw - texture->tile_width) / 2,
        draw_params->y - (gfx_font_xheight(draw_params->font) + texture->tile_height + 1) / 2,
        data->scale,
        data->scale,
    };
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color, data->item_id == 0 ? 127 : 255));
    gfx_sprite_draw(&sprite);
    return 1;
}

static s32 activate_proc(struct menu_item *item) {
    struct item_data *data = item->data;
    data->callback_proc(item, data->callback_data);
    data->anim_state = 1;
    return 1;
}

struct menu_item *menu_add_item_button(struct menu *menu, s32 x, s32 y, const char **item_names,
                                       struct gfx_texture **item_texture_list, enum item_type type, u16 item_slot_index,
                                       f32 scale, menu_action_callback callback_proc, void *callback_data) {
    struct item_data *data = malloc(sizeof(*data));
    data->callback_proc = callback_proc;
    data->callback_data = callback_data;
    data->item_type = type;
    data->item_slot = get_item_slot(type, item_slot_index);
    data->item_id = 0;
    data->item_names = item_names;
    data->item_texture_list = item_texture_list;
    data->scale = scale;
    data->anim_state = 0;
    struct menu_item *item = menu_item_add(menu, x, y, NULL, 0xFFFFFF);
    item->data = data;
    item->enter_proc = enter_proc;
    item->think_proc = think_proc;
    item->draw_proc = draw_proc;
    item->activate_proc = activate_proc;
    return item;
}

u16 *menu_item_button_get_slot(struct menu_item *item) {
    struct item_data *data = item->data;
    return data->item_slot;
}

enum item_type menu_item_button_get_type(struct menu_item *item) {
    struct item_data *data = item->data;
    return data->item_type;
}
