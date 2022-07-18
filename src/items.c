#include <stdlib.h>
#include <stdio.h>
#include "fp.h"
#include "items.h"
#include "item_button.h"
#include "resource.h"
#include "util.h"

static const u16 items_normal_food[] = {
    0x8A, 0x8B, 0x8C, 0x8E, 0x95, 0x8D, 0xA4, 0xA3, 0xA2, 0x89, 0x93, 0x94, 0x9B, 0x9C, 0x9D,
    0x9E, 0x9F, 0xA0, 0xA1, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAD, 0xAE, 0xAF,
};

static const u16 items_normal_other[] = {
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x88, 0x8F, 0x90, 0x92, 0x96, 0x97, 0x98, 0x9A, 0xAC,
};

static const u16 items_normal_tayce[] = {
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE,
    0xBF, 0xC0, 0xC1, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE,
    0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xC2,
};

static const u16 items_normal_unused[] = {
    0x87, 0x91, 0x99, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
};

static const u16 items_key_required[] = {
    0x7,  0x19, 0x10, 0x12, 0x11, 0x16, 0x17, 0x15, 0x22, 0x1C, 0x1D, 0x13, 0x1F, 0x20,
    0x21, 0x23, 0x24, 0x25, 0x2A, 0xF,  0x5B, 0x2B, 0x2C, 0x2D, 0x2E, 0x1E, 0x57, 0x58,
    0x59, 0x5A, 0x79, 0x32, 0x31, 0x6C, 0x34, 0x33, 0x14, 0x1A, 0x6A, 0x6D,
};

static const u16 items_key_other[] = {
    0x56, 0x67, 0x68, 0x53, 0x30, 0x69, 0x29, 0x6B, 0x7A, 0x6E, 0x27, 0x5C, 0x6F,
    0x70, 0x71, 0x73, 0x74, 0x76, 0x77, 0x7B, 0x7C, 0xA,  0xB,  0xC,  0xD,  0xE,
};

static const u16 items_key_letters[] = {
    0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x40, 0x41, 0x45, 0x46,
    0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4E, 0x4F, 0x50, 0x51, 0x52, 0x54,
};

static const u16 items_key_unused[] = {
    0x8,  0x9,  0x18, 0x1B, 0x26, 0x28, 0x2F, 0x35, 0x3F, 0x42,
    0x43, 0x44, 0x4D, 0x55, 0x72, 0x75, 0x78, 0x7D, 0x7E, 0x7F,
};

static const u16 items_badges[] = {
    0x121, 0x123, 0x0E1, 0x10A, 0x10C, 0x12F, 0x0EB, 0x0EF, 0x12E, 0x0E4, 0x124, 0x126, 0x109, 0x10B, 0x0E0, 0x0E8,
    0x117, 0x108, 0x107, 0x0EE, 0x0E9, 0x106, 0x143, 0x0ED, 0x144, 0x0E3, 0x0FF, 0x136, 0x140, 0x141, 0x129, 0x132,
    0x133, 0x0E7, 0x134, 0x14A, 0x0F8, 0x135, 0x14D, 0x105, 0x138, 0x0E2, 0x13C, 0x0FB, 0x130, 0x13A, 0x116, 0x113,
    0x0F5, 0x0F4, 0x0F9, 0x111, 0x10D, 0x112, 0x0F1, 0x114, 0x0FA, 0x0EC, 0x0E5, 0x0F2, 0x100, 0x11A, 0x11B, 0x103,
    0x104, 0x0FC, 0x0FD, 0x11E, 0x0FE, 0x11F, 0x11D, 0x11C, 0x0E6, 0x120, 0x0F7, 0x0F3, 0x146, 0x148, 0x145, 0x147,
};

static const u16 items_badges_unused[] = {
    0xF0,  0xF6,  0x101, 0x102, 0x10E, 0x10F, 0x110, 0x115, 0x118, 0x119, 0x122, 0x125, 0x12A,
    0x12B, 0x12D, 0x131, 0x137, 0x139, 0x13B, 0x13D, 0x13E, 0x13F, 0x142, 0x14B, 0x14C, 0x14E,
    0x14F, 0x150, 0x149, 0x151, 0x152, 0x153, 0x154, 0xEA,  0x127, 0x128, 0x12C,
};

static const u16 items_misc[] = {
    0x1,   0x2,   0x3,   0x4,   0x5,   0x6,   0x5D,  0x5E,  0x5F,  0x60,  0x61,  0x62,  0x63,  0x64,
    0x65,  0x66,  0x155, 0x156, 0x157, 0x158, 0x159, 0x15A, 0x15B, 0x15C, 0x15D, 0x15E, 0x15F, 0x160,
    0x161, 0x162, 0x163, 0x164, 0x165, 0x166, 0x167, 0x168, 0x169, 0x16A, 0x16B, 0x16C,
};

static struct menu item_selection_menu;
static struct menu_item *item_selection_menu_return;
static struct menu_item *item_selection_menu_tab;

static u16 *item_slot_to_update;

static s32 badge_proc_switch(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    u32 badge_id = (u32)data;
    u16 *badge_list = pm_player.player_data.badges;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        for (u16 i = 0; i < 128; i++) {
            if (badge_list[i] == 0) {
                badge_list[i] = badge_id;
                break;
            }
        }
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        for (u16 i = 0; i < 128; i++) {
            if (badge_list[i] == badge_id) {
                badge_list[i] = 0;
            }
        }
        for (u16 i = 0; i < 64; i++) {
            if (pm_player.player_data.equipped_badges[i] == badge_id) {
                pm_player.player_data.equipped_badges[i] = 0;
            }
        }
    } else if (reason == MENU_CALLBACK_THINK) {
        _Bool has_badge = 0;
        for (u16 i = 0; i < 128; i++) {
            if (badge_list[i] == badge_id) {
                has_badge = 1;
                break;
            }
        }
        menu_switch_set(item, has_badge);
    }
    return 0;
}

static void tab_prev_proc(struct menu_item *item, void *data) {
    menu_tab_previous(data);
}

static void tab_next_proc(struct menu_item *item, void *data) {
    menu_tab_next(data);
}

static void item_list_button_proc(struct menu_item *item, void *data) {
    item_slot_to_update = menu_item_button_get_slot(item);
    enum item_type item_type = menu_item_button_get_type(item);
    s32 last_page = menu_tab_get_current_tab(item_selection_menu_tab);
    menu_enter(fp.main_menu, &item_selection_menu);
    switch (item_type) {
        case ITEM_TYPE_NORMAL:
        case ITEM_TYPE_STORED:
            if (last_page != 0 && last_page != 1) {
                menu_select(&item_selection_menu, item_selection_menu_return);
                menu_tab_goto(item_selection_menu_tab, 0);
            }
            break;
        case ITEM_TYPE_KEY:
            if (last_page != 2 && last_page != 3) {
                menu_select(&item_selection_menu, item_selection_menu_return);
                menu_tab_goto(item_selection_menu_tab, 2);
            }
            break;
        case ITEM_TYPE_BADGE:
            if (last_page != 4 && last_page != 5) {
                menu_select(&item_selection_menu, item_selection_menu_return);
                menu_tab_goto(item_selection_menu_tab, 4);
            }
            break;
    }
    return;
}

static void item_selection_button_proc(struct menu_item *item, void *data) {
    u16 item_id = (u32)data;
    *item_slot_to_update = item_id;
    menu_return(&item_selection_menu);
    return;
}

static void item_delete_button_proc(struct menu_item *item, void *data) {
    *item_slot_to_update = 0;
    menu_return(&item_selection_menu);
    return;
}

static void create_item_selection_page(struct menu *page, const char *title, struct gfx_texture *item_texture_list[],
                                      const u16 *item_lists[], const u16 item_list_sizes[], u16 item_list_count) {
    menu_init(page, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_add_static(page, 12, 0, title, 0xC0C0C0);
    const s32 row_size = 12;
    const s32 base_x = 1;
    const s32 base_y = 5;
    const s32 spacing = 2;
    const f32 scale = 0.6f;

    s32 total_item_count = 0;
    for (s32 i = 0; i < item_list_count; i++) {
        total_item_count += item_list_sizes[i];
    }
    struct menu_item **item_buttons = malloc(sizeof(*item_buttons) * total_item_count);
    s32 item_button_idx = 0;
    s32 start_y = base_y;
    for (s32 i_lists = 0; i_lists < item_list_count; i_lists++) {
        s32 final_item_y = base_y;
        for (s32 i_pos = 0; i_pos < item_list_sizes[i_lists]; i_pos++) {
            const s32 item_x = base_x + (i_pos % row_size) * spacing;
            const s32 item_y = start_y + (i_pos / row_size) * spacing;
            final_item_y = item_y;

            u16 item_id = item_lists[i_lists][i_pos];
            struct gfx_texture *icon = item_texture_list[item_id];
            struct menu_item *button = menu_add_button_icon(page, item_x, item_y, icon, 0, 0, 0xFFFFFF, scale,
                                                            item_selection_button_proc, (void *)(u32)item_id);
            button->tooltip = item_names[item_id];
            item_buttons[item_button_idx++] = button;
        }
        start_y = final_item_y + spacing + 1;
    }
    menu_item_create_chain(item_buttons, total_item_count, MENU_NAVIGATE_RIGHT, 1, 0);
    menu_item_create_chain(item_buttons, total_item_count, MENU_NAVIGATE_LEFT, 1, 1);
    free(item_buttons);
}

void create_item_selection_menu(struct gfx_texture *item_texture_list[]) {
    const s32 page_count = 7;

    struct menu *menu = &item_selection_menu;
    menu_init(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    item_selection_menu_return = menu->selector = menu_add_submenu(menu, 0, 0, NULL, "return");
    menu_add_button(menu, 0, 2, "delete", item_delete_button_proc, NULL);
    menu_add_tooltip(menu, 8, 2, fp.main_menu, 0xC0C0C0);

    struct menu *pages = malloc(sizeof(*pages) * page_count);
    item_selection_menu_tab = menu_add_tab(menu, 0, 0, pages, page_count);

    s32 page_index = 0;

    const u16 *list_items_1[] = {items_normal_food, items_normal_other};
    const u16 *list_items_2[] = {items_normal_tayce, items_normal_unused};
    const u16 *list_key_1[] = {items_key_required, items_key_other};
    const u16 *list_key_2[] = {items_key_letters, items_key_unused};
    const u16 *list_badges_1[] = {items_badges};
    const u16 *list_badges_2[] = {items_badges_unused};
    const u16 *list_misc[] = {items_misc};

    const u16 sizes_items_1[] = {ARRAY_LENGTH(items_normal_food), ARRAY_LENGTH(items_normal_other)};
    const u16 sizes_items_2[] = {ARRAY_LENGTH(items_normal_tayce), ARRAY_LENGTH(items_normal_unused)};
    const u16 sizes_key_1[] = {ARRAY_LENGTH(items_key_required), ARRAY_LENGTH(items_key_other)};
    const u16 sizes_key_2[] = {ARRAY_LENGTH(items_key_letters), ARRAY_LENGTH(items_key_unused)};
    const u16 sizes_badges_1[] = {ARRAY_LENGTH(items_badges)};
    const u16 sizes_badges_2[] = {ARRAY_LENGTH(items_badges_unused)};
    const u16 sizes_misc[] = {ARRAY_LENGTH(items_misc)};

    create_item_selection_page(&pages[page_index++], "items (1/2)", item_texture_list, list_items_1, sizes_items_1,
                               ARRAY_LENGTH(list_items_1));
    create_item_selection_page(&pages[page_index++], "items (2/2)", item_texture_list, list_items_2, sizes_items_2,
                               ARRAY_LENGTH(list_items_2));
    create_item_selection_page(&pages[page_index++], "key items (1/2)", item_texture_list, list_key_1, sizes_key_1,
                               ARRAY_LENGTH(list_key_1));
    create_item_selection_page(&pages[page_index++], "key items (2/2)", item_texture_list, list_key_2, sizes_key_2,
                               ARRAY_LENGTH(list_key_2));
    create_item_selection_page(&pages[page_index++], "badges (1/2)", item_texture_list, list_badges_1,
                               sizes_badges_1, ARRAY_LENGTH(list_badges_1));
    create_item_selection_page(&pages[page_index++], "badges (2/2)", item_texture_list, list_badges_2,
                               sizes_badges_2, ARRAY_LENGTH(list_badges_2));
    create_item_selection_page(&pages[page_index++], "misc (1/1)", item_texture_list, list_misc, sizes_misc,
                               ARRAY_LENGTH(list_misc));

    menu_tab_goto(item_selection_menu_tab, 0);
    menu_add_button(menu, 8, 0, "<", tab_prev_proc, item_selection_menu_tab);
    menu_add_button(menu, 10, 0, ">", tab_next_proc, item_selection_menu_tab);
}

static void create_items_menu(struct menu *menu, enum item_type item_type, struct gfx_texture *item_texture_list[],
                              f32 icon_scale, s32 item_count, s32 row_size, s32 page_size, s32 spacing_x,
                              s32 spacing_y) {
    menu->selector = menu_add_submenu(menu, 0, 0, NULL, "return");
    const s32 base_x = 2;
    const s32 base_y = 3;
    const s32 page_count = (item_count + (page_size - 1)) / page_size;
    const s32 tooltip_x = page_count > 1 ? 18 : 8;

    menu_add_tooltip(menu, tooltip_x, 0, fp.main_menu, 0xC0C0C0);

    struct menu *pages = malloc(sizeof(*pages) * page_count);
    struct menu_item *tab = menu_add_tab(menu, 0, 0, pages, page_count);
    for (s32 i_page = 0; i_page < page_count; i_page++) {
        struct menu_item *item_buttons[page_size];
        struct menu *page = &pages[i_page];
        menu_init(page, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
        if (page_count > 1) {
            char buffer[8];
            sprintf(buffer, "(%ld/%ld)", i_page + 1, page_count);
            menu_add_static(page, 12, 0, buffer, 0xC0C0C0);
        }

        for (s32 i_pos = 0; i_pos < page_size; i_pos++) {
            const s32 item_x = base_x + (i_pos % row_size) * spacing_x;
            const s32 item_y = base_y + (i_pos / row_size) * spacing_y;
            const s32 item_idx = (i_page * page_size) + i_pos;

            item_buttons[i_pos] = menu_add_item_button(page, item_x, item_y, item_texture_list, item_type, item_idx,
                                                       icon_scale, item_list_button_proc, menu);
        }
        menu_item_create_chain(item_buttons, page_size, MENU_NAVIGATE_RIGHT, 1, 0);
        menu_item_create_chain(item_buttons, page_size, MENU_NAVIGATE_LEFT, 1, 1);
    }
    menu_tab_goto(tab, 0);
    if (page_count > 1) {
        menu_add_button(menu, 8, 0, "<", tab_prev_proc, tab);
        menu_add_button(menu, 10, 0, ">", tab_next_proc, tab);
    }
}

void create_normal_items_menu(struct menu *menu, struct gfx_texture **item_texture_list) {
    create_items_menu(menu, ITEM_TYPE_NORMAL, item_texture_list, 0.8f, 10, 5, 10, 4, 4);
}

void create_key_items_menu(struct menu *menu, struct gfx_texture **item_texture_list) {
    create_items_menu(menu, ITEM_TYPE_KEY, item_texture_list, 0.7f, 32, 8, 32, 3, 3);
}

void create_stored_items_menu(struct menu *menu, struct gfx_texture **item_texture_list) {
    create_items_menu(menu, ITEM_TYPE_STORED, item_texture_list, 0.7f, 32, 8, 32, 3, 3);
}

void create_badges_menu(struct menu *menu, struct gfx_texture **item_texture_list) {
    struct menu_item *return_item = menu->selector = menu_add_submenu(menu, 0, 0, NULL, "return");
    menu_add_tooltip(menu, 8, 0, fp.main_menu, 0xC0C0C0);
    const u8 base_x = 1;
    const u8 base_y = 2;
    const u8 row_width = 10;
    const u8 spacing_x = 2;
    const u8 spacing_y = 2;
    struct menu_item *badge_items[80];

    for (s32 i = 0; i < 80; i++) {
        u8 badge_x = base_x + (i % row_width) * spacing_x;
        u8 badge_y = base_y + (i / row_width) * spacing_y;
        u32 item_id = items_badges[i];
        struct gfx_texture *badge_icon = resource_load_pmicon_item(item_id, 0);
        badge_items[i] = menu_add_switch(menu, badge_x, badge_y, badge_icon, 0, 0, 0xFFFFFF, badge_icon, 0, 1, 0xFFFFFF,
                                         0.6f, 0, badge_proc_switch, (void *)item_id);
        badge_items[i]->tooltip = item_names[item_id];
    }
    menu_item_add_chain_link(return_item, badge_items[0], MENU_NAVIGATE_DOWN);
    menu_item_add_chain_link(badge_items[0], return_item, MENU_NAVIGATE_UP);
    menu_item_create_chain(badge_items, 80, MENU_NAVIGATE_RIGHT, 1, 0);
    menu_item_create_chain(badge_items, 80, MENU_NAVIGATE_LEFT, 1, 1);

    static struct menu full_badge_list;
    menu_init(&full_badge_list, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    struct menu_item *full_list_item = menu_add_submenu(menu, 0, 18, &full_badge_list, "full list");
    create_items_menu(&full_badge_list, ITEM_TYPE_BADGE, item_texture_list, 0.7f, 128, 8, 32, 3, 3);

    menu_item_add_chain_link(full_list_item, badge_items[70], MENU_NAVIGATE_UP);
    menu_item_add_chain_link(badge_items[70], full_list_item, MENU_NAVIGATE_DOWN);
}
