#ifndef _ITEM_BUTTON_H_
#define _ITEM_BUTTON_H_
#include "menu.h"

enum item_type {
    ITEM_TYPE_NORMAL,
    ITEM_TYPE_KEY,
    ITEM_TYPE_STORED,
    ITEM_TYPE_BADGE,
};

struct menu_item *menu_add_item_button(struct menu *menu, s32 x, s32 y, const char **item_names,
                                       struct gfx_texture **item_texture_list, enum item_type type, u16 item_slot_index,
                                       f32 scale, menu_action_callback callback_proc, void *callback_data);
u16 *menu_item_button_get_slot(struct menu_item *item);
enum item_type menu_item_button_get_type(struct menu_item *item);

#endif //_ITEM_BUTTON_H_
