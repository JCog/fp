#include <stdlib.h>
#include "fp.h"
#include "menu.h"
#include "settings.h"
#include "gfx.h"
#include "resource.h"
#include "items.h"

static const char *boots_normal_str = "Normal Boots";
static const char *boots_super_str = "Super Boots";
static const char *boots_ultra_str = "Ultra Boots";
static const char *hammer_normal_str = "Normal Hammer";
static const char *hammer_super_str = "Super Hammer";
static const char *hammer_ultra_str = "Ultra Hammer";
static const char *partner_names[] = {
    "goombario", "kooper",     "bombette", "parakarry", "bow",   "watt",
    "sushie",    "lakilester", "goompa",   "goombaria", "twink",
};
static const u32 partner_order[] = {
    0, 1, 2, 3, 4, 9, 6, 7, 8, 5, 10, 11,
};
static const char *super_rank_str = "super rank";
static const char *ultra_rank_str = "ultra rank";

static struct gfx_texture *item_texture_list[0x16D];

// static s32 halfword_mod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
//     u16 *p = data;
//     if (reason == MENU_CALLBACK_THINK_INACTIVE) {
//         if (menu_intinput_get(item) != *p) {
//             menu_intinput_set(item, *p);
//         }
//     } else if (reason == MENU_CALLBACK_CHANGED) {
//         *p = menu_intinput_get(item);
//     }
//     return 0;
// }

static s32 byte_mod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    u8 *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != *p) {
            menu_intinput_set(item, *p);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        *p = menu_intinput_get(item);
    }
    return 0;
}

static s32 byte_optionmod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    u8 *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != *p) {
            menu_option_set(item, *p);
        }
    } else if (reason == MENU_CALLBACK_DEACTIVATE) {
        *p = menu_option_get(item);
    }
    return 0;
}

// static s32 halfword_optionmod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
//     u16 *p = data;
//     if (reason == MENU_CALLBACK_THINK_INACTIVE) {
//         if (menu_option_get(item) != *p) {
//             menu_option_set(item, *p);
//         }
//     } else if (reason == MENU_CALLBACK_DEACTIVATE) {
//         *p = menu_option_get(item);
//     }
//     return 0;
// }
//
// static s32 checkbox_mod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
//     u8 *p = data;
//     if (reason == MENU_CALLBACK_SWITCH_ON) {
//         *p = 1;
//     } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
//         *p = 0;
//     } else if (reason == MENU_CALLBACK_THINK) {
//         menu_checkbox_set(item, *p);
//     }
//     return 0;
// }
//
// static s32 max_hp_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
//     if (reason == MENU_CALLBACK_THINK_INACTIVE) {
//         if (menu_intinput_get(item) != pm_player.player_data.max_hp) {
//             menu_intinput_set(item, pm_player.player_data.max_hp);
//         }
//     } else if (reason == MENU_CALLBACK_CHANGED) {
//         pm_player.player_data.max_hp = menu_intinput_get(item);
//         pm_player.player_data.menu_max_hp = menu_intinput_get(item);
//     }
//     return 0;
// }
//
// static s32 max_fp_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
//     if (reason == MENU_CALLBACK_THINK_INACTIVE) {
//         if (menu_intinput_get(item) != pm_player.player_data.max_fp) {
//             menu_intinput_set(item, pm_player.player_data.max_fp);
//         }
//     } else if (reason == MENU_CALLBACK_CHANGED) {
//         pm_player.player_data.max_fp = menu_intinput_get(item);
//         pm_player.player_data.menu_max_fp = menu_intinput_get(item);
//     }
//     return 0;
// }

static s32 current_partner_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        menu_option_set(item, partner_order[pm_player.player_data.current_partner]);
    } else if (reason == MENU_CALLBACK_DEACTIVATE) {
        pm_player.player_data.current_partner = partner_order[menu_option_get(item)];
    }
    return 0;
}

static s32 boots_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    u32 tracked_level = (u32)data;
    u8 *boots_upgrade = &pm_player.player_data.boots_upgrade;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        *boots_upgrade = tracked_level;
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_switch_set(item, *boots_upgrade == tracked_level);
    }
    return 0;
}

static s32 hammer_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    u32 tracked_level = (u32)data;
    u8 *hammer_upgrade = &pm_player.player_data.hammer_upgrade;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        *hammer_upgrade = tracked_level;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        *hammer_upgrade = 0xFF;
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_switch_set(item, *hammer_upgrade == tracked_level);
    }
    return 0;
}

static s32 in_party_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    partner_t *partner = (partner_t *)data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        partner->in_party = 1;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        partner->in_party = 0;
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_switch_set(item, partner->in_party);
    }
    return 0;
}

static s32 super_rank_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    partner_t *partner = (partner_t *)data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        partner->upgrade = 1;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        if (partner->upgrade == 2) {
            partner->upgrade = 1;
        } else {
            partner->upgrade = 0;
        }
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_switch_set(item, partner->upgrade > 0);
    }
    return 0;
}

static s32 ultra_rank_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    partner_t *partner = (partner_t *)data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        partner->upgrade = 2;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        partner->upgrade = 0;
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_switch_set(item, partner->upgrade == 2);
    }
    return 0;
}

static s32 star_power_full_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    u8 *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != *p) {
            menu_intinput_set(item, *p);
        }
        if (pm_player.player_data.star_power.full_bars_filled >= pm_player.player_data.star_power.star_spirits_saved) {
            menu_intinput_set(item, pm_player.player_data.star_power.star_spirits_saved);
            *p = pm_player.player_data.star_power.star_spirits_saved;
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        if (menu_intinput_get(item) > pm_player.player_data.star_power.star_spirits_saved) {
            *p = pm_player.player_data.star_power.star_spirits_saved;
        } else {
            *p = menu_intinput_get(item);
        }
    }
    return 0;
}

static s32 star_power_partial_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    u8 *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != *p) {
            menu_intinput_set(item, *p);
        }
        if (pm_player.player_data.star_power.full_bars_filled >= pm_player.player_data.star_power.star_spirits_saved) {
            menu_intinput_set(item, 0);
            *p = 0;
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        if (pm_player.player_data.star_power.full_bars_filled > 6) {
            *p = 0;
        } else {
            *p = menu_intinput_get(item);
        }
    }
    return 0;
}

static s32 peach_or_mario_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_status.peach_flags |= (1 << 0);
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_status.peach_flags &= ~(1 << 0);
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_status.peach_flags & (1 << 0));
    }
    return 0;
}

static s32 peach_transformed_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_status.peach_flags |= (1 << 1);
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_status.peach_flags &= ~(1 << 1);
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_status.peach_flags & (1 << 1));
    }
    return 0;
}

static s32 peach_parasol_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_status.peach_flags |= (1 << 2);
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_status.peach_flags &= ~(1 << 2);
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_status.peach_flags & (1 << 2));
    }
    return 0;
}

static void create_mario_menu(struct menu *menu) {
    menu->selector = menu_add_submenu(menu, 0, 0, NULL, "return");
    menu_add_tooltip(menu, 7, 0, fp.main_menu, 0xC0C0C0);

    const u8 base_x = 1;
    const u8 base_y = 2;

    struct gfx_texture *boots_normal = resource_load_pmicon_item(ITEM_JUMP);
    struct gfx_texture *boots_super = resource_load_pmicon_item(ITEM_SPIN_JUMP);
    struct gfx_texture *boots_ultra = resource_load_pmicon_item(ITEM_TORNADO_JUMP);
    struct gfx_texture *hammer_normal = resource_load_pmicon_item(ITEM_HAMMER);
    struct gfx_texture *hammer_super = resource_load_pmicon_item(ITEM_SUPER_HAMMER);
    struct gfx_texture *hammer_ultra = resource_load_pmicon_item(ITEM_ULTRA_HAMMER);
    gfx_add_grayscale_palette(boots_normal, 0);
    gfx_add_grayscale_palette(boots_super, 0);
    gfx_add_grayscale_palette(boots_ultra, 0);
    gfx_add_grayscale_palette(hammer_normal, 0);
    gfx_add_grayscale_palette(hammer_super, 0);
    gfx_add_grayscale_palette(hammer_ultra, 0);

    struct menu_item *item;
    item = menu_add_switch(menu, base_x, base_y, boots_normal, 0, 0, 0xFFFFFF, boots_normal, 0, 1, 0xFFFFFF, 0.7f, 0,
                           boots_proc, (void *)0);
    item->tooltip = boots_normal_str;
    item = menu_add_switch(menu, base_x + 3, base_y, boots_super, 0, 0, 0xFFFFFF, boots_super, 0, 1, 0xFFFFFF, 0.7f, 0,
                           boots_proc, (void *)1);
    item->tooltip = boots_super_str;
    item = menu_add_switch(menu, base_x + 6, base_y, boots_ultra, 0, 0, 0xFFFFFF, boots_ultra, 0, 1, 0xFFFFFF, 0.7f, 0,
                           boots_proc, (void *)2);
    item->tooltip = boots_ultra_str;

    item = menu_add_switch(menu, base_x, base_y + 3, hammer_normal, 0, 0, 0xFFFFFF, hammer_normal, 0, 1, 0xFFFFFF, 0.7f,
                           0, hammer_proc, (void *)0);
    item->tooltip = hammer_normal_str;
    item = menu_add_switch(menu, base_x + 3, base_y + 3, hammer_super, 0, 0, 0xFFFFFF, hammer_super, 0, 1, 0xFFFFFF,
                           0.7f, 0, hammer_proc, (void *)1);
    item->tooltip = hammer_super_str;
    item = menu_add_switch(menu, base_x + 6, base_y + 3, hammer_ultra, 0, 0, 0xFFFFFF, hammer_ultra, 0, 1, 0xFFFFFF,
                           0.7f, 0, hammer_proc, (void *)2);
    item->tooltip = hammer_ultra_str;
}

static void create_party_menu(struct menu *menu) {
    menu->selector = menu_add_submenu(menu, 0, 0, NULL, "return");
    menu_add_tooltip(menu, 7, 0, fp.main_menu, 0xC0C0C0);
    const u8 base_x = 2;
    const u8 base_y = 3;
    const u8 width = 4;
    const u8 spacing_x = 6;
    const u8 spacing_y = 7;
    struct menu_item *partners[8];
    struct menu_item *super_ranks[8];
    struct menu_item *ultra_ranks[8];
    struct menu_item *all_ranks[16];
    struct gfx_texture *partner = resource_get(RES_PMICON_PARTNER);
    struct gfx_texture *rank = resource_load_pmicon_global(ICON_PARTNER_RANK_1_A, 1);
    gfx_add_grayscale_palette(rank, 0);
    gfx_texture_translate(rank, 0, 3, 3);

    for (u32 i = 0; i < 8; i++) {
        u8 partner_x = base_x + (i % width) * spacing_x;
        u8 partner_y = base_y + (i / width) * spacing_y;

        partners[i] =
            menu_add_switch(menu, partner_x, partner_y, partner, i + 1, 0, 0xFFFFFF, partner, i + 1, 1, 0xFFFFFF, 1.0f,
                            0, in_party_proc, &pm_player.player_data.partners[partner_order[i + 1]]);
        partners[i]->tooltip = partner_names[i];

        // super rank
        super_ranks[i] =
            menu_add_switch(menu, partner_x - 1, partner_y + 3, rank, 0, 0, 0xFFFFFF, rank, 0, 1, 0xFFFFFF, 1.0f, 0,
                            super_rank_proc, &pm_player.player_data.partners[partner_order[i + 1]]);
        menu_item_add_chain_link(partners[i], super_ranks[i], MENU_NAVIGATE_DOWN);
        menu_item_add_chain_link(super_ranks[i], partners[i], MENU_NAVIGATE_UP);
        super_ranks[i]->tooltip = super_rank_str;
        all_ranks[i * 2] = super_ranks[i];

        // ultra rank
        ultra_ranks[i] =
            menu_add_switch(menu, partner_x + 1, partner_y + 3, rank, 0, 0, 0xFFFFFF, rank, 0, 1, 0xFFFFFF, 1.0f, 0,
                            ultra_rank_proc, &pm_player.player_data.partners[partner_order[i + 1]]);
        menu_item_add_chain_link(ultra_ranks[i], partners[i], MENU_NAVIGATE_UP);
        ultra_ranks[i]->tooltip = ultra_rank_str;
        all_ranks[i * 2 + 1] = ultra_ranks[i];
    }

    menu_item_add_chain_link(menu->selector, partners[0], MENU_NAVIGATE_DOWN);
    menu_item_create_chain(partners, 8, MENU_NAVIGATE_RIGHT, 1, 0);
    menu_item_create_chain(partners, 8, MENU_NAVIGATE_LEFT, 1, 1);
    menu_item_create_chain(all_ranks, 16, MENU_NAVIGATE_RIGHT, 1, 0);
    menu_item_create_chain(all_ranks, 16, MENU_NAVIGATE_LEFT, 1, 1);
    for (s32 i = 0; i < 4; i++) {
        menu_item_add_chain_link(super_ranks[i], partners[i + 4], MENU_NAVIGATE_DOWN);
        menu_item_add_chain_link(ultra_ranks[i], partners[i + 4], MENU_NAVIGATE_DOWN);
        menu_item_add_chain_link(partners[i + 4], super_ranks[i], MENU_NAVIGATE_UP);
    }

    menu_add_static(menu, 0, 15, "current", 0xC0C0C0);
    menu_add_option(menu, 8, 15,
                    "none\0"
                    "goombario\0"
                    "kooper\0"
                    "bombette\0"
                    "parakarry\0"
                    "bow\0"
                    "watt\0"
                    "sushie\0"
                    "lakilester\0"
                    "goompa\0"
                    "goombaria\0"
                    "twink\0",
                    current_partner_proc, NULL);
}

struct menu *create_player_menu(void) {
    static struct menu menu;
    static struct menu mario;
    static struct menu party;
    static struct menu items;
    static struct menu key_items;
    static struct menu stored_items;
    static struct menu badges;
    static struct menu star_power;
    static struct menu peach;
    static struct menu merlee;

    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&mario, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&party, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&items, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&key_items, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&stored_items, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&badges, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&star_power, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&peach, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&merlee, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    {
        s32 y = 0;
        menu.selector = menu_add_submenu(&menu, 0, y++, NULL, "return");

        /*build player menu*/
        menu_add_submenu(&menu, 0, y++, &mario, "mario");
        menu_add_submenu(&menu, 0, y++, &party, "party");
        menu_add_submenu(&menu, 0, y++, &badges, "badges");
        menu_add_submenu(&menu, 0, y++, &items, "items");
        menu_add_submenu(&menu, 0, y++, &key_items, "key items");
        menu_add_submenu(&menu, 0, y++, &stored_items, "stored items");
        menu_add_submenu(&menu, 0, y++, &star_power, "star power");
        menu_add_submenu(&menu, 0, y++, &peach, "princess peach");
        menu_add_submenu(&menu, 0, y++, &merlee, "merlee");

        for (u16 i = 0; i < 0x16D; i++) {
            item_texture_list[i] = resource_load_pmicon_item(i);
        }
        gfx_add_grayscale_palette(item_texture_list[ITEM_KOOPA_FORTRESS_KEY], 0);

        create_mario_menu(&mario);
        create_party_menu(&party);
        create_badges_menu(&badges, item_texture_list);
        create_normal_items_menu(&items, item_texture_list);
        create_key_items_menu(&key_items, item_texture_list);
        create_stored_items_menu(&stored_items, item_texture_list);
        create_item_selection_menu(item_texture_list);
    }

    {
        /*build mario menu*/
        //
        // menu_add_static(&mario, 0, y_value, "action commands", 0xC0C0C0);
        // menu_add_checkbox(&mario, STATS_X, y_value++, checkbox_mod_proc, &pm_player.player_data.has_action_command);
        //
        // menu_add_static(&mario, 0, y_value, "hp", 0xC0C0C0);
        // menu_add_intinput(&mario, STATS_X, y_value, 10, 2, byte_mod_proc, &pm_player.player_data.hp);
        // menu_add_static(&mario, STATS_X + 2, y_value, "/", 0xC0C0C0);
        // menu_add_intinput(&mario, STATS_X + 3, y_value++, 10, 2, max_hp_proc, NULL);
        //
        // menu_add_static(&mario, 0, y_value, "fp", 0xC0C0C0);
        // menu_add_intinput(&mario, STATS_X, y_value, 10, 2, byte_mod_proc, &pm_player.player_data.fp);
        // menu_add_static(&mario, STATS_X + 2, y_value, "/", 0xC0C0C0);
        // menu_add_intinput(&mario, STATS_X + 3, y_value++, 10, 2, max_fp_proc, NULL);
        //
        // menu_add_static(&mario, 0, y_value, "bp", 0xC0C0C0);
        // menu_add_intinput(&mario, STATS_X, y_value++, 10, 2, byte_mod_proc, &pm_player.player_data.bp);
        //
        // menu_add_static(&mario, 0, y_value, "level", 0xC0C0C0);
        // menu_add_intinput(&mario, STATS_X, y_value++, 10, 2, byte_mod_proc, &pm_player.player_data.level);
        //
        // menu_add_static(&mario, 0, y_value, "star points", 0xC0C0C0);
        // menu_add_intinput(&mario, STATS_X, y_value++, 10, 2, byte_mod_proc, &pm_player.player_data.star_points);
        //
        // menu_add_static(&mario, 0, y_value, "star pieces", 0xC0C0C0);
        // menu_add_intinput(&mario, STATS_X, y_value++, 10, 2, byte_mod_proc, &pm_player.player_data.star_pieces);
        //
        // menu_add_static(&mario, 0, y_value, "coins", 0xC0C0C0);
        // menu_add_intinput(&mario, STATS_X, y_value++, 10, 3, halfword_mod_proc, &pm_player.player_data.coins);
    }

    {
        /*build star power menu*/
        const s32 STAR_POWER_X = 19;
        s32 y_value = 0;

        star_power.selector = menu_add_submenu(&star_power, 0, y_value++, NULL, "return");

        menu_add_static(&star_power, 0, y_value, "star spirits saved", 0xC0C0C0);
        menu_add_intinput(&star_power, STAR_POWER_X, y_value++, 10, 1, byte_mod_proc,
                          &pm_player.player_data.star_power.star_spirits_saved);

        menu_add_static(&star_power, 0, y_value, "bars full", 0xC0C0C0);
        menu_add_intinput(&star_power, STAR_POWER_X, y_value++, 10, 1, star_power_full_proc,
                          &pm_player.player_data.star_power.full_bars_filled);

        menu_add_static(&star_power, 0, y_value, "partial bar", 0xC0C0C0);
        menu_add_intinput(&star_power, STAR_POWER_X, y_value++, 16, 2, star_power_partial_proc,
                          &pm_player.player_data.star_power.partial_bars_filled);

        menu_add_static(&star_power, 0, y_value, "beam", 0xC0C0C0);
        menu_add_option(&star_power, STAR_POWER_X, y_value++,
                        "none\0"
                        "star beam\0"
                        "peach beam\0",
                        byte_optionmod_proc, &pm_player.player_data.star_power.beam_rank);
    }

    {
        /*build peach menu*/
        const s32 PEACH_X = 12;
        s32 y_value = 0;

        peach.selector = menu_add_submenu(&peach, 0, y_value++, NULL, "return");

        menu_add_static(&peach, 0, y_value, "peach", 0xC0C0C0);
        menu_add_checkbox(&peach, PEACH_X, y_value++, peach_or_mario_proc, NULL);

        menu_add_static(&peach, 0, y_value, "transformed", 0xC0C0C0);
        menu_add_checkbox(&peach, PEACH_X, y_value++, peach_transformed_proc, NULL);

        menu_add_static(&peach, 0, y_value, "parasol", 0xC0C0C0);
        menu_add_checkbox(&peach, PEACH_X, y_value++, peach_parasol_proc, NULL);

        menu_add_static(&peach, 0, y_value, "disguise", 0xC0C0C0);
        menu_add_option(&peach, PEACH_X, y_value++,
                        "none\0"
                        "koopatrol\0"
                        "hammer bro\0"
                        "clubba\0",
                        byte_optionmod_proc, &pm_status.peach_disguise);
    }

    {
        /*build merlee menu*/
        const s32 MERLEE_X = 16;
        s32 y_value = 0;

        merlee.selector = menu_add_submenu(&merlee, 0, y_value++, NULL, "return");

        menu_add_static(&merlee, 0, y_value, "spell type", 0xC0C0C0);
        menu_add_option(&merlee, MERLEE_X, y_value++,
                        "none\0"
                        "+3 ATK\0"
                        "+3 DEF\0"
                        "EXP x2\0"
                        "Coins x2\0",
                        byte_optionmod_proc, &pm_player.player_data.merlee.spell_type);

        menu_add_static(&merlee, 0, y_value, "casts remaining", 0xC0C0C0);
        menu_add_intinput(&merlee, MERLEE_X, y_value++, 10, 2, byte_mod_proc,
                          &pm_player.player_data.merlee.casts_remaining);

        menu_add_static(&merlee, 0, y_value, "turns remaining", 0xC0C0C0);
        menu_add_intinput(&merlee, MERLEE_X, y_value++, 10, 3, byte_mod_proc,
                          &pm_player.player_data.merlee.turns_until_spell);

        return &menu;
    }
}
