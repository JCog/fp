#include <stdlib.h>
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

static s32 halfword_mod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    u16 *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != *p) {
            menu_intinput_set(item, *p);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        *p = menu_intinput_get(item);
    }
    return 0;
}

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

static s32 halfword_optionmod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    u16 *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != *p) {
            menu_option_set(item, *p);
        }
    } else if (reason == MENU_CALLBACK_DEACTIVATE) {
        *p = menu_option_get(item);
    }
    return 0;
}

static s32 checkbox_mod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    u8 *p = data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        *p = 1;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        *p = 0;
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, *p);
    }
    return 0;
}

static s32 max_hp_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != pm_player.player_data.max_hp) {
            menu_intinput_set(item, pm_player.player_data.max_hp);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        pm_player.player_data.max_hp = menu_intinput_get(item);
        pm_player.player_data.menu_max_hp = menu_intinput_get(item);
    }
    return 0;
}

static s32 max_fp_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != pm_player.player_data.max_fp) {
            menu_intinput_set(item, pm_player.player_data.max_fp);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        pm_player.player_data.max_fp = menu_intinput_get(item);
        pm_player.player_data.menu_max_fp = menu_intinput_get(item);
    }
    return 0;
}

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

static s32 item_int_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    u16 *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != *p) {
            menu_intinput_set(item, *p);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        if (menu_intinput_get(item) > 0x16c) {
            *p = 0x16c;
        } else {
            *p = menu_intinput_get(item);
        }
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

    game_icon *boots_normal_on = game_icons_create_item(ITEM_JUMP, 0);
    game_icon *boots_normal_off = game_icons_create_item(ITEM_JUMP, 1);
    game_icon *boots_super_on = game_icons_create_item(ITEM_SPIN_JUMP, 0);
    game_icon *boots_super_off = game_icons_create_item(ITEM_SPIN_JUMP, 1);
    game_icon *boots_ultra_on = game_icons_create_item(ITEM_TORNADO_JUMP, 0);
    game_icon *boots_ultra_off = game_icons_create_item(ITEM_TORNADO_JUMP, 1);
    game_icons_set_scale(boots_normal_on, 0.7f);
    game_icons_set_scale(boots_normal_off, 0.7f);
    game_icons_set_scale(boots_super_on, 0.7f);
    game_icons_set_scale(boots_super_off, 0.7f);
    game_icons_set_scale(boots_ultra_on, 0.7f);
    game_icons_set_scale(boots_ultra_off, 0.7f);

    game_icon *hammer_normal_on = game_icons_create_item(ITEM_HAMMER, 0);
    game_icon *hammer_normal_off = game_icons_create_item(ITEM_HAMMER, 1);
    game_icon *hammer_super_on = game_icons_create_item(ITEM_SUPER_HAMMER, 0);
    game_icon *hammer_super_off = game_icons_create_item(ITEM_SUPER_HAMMER, 1);
    game_icon *hammer_ultra_on = game_icons_create_item(ITEM_ULTRA_HAMMER, 0);
    game_icon *hammer_ultra_off = game_icons_create_item(ITEM_ULTRA_HAMMER, 1);
    game_icons_set_scale(hammer_normal_on, 0.7f);
    game_icons_set_scale(hammer_normal_off, 0.7f);
    game_icons_set_scale(hammer_super_on, 0.7f);
    game_icons_set_scale(hammer_super_off, 0.7f);
    game_icons_set_scale(hammer_ultra_on, 0.7f);
    game_icons_set_scale(hammer_ultra_off, 0.7f);

    struct menu_item *item;
    item = menu_add_switch_game_icon(menu, base_x, base_y, boots_normal_on, boots_normal_off, boots_proc, (void *)0);
    item->tooltip = boots_normal_str;
    item = menu_add_switch_game_icon(menu, base_x + 3, base_y, boots_super_on, boots_super_off, boots_proc, (void *)1);
    item->tooltip = boots_super_str;
    item = menu_add_switch_game_icon(menu, base_x + 6, base_y, boots_ultra_on, boots_ultra_off, boots_proc, (void *)2);
    item->tooltip = boots_ultra_str;

    item = menu_add_switch_game_icon(menu, base_x, base_y + 3, hammer_normal_on, hammer_normal_off, hammer_proc,
                                     (void *)0);
    item->tooltip = hammer_normal_str;
    item = menu_add_switch_game_icon(menu, base_x + 3, base_y + 3, hammer_super_on, hammer_super_off, hammer_proc,
                                     (void *)1);
    item->tooltip = hammer_super_str;
    item = menu_add_switch_game_icon(menu, base_x + 6, base_y + 3, hammer_ultra_on, hammer_ultra_off, hammer_proc,
                                     (void *)2);
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

    for (u32 i = 0; i < 8; i++) {
        game_icon *partner_on = game_icons_create_partner(partner_order[i + 1], 0);
        game_icon *partner_off = game_icons_create_partner(partner_order[i + 1], 1);
        game_icons_set_scale(partner_on, 0.9f);
        game_icons_set_scale(partner_off, 0.9f);
        game_icon *super_rank_on = game_icons_create_global(ICON_PARTNER_RANK_1_A, 0);
        game_icon *super_rank_off = game_icons_create_global(ICON_PARTNER_RANK_1_A, 1);
        game_icon *ultra_rank_on = game_icons_create_global(ICON_PARTNER_RANK_1_A, 0);
        game_icon *ultra_rank_off = game_icons_create_global(ICON_PARTNER_RANK_1_A, 1);
        game_icons_set_pos_offset(super_rank_on, 1, 1);
        game_icons_set_pos_offset(super_rank_off, 1, 1);
        game_icons_set_pos_offset(ultra_rank_on, 1, 1);
        game_icons_set_pos_offset(ultra_rank_off, 1, 1);

        u8 partner_x = base_x + (i % width) * spacing_x;
        u8 partner_y = base_y + (i / width) * spacing_y;

        // partner
        partners[i] = menu_add_switch_game_icon(menu, partner_x, partner_y, partner_on, partner_off, in_party_proc,
                                                &pm_player.player_data.partners[partner_order[i + 1]]);
        partners[i]->tooltip = partner_names[i];

        // super rank
        super_ranks[i] =
            menu_add_switch_game_icon(menu, partner_x + -1, partner_y + 3, super_rank_on, super_rank_off,
                                      super_rank_proc, &pm_player.player_data.partners[partner_order[i + 1]]);
        menu_item_add_chain_link(partners[i], super_ranks[i], MENU_NAVIGATE_DOWN);
        menu_item_add_chain_link(super_ranks[i], partners[i], MENU_NAVIGATE_UP);
        super_ranks[i]->tooltip = super_rank_str;

        // ultra rank
        ultra_ranks[i] =
            menu_add_switch_game_icon(menu, partner_x + 1, partner_y + 3, ultra_rank_on, ultra_rank_off,
                                      ultra_rank_proc, &pm_player.player_data.partners[partner_order[i + 1]]);
        menu_item_add_chain_link(ultra_ranks[i], partners[i], MENU_NAVIGATE_UP);
        ultra_ranks[i]->tooltip = ultra_rank_str;
    }

    menu_item_add_chain_link(menu->selector, partners[0], MENU_NAVIGATE_DOWN);
    menu_item_create_chain(partners, 8, MENU_NAVIGATE_RIGHT, 1, 0);
    menu_item_create_chain(partners, 8, MENU_NAVIGATE_LEFT, 1, 1);
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

        create_mario_menu(&mario);
        create_party_menu(&party);
        create_badges_menu(&badges);
        create_normal_items_menu(&items);
        create_key_items_menu(&key_items);
        create_stored_items_menu(&stored_items);
        create_item_selection_menu();
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
