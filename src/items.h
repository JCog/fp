#ifndef _ITEMS_H_
#define _ITEMS_H_
#include "gfx.h"
#include "menu.h"

void create_badges_menu(struct menu *menu, struct gfx_texture *item_texture_list[]);
void create_normal_items_menu(struct menu *menu, struct gfx_texture *item_texture_list[]);
void create_key_items_menu(struct menu *menu, struct gfx_texture *item_texture_list[]);
void create_stored_items_menu(struct menu *menu, struct gfx_texture *item_texture_list[]);
void create_item_selection_menu(struct gfx_texture *item_texture_list[]);

#endif //_ITEMS_H_
