#include <stdlib.h>
#include "fp.h"
#include "menu.h"
#include "settings.h"
#include "gfx.h"
#include "resource.h"
#include "items.h"

static const char *hp_str = "HP";
static const char *max_hp_str = "Max HP";
static const char *fp_str = "FP";
static const char *max_fp_str = "Max FP";
static const char *bp_str = "BP";
static const char *coins_str = "Coins";
static const char *star_pieces_str = "Star Pieces";
static const char *level_str = "Level";
static const char *star_points_str = "Star Points";
static const char *boots_normal_str = "Normal Boots";
static const char *boots_super_str = "Super Boots";
static const char *boots_ultra_str = "Ultra Boots";
static const char *hammer_normal_str = "Normal Hammer";
static const char *hammer_super_str = "Super Hammer";
static const char *hammer_ultra_str = "Ultra Hammer";
static const char *action_commands_str = "Action Commands";
static const char *partner_names[] = {
    "Goombario", "Kooper",     "Bombette", "Parakarry", "Bow",   "Watt",
    "Sushie",    "Lakilester", "Goompa",   "Goombaria", "Twink",
};
static const u32 partner_order[] = {
    0, 1, 2, 3, 4, 9, 6, 7, 8, 5, 10, 11,
};
static const char *super_rank_str = "Super Rank";
static const char *ultra_rank_str = "Ultra Rank";

static struct gfx_texture *item_texture_list[0x16D];

static struct gfx_texture **get_item_texture_list(void) {
    static _Bool ready = 0;
    if (!ready) {
        ready = 1;
        for (u16 i = 0; i < 0x16D; i++) {
            item_texture_list[i] = resource_load_pmicon_item(i);
        }
    }
    return item_texture_list;
}

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

static s32 action_commands_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    u8 *has_action_commands = &pm_player.player_data.has_action_command;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        *has_action_commands = 1;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        *has_action_commands = 0;
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_switch_set(item, *has_action_commands);
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

static s32 star_spirit_switch_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    u8 *ss_saved = &pm_player.player_data.star_power.star_spirits_saved;
    u16 *star_power = &pm_player.player_data.star_power.total_star_power;
    u32 ss_index = (u32)data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        *ss_saved = ss_index;
        *star_power = ss_index * 0x100;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        if (*ss_saved == ss_index) {
            if (*star_power < ss_index * 0x100) {
                *star_power = ss_index * 0x100;
            } else {
                *ss_saved = 0;
                *star_power = 0;
            }
        } else {
            *ss_saved = ss_index;
            *star_power = ss_index * 0x100;
        }
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_switch_set(item, *ss_saved >= ss_index);
    }
    return 0;
}

static s32 beam_rank_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    u8 *beam_rank = &pm_player.player_data.star_power.beam_rank;
    if (reason == MENU_CALLBACK_CHANGED) {
        *beam_rank = menu_cycle_get(item);
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_cycle_set(item, *beam_rank);
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
    menu_add_tooltip(menu, 8, 0, fp.main_menu, 0xC0C0C0);

    struct gfx_texture **item_textures = get_item_texture_list();

    struct gfx_texture *tex_boots_normal = item_textures[ITEM_JUMP];
    struct gfx_texture *tex_boots_super = item_textures[ITEM_SPIN_JUMP];
    struct gfx_texture *tex_boots_ultra = item_textures[ITEM_TORNADO_JUMP];
    struct gfx_texture *tex_hammer_normal = item_textures[ITEM_HAMMER];
    struct gfx_texture *tex_hammer_super = item_textures[ITEM_SUPER_HAMMER];
    struct gfx_texture *tex_hammer_ultra = item_textures[ITEM_ULTRA_HAMMER];
    struct gfx_texture *tex_lucky_star = item_textures[ITEM_LUCKY_STAR];
    struct gfx_texture *tex_heart = resource_load_pmicon_global(ICON_STATUS_HEART, 1);
    struct gfx_texture *tex_flower = resource_load_pmicon_global(ICON_STATUS_FLOWER, 1);
    struct gfx_texture *tex_bp_icon = resource_get(RES_PMICON_BP);
    struct gfx_texture *tex_mario_head = resource_load_pmicon_global(ICON_MARIO_HEAD, 1);
    struct gfx_texture *tex_star_point = resource_load_pmicon_global(ICON_STATUS_STAR_POINT, 1);
    struct gfx_texture *tex_star_piece = resource_load_pmicon_global(ICON_STATUS_STAR_PIECE, 1);
    struct gfx_texture *tex_coin = resource_load_pmicon_global(ICON_STATUS_COIN, 1);
    gfx_add_grayscale_palette(tex_boots_normal, 0);
    gfx_add_grayscale_palette(tex_boots_super, 0);
    gfx_add_grayscale_palette(tex_boots_ultra, 0);
    gfx_add_grayscale_palette(tex_hammer_normal, 0);
    gfx_add_grayscale_palette(tex_hammer_super, 0);
    gfx_add_grayscale_palette(tex_hammer_ultra, 0);
    gfx_add_grayscale_palette(tex_lucky_star, 0);

    struct menu_item *item;
    s32 boots_x = 10;
    s32 boots_y = 2;
    item = menu_add_switch(menu, boots_x, boots_y, tex_boots_normal, 0, 0, 0xFFFFFF, tex_boots_normal, 0, 1, 0xFFFFFF,
                           0.7f, 0, boots_proc, (void *)0);
    item->tooltip = boots_normal_str;
    item = menu_add_switch(menu, boots_x + 3, boots_y, tex_boots_super, 0, 0, 0xFFFFFF, tex_boots_super, 0, 1, 0xFFFFFF,
                           0.7f, 0, boots_proc, (void *)1);
    item->tooltip = boots_super_str;
    item = menu_add_switch(menu, boots_x + 6, boots_y, tex_boots_ultra, 0, 0, 0xFFFFFF, tex_boots_ultra, 0, 1, 0xFFFFFF,
                           0.7f, 0, boots_proc, (void *)2);
    item->tooltip = boots_ultra_str;

    s32 hammer_x = 10;
    s32 hammer_y = 5;
    item = menu_add_switch(menu, hammer_x, hammer_y, tex_hammer_normal, 0, 0, 0xFFFFFF, tex_hammer_normal, 0, 1,
                           0xFFFFFF, 0.7f, 0, hammer_proc, (void *)0);
    item->tooltip = hammer_normal_str;
    item = menu_add_switch(menu, hammer_x + 3, hammer_y, tex_hammer_super, 0, 0, 0xFFFFFF, tex_hammer_super, 0, 1,
                           0xFFFFFF, 0.7f, 0, hammer_proc, (void *)1);
    item->tooltip = hammer_super_str;
    item = menu_add_switch(menu, hammer_x + 6, hammer_y, tex_hammer_ultra, 0, 0, 0xFFFFFF, tex_hammer_ultra, 0, 1,
                           0xFFFFFF, 0.7f, 0, hammer_proc, (void *)2);
    item->tooltip = hammer_ultra_str;

    item = menu_add_switch(menu, 20, 2, tex_lucky_star, 0, 0, 0xFFFFFF, tex_lucky_star, 0, 1, 0xFFFFFF, 0.7f, 0,
                           action_commands_proc, NULL);
    item->tooltip = action_commands_str;

    s32 hp_x = 1;
    s32 hp_y = 2;
    menu_add_static_icon(menu, hp_x, hp_y, tex_heart, 0, 0xFFFFFF, 1.0f);
    item = menu_add_intinput(menu, hp_x + 2, hp_y, 10, 2, byte_mod_proc, &pm_player.player_data.hp);
    item->tooltip = hp_str;
    menu_add_static(menu, hp_x + 4, hp_y, "/", 0xC0C0C0);
    item = menu_add_intinput(menu, hp_x + 5, hp_y, 10, 2, max_hp_proc, NULL);
    item->tooltip = max_hp_str;

    s32 fp_x = 1;
    s32 fp_y = 4;
    menu_add_static_icon(menu, fp_x, fp_y, tex_flower, 0, 0xFFFFFF, 1.0f);
    item = menu_add_intinput(menu, fp_x + 2, fp_y, 10, 2, byte_mod_proc, &pm_player.player_data.fp);
    item->tooltip = fp_str;
    menu_add_static(menu, fp_x + 4, fp_y, "/", 0xC0C0C0);
    item = menu_add_intinput(menu, fp_x + 5, fp_y, 10, 2, max_fp_proc, NULL);
    item->tooltip = max_fp_str;

    s32 bp_x = 1;
    s32 bp_y = 6;
    menu_add_static_icon(menu, bp_x, bp_y, tex_bp_icon, 0, 0xFFFFFF, 1.0f);
    item = menu_add_intinput(menu, bp_x + 2, bp_y, 10, 2, byte_mod_proc, &pm_player.player_data.bp);
    item->tooltip = bp_str;

    s32 coin_x = 1;
    s32 coin_y = 9;
    menu_add_static_icon(menu, coin_x, coin_y, tex_coin, 0, 0xFFFFFF, 1.0f);
    item = menu_add_intinput(menu, coin_x + 2, coin_y, 10, 3, halfword_mod_proc, &pm_player.player_data.coins);
    item->tooltip = coins_str;

    s32 star_piece_x = 1;
    s32 star_piece_y = 11;
    menu_add_static_icon(menu, star_piece_x, star_piece_y, tex_star_piece, 0, 0xFFFFFF, 1.0f);
    item = menu_add_intinput(menu, star_piece_x + 2, star_piece_y, 10, 2, byte_mod_proc,
                             &pm_player.player_data.star_pieces);
    item->tooltip = star_pieces_str;

    s32 level_x = 8;
    s32 level_y = 9;
    menu_add_static_icon(menu, level_x, level_y, tex_mario_head, 0, 0xFFFFFF, 1.0f);
    item = menu_add_intinput(menu, level_x + 2, level_y, 10, 2, byte_mod_proc, &pm_player.player_data.level);
    item->tooltip = level_str;

    s32 star_point_x = 8;
    s32 star_point_y = 11;
    menu_add_static_icon(menu, star_point_x, star_point_y, tex_star_point, 0, 0xFFFFFF, 1.0f);
    item = menu_add_intinput(menu, star_point_x + 2, star_point_y, 10, 2, byte_mod_proc,
                             &pm_player.player_data.star_points);
    item->tooltip = star_points_str;
}

static void create_party_menu(struct menu *menu) {
    struct menu_item *return_item = menu->selector = menu_add_submenu(menu, 0, 0, NULL, "return");
    menu_add_tooltip(menu, 8, 0, fp.main_menu, 0xC0C0C0);
    const s32 base_x = 1;
    const s32 base_y = 4;
    const s32 col_height = 4;
    const s32 spacing_x = 6;
    const s32 spacing_y = 4;
    const f32 scale = 0.7f;

    struct menu_item *partners[8];
    struct menu_item *super_ranks[8];
    struct menu_item *ultra_ranks[8];
    struct gfx_texture *tex_partner = resource_get(RES_PMICON_PARTNERS);
    struct gfx_texture *tex_rank = resource_load_pmicon_global(ICON_PARTNER_RANK_1_A, 1);
    gfx_add_grayscale_palette(tex_rank, 0);
    gfx_texture_translate(tex_rank, 0, 2, 2);

    menu_add_static(menu, 0, 2, "active", 0xC0C0C0);
    struct menu_item *active_item = menu_add_option(menu, 7, 2,
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
    menu_item_add_chain_link(return_item, active_item, MENU_NAVIGATE_DOWN);

    for (s32 i = 0; i < 8; i++) {
        s32 partner_x = base_x + (i / col_height) * spacing_x;
        s32 partner_y = base_y + (i % col_height) * spacing_y;

        partners[i] =
            menu_add_switch(menu, partner_x, partner_y, tex_partner, i + 1, 0, 0xFFFFFF, tex_partner, i + 1, 1,
                            0xFFFFFF, scale, 0, in_party_proc, &pm_player.player_data.partners[partner_order[i + 1]]);
        partners[i]->tooltip = partner_names[i];

        // super tex
        super_ranks[i] =
            menu_add_switch(menu, partner_x + 2, partner_y, tex_rank, 0, 0, 0xFFFFFF, tex_rank, 0, 1, 0xFFFFFF, scale,
                            0, super_rank_proc, &pm_player.player_data.partners[partner_order[i + 1]]);
        super_ranks[i]->tooltip = super_rank_str;

        // ultra tex
        ultra_ranks[i] =
            menu_add_switch(menu, partner_x + 3, partner_y, tex_rank, 0, 0, 0xFFFFFF, tex_rank, 0, 1, 0xFFFFFF, scale,
                            0, ultra_rank_proc, &pm_player.player_data.partners[partner_order[i + 1]]);
        ultra_ranks[i]->tooltip = ultra_rank_str;
    }
    menu_item_add_chain_link(active_item, partners[0], MENU_NAVIGATE_DOWN);
    menu_item_add_chain_link(partners[0], active_item, MENU_NAVIGATE_UP);
    menu_item_create_chain(partners, 8, MENU_NAVIGATE_DOWN, 0, 0);
    menu_item_create_chain(partners, 8, MENU_NAVIGATE_UP, 0, 1);
    menu_item_create_chain(super_ranks, 8, MENU_NAVIGATE_DOWN, 0, 0);
    menu_item_create_chain(super_ranks, 8, MENU_NAVIGATE_UP, 0, 1);
    menu_item_create_chain(ultra_ranks, 8, MENU_NAVIGATE_DOWN, 0, 0);
    menu_item_create_chain(ultra_ranks, 8, MENU_NAVIGATE_UP, 0, 1);
}

static void create_star_spirit_menu(struct menu *menu) {
    menu->selector = menu_add_submenu(menu, 0, 0, NULL, "return");
    menu_add_tooltip(menu, 8, 0, fp.main_menu, 0xC0C0C0);
    const s32 row_width = 4;
    const s32 base_x = 1;
    const s32 base_y = 2;
    const s32 spacing_x = 4;
    const s32 spacing_y = 5;
    const f32 scale = 0.7f;

    struct menu_item *star_spirits[8];
    struct gfx_texture *tex_star_spirits = resource_get(RES_PMICON_STAR_SPIRITS);

    s32 ss_x;
    s32 ss_y;
    for (s32 i = 0; i < 7; i++) {
        ss_x = base_x + (i % row_width) * spacing_x;
        ss_y = base_y + (i / row_width) * spacing_y;
        star_spirits[i] = menu_add_switch(menu, ss_x, ss_y, tex_star_spirits, i, 0, 0xFFFFFF, tex_star_spirits, i, 1,
                                          0xFFFFFF, scale, 0, star_spirit_switch_proc, (void *)i + 1);
    }
    ss_x = base_x + (7 % row_width) * spacing_x;
    ss_y = base_y + (7 / row_width) * spacing_y;
    struct gfx_texture *beam_textures[] = {tex_star_spirits, tex_star_spirits, tex_star_spirits};
    s32 beam_tiles[] = {7, 7, 8};
    s8 beam_palettes[] = {1, 0, 0};
    u32 beam_colors[] = {0xFFFFFF, 0xFFFFFF, 0xFFFFFF};
    menu_add_cycle(menu, ss_x, ss_y, 3, beam_textures, beam_tiles, beam_palettes, beam_colors, scale, 0, beam_rank_proc,
                   NULL);
    menu_item_create_chain(star_spirits, 8, MENU_NAVIGATE_RIGHT, 0, 0);
    menu_item_create_chain(star_spirits, 8, MENU_NAVIGATE_LEFT, 0, 1);
}

struct menu *create_player_menu(void) {
    static struct menu menu;
    static struct menu mario;
    static struct menu partners;
    static struct menu items;
    static struct menu key_items;
    static struct menu stored_items;
    static struct menu badges;
    static struct menu star_spirits;
    static struct menu peach;
    static struct menu merlee;

    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&mario, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&partners, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&items, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&key_items, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&stored_items, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&badges, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&star_spirits, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&peach, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&merlee, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    {
        s32 y = 0;
        menu.selector = menu_add_submenu(&menu, 0, y++, NULL, "return");

        /*build player menu*/
        menu_add_submenu(&menu, 0, y++, &mario, "mario");
        menu_add_submenu(&menu, 0, y++, &partners, "partners");
        menu_add_submenu(&menu, 0, y++, &badges, "badges");
        menu_add_submenu(&menu, 0, y++, &items, "items");
        menu_add_submenu(&menu, 0, y++, &key_items, "key items");
        menu_add_submenu(&menu, 0, y++, &stored_items, "stored items");
        menu_add_submenu(&menu, 0, y++, &star_spirits, "star spirits");
        menu_add_submenu(&menu, 0, y++, &peach, "princess peach");
        menu_add_submenu(&menu, 0, y++, &merlee, "merlee");

        struct gfx_texture **item_textures = get_item_texture_list();
        gfx_add_grayscale_palette(item_textures[ITEM_KOOPA_FORTRESS_KEY], 0);

        create_mario_menu(&mario);
        create_party_menu(&partners);
        create_badges_menu(&badges, item_texture_list);
        create_normal_items_menu(&items, item_texture_list);
        create_key_items_menu(&key_items, item_texture_list);
        create_stored_items_menu(&stored_items, item_texture_list);
        create_item_selection_menu(item_texture_list);
        create_star_spirit_menu(&star_spirits);
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
