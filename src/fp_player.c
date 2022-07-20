#include "common.h"
#include "fp.h"
#include "gfx.h"
#include "items.h"
#include "menu.h"
#include "resource.h"
#include <stdlib.h>

static const char *str_hp = "HP";
static const char *str_max_hp = "Max HP";
static const char *str_fp = "FP";
static const char *str_max_fp = "Max FP";
static const char *str_bp = "BP";
static const char *str_coins = "Coins";
static const char *str_star_pieces = "Star Pieces";
static const char *str_level = "Level";
static const char *str_star_points = "Star Points";
static const char *str_action_commands = "Action Commands";
static const char *str_boots_normal = "Normal Boots";
static const char *str_boots_super = "Super Boots";
static const char *str_boots_ultra = "Ultra Boots";
static const char *str_hammer_normal = "Normal Hammer";
static const char *str_hammer_super = "Super Hammer";
static const char *str_hammer_ultra = "Ultra Hammer";
static const char *str_partner_names[] = {
    "Goombario", "Kooper",     "Bombette", "Parakarry", "Bow",   "Watt",
    "Sushie",    "Lakilester", "Goompa",   "Goombaria", "Twink",
};
static const char *str_star_spirit_names[] = {
    "Eldstar", "Mamar", "Skolar", "Muskular", "Misstar", "Klevar", "Kalmar",
};
static const char *str_star_peach_beam = "Star/Peach Beam";
static const char *str_super_rank = "Super Rank";
static const char *str_ultra_rank = "Ultra Rank";
static const u32 partner_order[] = {
    0, 1, 2, 3, 4, 9, 6, 7, 8, 5, 10, 11,
};

static struct gfx_texture *item_texture_list[0x16D];

static struct gfx_texture **get_item_texture_list(void) {
    static bool ready = FALSE;
    if (!ready) {
        ready = TRUE;
        for (u16 i = 0; i < 0x16D; i++) {
            item_texture_list[i] = resource_load_pmicon_item(i, FALSE);
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
        if (menu_intinput_get(item) != pm_player.playerData.maxHP) {
            menu_intinput_set(item, pm_player.playerData.maxHP);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        pm_player.playerData.maxHP = menu_intinput_get(item);
        pm_player.playerData.hardMaxHP = menu_intinput_get(item);
    }
    return 0;
}

static s32 max_fp_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != pm_player.playerData.curMaxFP) {
            menu_intinput_set(item, pm_player.playerData.curMaxFP);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        pm_player.playerData.curMaxFP = menu_intinput_get(item);
        pm_player.playerData.hardMaxFP = menu_intinput_get(item);
    }
    return 0;
}

static s32 current_partner_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        menu_option_set(item, partner_order[pm_player.playerData.currentPartner]);
    } else if (reason == MENU_CALLBACK_DEACTIVATE) {
        pm_player.playerData.currentPartner = partner_order[menu_option_get(item)];
    }
    return 0;
}

static s32 boots_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    u32 tracked_level = (u32)data;
    u8 *boots_upgrade = &pm_player.playerData.bootsLevel;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        *boots_upgrade = tracked_level;
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_switch_set(item, *boots_upgrade == tracked_level);
    }
    return 0;
}

static s32 hammer_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    u32 tracked_level = (u32)data;
    u8 *hammer_upgrade = &pm_player.playerData.hammerLevel;
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
    u8 *has_action_commands = &pm_player.playerData.hasActionCommands;
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
    pm_PartnerData_t *partner = (pm_PartnerData_t *)data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        partner->enabled = 1;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        partner->enabled = 0;
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_switch_set(item, partner->enabled);
    }
    return 0;
}

static s32 super_rank_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    pm_PartnerData_t *partner = (pm_PartnerData_t *)data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        partner->level = 1;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        if (partner->level == 2) {
            partner->level = 1;
        } else {
            partner->level = 0;
        }
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_switch_set(item, partner->level > 0);
    }
    return 0;
}

static s32 ultra_rank_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    pm_PartnerData_t *partner = (pm_PartnerData_t *)data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        partner->level = 2;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        partner->level = 0;
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_switch_set(item, partner->level == 2);
    }
    return 0;
}

static s32 star_spirit_switch_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    u8 *ss_saved = &pm_player.playerData.starSpiritsSaved;
    u16 *star_power = &pm_player.playerData.totalStarPower;
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
    u8 *beam_rank = &pm_player.playerData.starBeamLevel;
    if (reason == MENU_CALLBACK_CHANGED) {
        *beam_rank = menu_cycle_get(item);
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_cycle_set(item, *beam_rank);
    }
    return 0;
}

static s32 peach_or_mario_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_gGameStatus.peachFlags |= (1 << 0);
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_gGameStatus.peachFlags &= ~(1 << 0);
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_gGameStatus.peachFlags & (1 << 0));
    }
    return 0;
}

static s32 peach_transformed_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_gGameStatus.peachFlags |= (1 << 1);
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_gGameStatus.peachFlags &= ~(1 << 1);
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_gGameStatus.peachFlags & (1 << 1));
    }
    return 0;
}

static s32 peach_parasol_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        pm_gGameStatus.peachFlags |= (1 << 2);
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        pm_gGameStatus.peachFlags &= ~(1 << 2);
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, pm_gGameStatus.peachFlags & (1 << 2));
    }
    return 0;
}

static void create_boots_and_hammer_menu(struct menu *menu) {
    menu->selector = menu_add_submenu(menu, 0, 0, NULL, "return");
    menu_add_tooltip(menu, 8, 0, fp.main_menu, 0xC0C0C0);

    struct gfx_texture **item_textures = get_item_texture_list();

    struct gfx_texture *tex_boots_normal = item_textures[ITEM_JUMP];
    struct gfx_texture *tex_boots_super = item_textures[ITEM_SPIN_JUMP];
    struct gfx_texture *tex_boots_ultra = item_textures[ITEM_TORNADO_JUMP];
    struct gfx_texture *tex_hammer_normal = item_textures[ITEM_HAMMER];
    struct gfx_texture *tex_hammer_super = item_textures[ITEM_SUPER_HAMMER];
    struct gfx_texture *tex_hammer_ultra = item_textures[ITEM_ULTRA_HAMMER];
    gfx_add_grayscale_palette(tex_boots_normal, 0);
    gfx_add_grayscale_palette(tex_boots_super, 0);
    gfx_add_grayscale_palette(tex_boots_ultra, 0);
    gfx_add_grayscale_palette(tex_hammer_normal, 0);
    gfx_add_grayscale_palette(tex_hammer_super, 0);
    gfx_add_grayscale_palette(tex_hammer_ultra, 0);

    struct menu_item *item;
    s32 boots_x = 1;
    s32 boots_y = 2;
    item = menu_add_switch(menu, boots_x, boots_y, tex_boots_normal, 0, 0, 0xFFFFFF, tex_boots_normal, 0, 1, 0xFFFFFF,
                           0.7f, FALSE, boots_proc, (void *)0);
    item->tooltip = str_boots_normal;
    item = menu_add_switch(menu, boots_x + 3, boots_y, tex_boots_super, 0, 0, 0xFFFFFF, tex_boots_super, 0, 1, 0xFFFFFF,
                           0.7f, FALSE, boots_proc, (void *)1);
    item->tooltip = str_boots_super;
    item = menu_add_switch(menu, boots_x + 6, boots_y, tex_boots_ultra, 0, 0, 0xFFFFFF, tex_boots_ultra, 0, 1, 0xFFFFFF,
                           0.7f, FALSE, boots_proc, (void *)2);
    item->tooltip = str_boots_ultra;

    s32 hammer_x = 1;
    s32 hammer_y = 5;
    item = menu_add_switch(menu, hammer_x, hammer_y, tex_hammer_normal, 0, 0, 0xFFFFFF, tex_hammer_normal, 0, 1,
                           0xFFFFFF, 0.7f, FALSE, hammer_proc, (void *)0);
    item->tooltip = str_hammer_normal;
    item = menu_add_switch(menu, hammer_x + 3, hammer_y, tex_hammer_super, 0, 0, 0xFFFFFF, tex_hammer_super, 0, 1,
                           0xFFFFFF, 0.7f, FALSE, hammer_proc, (void *)1);
    item->tooltip = str_hammer_super;
    item = menu_add_switch(menu, hammer_x + 6, hammer_y, tex_hammer_ultra, 0, 0, 0xFFFFFF, tex_hammer_ultra, 0, 1,
                           0xFFFFFF, 0.7f, FALSE, hammer_proc, (void *)2);
    item->tooltip = str_hammer_ultra;
}

static void create_stats_menu(struct menu *menu) {
    menu->selector = menu_add_submenu(menu, 0, 0, NULL, "return");
    menu_add_tooltip(menu, 8, 0, fp.main_menu, 0xC0C0C0);

    struct gfx_texture **item_textures = get_item_texture_list();

    struct gfx_texture *tex_lucky_star = item_textures[ITEM_LUCKY_STAR];
    struct gfx_texture *tex_heart = resource_load_pmicon_global(ICON_STATUS_HEART, 1);
    struct gfx_texture *tex_flower = resource_load_pmicon_global(ICON_STATUS_FLOWER, 1);
    struct gfx_texture *tex_bp_icon = resource_get(RES_PMICON_BP);
    struct gfx_texture *tex_mario_head = resource_load_pmicon_global(ICON_MARIO_HEAD, 1);
    struct gfx_texture *tex_star_point = resource_load_pmicon_global(ICON_STATUS_STAR_POINT, 1);
    struct gfx_texture *tex_star_piece = resource_load_pmicon_global(ICON_STATUS_STAR_PIECE, 1);
    struct gfx_texture *tex_coin = resource_load_pmicon_global(ICON_STATUS_COIN, 1);
    gfx_add_grayscale_palette(tex_lucky_star, 0);

    struct menu_item *item;

    s32 hp_x = 1;
    s32 hp_y = 2;
    menu_add_static_icon(menu, hp_x, hp_y, tex_heart, 0, 0xFFFFFF, 1.0f);
    item = menu_add_intinput(menu, hp_x + 2, hp_y, 10, 2, byte_mod_proc, &pm_player.playerData.curHP);
    item->tooltip = str_hp;
    menu_add_static(menu, hp_x + 4, hp_y, "/", 0xC0C0C0);
    item = menu_add_intinput(menu, hp_x + 5, hp_y, 10, 2, max_hp_proc, NULL);
    item->tooltip = str_max_hp;

    s32 fp_x = 1;
    s32 fp_y = 4;
    menu_add_static_icon(menu, fp_x, fp_y, tex_flower, 0, 0xFFFFFF, 1.0f);
    item = menu_add_intinput(menu, fp_x + 2, fp_y, 10, 2, byte_mod_proc, &pm_player.playerData.curFP);
    item->tooltip = str_fp;
    menu_add_static(menu, fp_x + 4, fp_y, "/", 0xC0C0C0);
    item = menu_add_intinput(menu, fp_x + 5, fp_y, 10, 2, max_fp_proc, NULL);
    item->tooltip = str_max_fp;

    s32 bp_x = 1;
    s32 bp_y = 6;
    menu_add_static_icon(menu, bp_x, bp_y, tex_bp_icon, 0, 0xFFFFFF, 1.0f);
    item = menu_add_intinput(menu, bp_x + 2, bp_y, 10, 2, byte_mod_proc, &pm_player.playerData.maxBP);
    item->tooltip = str_bp;

    s32 coin_x = 10;
    s32 coin_y = 2;
    menu_add_static_icon(menu, coin_x, coin_y, tex_coin, 0, 0xFFFFFF, 1.0f);
    item = menu_add_intinput(menu, coin_x + 2, coin_y, 10, 3, halfword_mod_proc, &pm_player.playerData.coins);
    item->tooltip = str_coins;

    s32 star_piece_x = 10;
    s32 star_piece_y = 4;
    menu_add_static_icon(menu, star_piece_x, star_piece_y, tex_star_piece, 0, 0xFFFFFF, 1.0f);
    item =
        menu_add_intinput(menu, star_piece_x + 2, star_piece_y, 10, 3, byte_mod_proc, &pm_player.playerData.starPieces);
    item->tooltip = str_star_pieces;

    s32 level_x = 17;
    s32 level_y = 2;
    menu_add_static_icon(menu, level_x, level_y, tex_mario_head, 0, 0xFFFFFF, 1.0f);
    item = menu_add_intinput(menu, level_x + 2, level_y, 10, 2, byte_mod_proc, &pm_player.playerData.level);
    item->tooltip = str_level;

    s32 star_point_x = 17;
    s32 star_point_y = 4;
    menu_add_static_icon(menu, star_point_x, star_point_y, tex_star_point, 0, 0xFFFFFF, 1.0f);
    item =
        menu_add_intinput(menu, star_point_x + 2, star_point_y, 10, 2, byte_mod_proc, &pm_player.playerData.starPoints);
    item->tooltip = str_star_points;

    s32 action_command_x = 23;
    s32 action_command_y = 2;
    item = menu_add_switch(menu, action_command_x, action_command_y, tex_lucky_star, 0, 0, 0xFFFFFF, tex_lucky_star, 0,
                           1, 0xFFFFFF, 0.7f, FALSE, action_commands_proc, NULL);
    item->tooltip = str_action_commands;
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

        partners[i] = menu_add_switch(menu, partner_x, partner_y, tex_partner, i + 1, 0, 0xFFFFFF, tex_partner, i + 1,
                                      1, 0xFFFFFF, scale, FALSE, in_party_proc,
                                      &pm_player.playerData.partners[partner_order[i + 1]]);
        partners[i]->tooltip = str_partner_names[i];

        // super tex
        super_ranks[i] =
            menu_add_switch(menu, partner_x + 2, partner_y, tex_rank, 0, 0, 0xFFFFFF, tex_rank, 0, 1, 0xFFFFFF, scale,
                            FALSE, super_rank_proc, &pm_player.playerData.partners[partner_order[i + 1]]);
        super_ranks[i]->tooltip = str_super_rank;

        // ultra tex
        ultra_ranks[i] =
            menu_add_switch(menu, partner_x + 3, partner_y, tex_rank, 0, 0, 0xFFFFFF, tex_rank, 0, 1, 0xFFFFFF, scale,
                            FALSE, ultra_rank_proc, &pm_player.playerData.partners[partner_order[i + 1]]);
        ultra_ranks[i]->tooltip = str_ultra_rank;
    }
    menu_item_add_chain_link(active_item, partners[0], MENU_NAVIGATE_DOWN);
    menu_item_add_chain_link(partners[0], active_item, MENU_NAVIGATE_UP);
    menu_item_create_chain(partners, 8, MENU_NAVIGATE_DOWN, FALSE, FALSE);
    menu_item_create_chain(partners, 8, MENU_NAVIGATE_UP, FALSE, TRUE);
    menu_item_create_chain(super_ranks, 8, MENU_NAVIGATE_DOWN, FALSE, FALSE);
    menu_item_create_chain(super_ranks, 8, MENU_NAVIGATE_UP, FALSE, TRUE);
    menu_item_create_chain(ultra_ranks, 8, MENU_NAVIGATE_DOWN, FALSE, FALSE);
    menu_item_create_chain(ultra_ranks, 8, MENU_NAVIGATE_UP, FALSE, TRUE);
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
                                          0xFFFFFF, scale, FALSE, star_spirit_switch_proc, (void *)i + 1);
        star_spirits[i]->tooltip = str_star_spirit_names[i];
    }
    ss_x = base_x + (7 % row_width) * spacing_x;
    ss_y = base_y + (7 / row_width) * spacing_y;
    struct gfx_texture *beam_textures[] = {tex_star_spirits, tex_star_spirits, tex_star_spirits};
    s32 beam_tiles[] = {7, 7, 8};
    s8 beam_palettes[] = {1, 0, 0};
    u32 beam_colors[] = {0xFFFFFF, 0xFFFFFF, 0xFFFFFF};
    struct menu_item *item = menu_add_cycle(menu, ss_x, ss_y, 3, beam_textures, beam_tiles, beam_palettes, beam_colors,
                                            scale, FALSE, beam_rank_proc, NULL);
    item->tooltip = str_star_peach_beam;
    menu_item_create_chain(star_spirits, 8, MENU_NAVIGATE_RIGHT, FALSE, FALSE);
    menu_item_create_chain(star_spirits, 8, MENU_NAVIGATE_LEFT, FALSE, TRUE);
}

static void create_peach_menu(struct menu *menu) {
    s32 y = 0;
    menu->selector = menu_add_submenu(menu, 0, y++, NULL, "return");

    menu_add_static(menu, 0, y, "peach", 0xC0C0C0);
    menu_add_checkbox(menu, 12, y++, peach_or_mario_proc, NULL);

    menu_add_static(menu, 0, y, "transformed", 0xC0C0C0);
    menu_add_checkbox(menu, 12, y++, peach_transformed_proc, NULL);

    menu_add_static(menu, 0, y, "parasol", 0xC0C0C0);
    menu_add_checkbox(menu, 12, y++, peach_parasol_proc, NULL);

    menu_add_static(menu, 0, y, "disguise", 0xC0C0C0);
    menu_add_option(menu, 12, y++,
                    "none\0"
                    "koopatrol\0"
                    "hammer bro\0"
                    "clubba\0",
                    byte_optionmod_proc, &pm_gGameStatus.peachDisguise);
}

static void create_merlee_menu(struct menu *menu) {
    s32 y_value = 0;
    menu->selector = menu_add_submenu(menu, 0, y_value++, NULL, "return");

    menu_add_static(menu, 0, y_value, "spell type", 0xC0C0C0);
    menu_add_option(menu, 16, y_value++,
                    "none\0"
                    "+3 ATK\0"
                    "+3 DEF\0"
                    "EXP x2\0"
                    "Coins x2\0",
                    byte_optionmod_proc, &pm_player.playerData.merleeSpellType);

    menu_add_static(menu, 0, y_value, "casts remaining", 0xC0C0C0);
    menu_add_intinput(menu, 16, y_value++, 10, 2, byte_mod_proc, &pm_player.playerData.merleeCastsRemaining);

    menu_add_static(menu, 0, y_value, "turns remaining", 0xC0C0C0);
    menu_add_intinput(menu, 16, y_value++, 10, 3, halfword_mod_proc, &pm_player.playerData.merleeTurnCount);
}

struct menu *create_player_menu(void) {
    static struct menu menu;

    static struct menu stats;

    static struct menu equipment;
    static struct menu boots_and_hammer;
    static struct menu badges;

    static struct menu party;
    static struct menu partners;
    static struct menu star_spirits;

    static struct menu items;
    static struct menu regular_items;
    static struct menu key_items;
    static struct menu stored_items;

    static struct menu misc;
    static struct menu peach;
    static struct menu merlee;

    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    menu_init(&stats, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    menu_init(&equipment, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&boots_and_hammer, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&badges, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    menu_init(&party, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&partners, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&star_spirits, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    menu_init(&items, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&regular_items, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&key_items, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&stored_items, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    menu_init(&misc, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&peach, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&merlee, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    /*build player menu*/
    struct gfx_texture **item_textures = get_item_texture_list();
    gfx_add_grayscale_palette(item_textures[ITEM_KOOPA_FORTRESS_KEY], 0);
    create_item_selection_menu(item_texture_list);

    s32 y = 0;
    menu.selector = menu_add_submenu(&menu, 0, y++, NULL, "return");
    menu_add_submenu(&menu, 0, y++, &stats, "stats");
    menu_add_submenu(&menu, 0, y++, &equipment, "equipment");
    menu_add_submenu(&menu, 0, y++, &party, "party");
    menu_add_submenu(&menu, 0, y++, &items, "items");
    menu_add_submenu(&menu, 0, y++, &misc, "misc");
    create_stats_menu(&stats);

    y = 0;
    equipment.selector = menu_add_submenu(&equipment, 0, y++, NULL, "return");
    menu_add_submenu(&equipment, 0, y++, &boots_and_hammer, "boots and hammer");
    menu_add_submenu(&equipment, 0, y++, &badges, "badges");
    create_boots_and_hammer_menu(&boots_and_hammer);
    create_badges_menu(&badges, item_texture_list);

    y = 0;
    party.selector = menu_add_submenu(&party, 0, y++, NULL, "return");
    menu_add_submenu(&party, 0, y++, &partners, "partners");
    menu_add_submenu(&party, 0, y++, &star_spirits, "star spirits");
    create_party_menu(&partners);
    create_star_spirit_menu(&star_spirits);

    y = 0;
    items.selector = menu_add_submenu(&items, 0, y++, NULL, "return");
    menu_add_submenu(&items, 0, y++, &regular_items, "regular items");
    menu_add_submenu(&items, 0, y++, &key_items, "key items");
    menu_add_submenu(&items, 0, y++, &stored_items, "stored items");
    create_normal_items_menu(&regular_items, item_texture_list);
    create_key_items_menu(&key_items, item_texture_list);
    create_stored_items_menu(&stored_items, item_texture_list);

    y = 0;
    misc.selector = menu_add_submenu(&misc, 0, y++, NULL, "return");
    menu_add_submenu(&misc, 0, y++, &peach, "princess peach");
    menu_add_submenu(&misc, 0, y++, &merlee, "merlee");
    create_peach_menu(&peach);
    create_merlee_menu(&merlee);

    return &menu;
}
