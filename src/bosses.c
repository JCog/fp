#include <stdlib.h>
#include "menu.h"
#include "fp.h"

static int byte_mod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    uint8_t *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != *p)
            menu_intinput_set(item, *p);
    }
    else if (reason == MENU_CALLBACK_CHANGED)
        *p = menu_intinput_get(item);
    return 0;
}

static void bowser_hallway_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0x5f;
    fp_set_global_flag(0x1fa, 0); //hallway not defeated
    fp_warp(GROUP_PEACHS_CASTLE, 0x7, 0x0);
}

static void bowser_phase1_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0x5f;
    fp_set_global_flag(0x1fc, 0); //bridge not broken
    fp_set_global_flag(0x1fe, 1); //skip camera zoom in
    fp_warp(GROUP_PEACHS_CASTLE, 0x13, 0x0);
}

static void bowser_phase2_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0x5f;
    fp_set_global_flag(0x1fc, 1); //bridge broken
    fp_warp(GROUP_PEACHS_CASTLE, 0x13, 0x1);
}

static void goomba_king_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0x8f;
    fp_set_global_flag(0x02d, 1); //skip cutscene
    fp_warp(GROUP_GOOMBA_VILLAGE, 0x9, 0x0);
}

static void koopa_bros_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0xb1;
    fp_warp(GROUP_KOOPA_BROS_FORTRESS, 0xa, 0x0);
}

static void tutankoopa_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0xc7;
    fp_warp(GROUP_DRY_DRY_RUINS, 0xe, 0x0);
}

static void tubba_blubba_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0xef;
    fp_warp(GROUP_GUSTY_GULCH, 0x4, 0x0);
}

static void general_guy_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0x03;
    fp_warp(GROUP_SHY_GUYS_TOY_BOX, 0xe, 0x0);
}

static void lava_piranha_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0x22;
    fp_warp(GROUP_VOLCANO, 0xd, 0x1);
}

static void huff_n_puff_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0x37;
    fp_warp(GROUP_FLOWER_FIELDS, 0xf, 0x0);
}

static void crystal_king_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0x55;
    fp_warp(GROUP_CRYSTAL_PALACE, 0x17, 0x0);
}

static void jr_playground_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0x89;
    uint8_t *partner = &pm_player.player_data.current_partner;
    if (*partner == 4 || *partner == 6 || *partner == 8 || *partner == 9) { //flying partners cause a softlock
        *partner = 1; //goombario
    }
    fp_warp(GROUP_GOOMBA_VILLAGE, 0x3, 0x0);
}

static void jr_pleasant_path_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0xb5;
    fp_warp(GROUP_KOOPA_VILLAGE_PLEASANT_PATH, 0x4, 0x1);
}

static void jr_forever_forest_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0xf3;
    fp_set_global_flag(0x39f, 0); //jr not defeated
    fp_warp(GROUP_FOREVER_FOREST, 0x6, 0x3);
}

static void jr_toad_town_port_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0x27;
    fp_set_global_flag(0x4c2, 0); //jr not defeated
    fp_warp(GROUP_TOAD_TOWN, 0x6, 0x1);
}

static void jr_shiver_snowfield_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0x43;
    fp_warp(GROUP_SHIVER_REGION, 0x2, 0x0);
}

static void jr_bowsers_castle_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0x5f;
    fp_set_global_byte(0x12c, 0);
    fp_warp(GROUP_BOWSERS_CASTLE, 0x1c, 0x0);
}

static void goomba_bros_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0x8e;
    fp_warp(GROUP_GOOMBA_VILLAGE, 0x6, 0x0);
}

static void tubbas_heart_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0xe9;
    fp_warp(GROUP_GUSTY_GULCH, 0x8, 0x0);
}

static void lantern_ghost_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0x00;
    fp_warp(GROUP_SHY_GUYS_TOY_BOX, 0xb, 0x0);
}

static void fuzzipede_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0x09;
    fp_warp(GROUP_WHALE, 0x1, 0x0);
}

static void lakilester_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0x32;
    fp_warp(GROUP_FLOWER_FIELDS, 0x8, 0x1);
}

static void monstar_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0x44;
    fp_warp(GROUP_SHIVER_REGION, 0x4, 0x0);
}

static void blooper_proc(struct menu_item *item, void *data) {
    fp_set_global_flag(0x1ab, 0); //blooper not defeated
    fp_set_global_flag(0x1ac, 0); //electro blooper not defeated
    fp_set_global_flag(0x1af, 0); //ch5 pipe switch
    fp_warp(GROUP_SEWERS, 0x7, 0x1);
}

static void electro_blooper_proc(struct menu_item *item, void *data) {
    fp_set_global_flag(0x1ab, 1); //blooper defeated
    fp_set_global_flag(0x1ac, 0); //electro blooper not defeated
    fp_set_global_flag(0x1af, 0); //ch5 pipe switch
    fp_warp(GROUP_SEWERS, 0x7, 0x1);
}

static void super_blooper_proc(struct menu_item *item, void *data) {
    fp_set_global_flag(0x1ab, 1); //blooper defeated
    fp_set_global_flag(0x1ac, 1); //electro blooper defeated
    fp_set_global_flag(0x1af, 0); //ch5 pipe switch
    fp_warp(GROUP_SEWERS, 0x7, 0x1);
}

static void buzzar_proc(struct menu_item *item, void *data) {
    fp_set_global_flag(0x2c4, 0); //buzzar not defeated
    fp_warp(GROUP_MT_RUGGED, 0x4, 0x1);
}

static void anti_guy_proc(struct menu_item *item, void *data) {
    fp_set_global_flag(0x451, 0); //anti guy not defeated
    fp_warp(GROUP_SHY_GUYS_TOY_BOX, 0xc, 0x1);
}

static void kent_c_koopa_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0x29;
    fp_set_global_flag(0x262, 0); //kent not defeated
    fp_warp(GROUP_KOOPA_VILLAGE_PLEASANT_PATH, 0x4, 0x0);
}

static void anti_guys_unit_proc(struct menu_item *item, void *data) {
    STORY_PROGRESS = 0x5f;
    fp_set_global_byte(0x12b, 0);
    fp_warp(GROUP_BOWSERS_CASTLE, 0x1b, 0x0);
}

static void chan_proc(struct menu_item *item, void *data) {
    fp_set_global_byte(0x1C, 0);
    if (pm_status.group_id == 0x1 && pm_status.room_id == 0x1 && !pm_status.is_battle) {
        fp_log("dojo set to chan");
    }
    else {
        fp_warp(GROUP_TOAD_TOWN, 0x1, 0x1);
    }
}

static void lee_proc(struct menu_item *item, void *data) {
    fp_set_global_byte(0x1C, 1);
    if (pm_status.group_id == 0x1 && pm_status.room_id == 0x1 && !pm_status.is_battle) {
        fp_log("dojo set to lee");
    }
    else {
        fp_warp(GROUP_TOAD_TOWN, 0x1, 0x1);
    }
}

static void master1_proc(struct menu_item *item, void *data) {
    fp_set_global_byte(0x1C, 2);
    if (pm_status.group_id == 0x1 && pm_status.room_id == 0x1 && !pm_status.is_battle) {
        fp_log("dojo set to master 1");
    }
    else {
        fp_warp(GROUP_TOAD_TOWN, 0x1, 0x1);
    }
}

static void master2_proc(struct menu_item *item, void *data) {
    fp_set_global_byte(0x1C, 3);
    if (pm_status.group_id == 0x1 && pm_status.room_id == 0x1 && !pm_status.is_battle) {
        fp_log("dojo set to master 2");
    }
    else {
        fp_warp(GROUP_TOAD_TOWN, 0x1, 0x1);
    }
}

static void master3_proc(struct menu_item *item, void *data) {
    fp_set_global_byte(0x1C, 4);
    if (pm_status.group_id == 0x1 && pm_status.room_id == 0x1 && !pm_status.is_battle) {
        fp_log("dojo set to master 3");
    }
    else {
        fp_warp(GROUP_TOAD_TOWN, 0x1, 0x1);
    }
}

static void tab_prev_proc(struct menu_item *item, void *data) {
    menu_tab_previous(data);
}

static void tab_next_proc(struct menu_item *item, void *data) {
    menu_tab_next(data);
}

void create_bosses_menu(struct menu *menu)
{
    int y_main = 0;

    /* initialize menu */
    menu_init(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu->selector = menu_add_submenu(menu, 0, y_main++, NULL, "return");


    int page_count = 6;
    struct menu *pages = malloc(sizeof(*pages) * page_count);
    struct menu_item *tab = menu_add_tab(menu, 0, y_main++, pages, page_count);
    for (int i = 0; i < page_count; ++i) {
        struct menu *page = &pages[i];
        menu_init(page, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    }

    /* bowser */
    int y_tab = 0;
    struct menu *page = &pages[0];
    menu_add_static(page, 0, y_tab++, "bowser", 0xC0C0C0);
    menu_add_button(page, 0, y_tab++, "hallway", bowser_hallway_proc, NULL);
    menu_add_button(page, 0, y_tab++, "final phase 1", bowser_phase1_proc, NULL);
    menu_add_button(page, 0, y_tab++, "final phase 2", bowser_phase2_proc, NULL);
    y_tab++;
    menu_add_static(page, 0, y_tab, "phase 2 hp:", 0xC0C0C0);
    menu_add_intinput(page, 12, y_tab++, 10, 2, byte_mod_proc, &pm_save_data.global_bytes[0x18a]);

    /* chapter bosses */
    y_tab = 0;
    page = &pages[1];
    menu_add_static(page, 0, y_tab++, "chapter bosses", 0xC0C0C0);
    menu_add_button(page, 0, y_tab++, "goomba king", goomba_king_proc, NULL);
    menu_add_button(page, 0, y_tab++, "koopa bros.", koopa_bros_proc, NULL);
    menu_add_button(page, 0, y_tab++, "tutankoopa", tutankoopa_proc, NULL);
    menu_add_button(page, 0, y_tab++, "tubba blubba", tubba_blubba_proc, NULL);
    menu_add_button(page, 0, y_tab++, "general guy", general_guy_proc, NULL);
    menu_add_button(page, 0, y_tab++, "lava piranha", lava_piranha_proc, NULL);
    menu_add_button(page, 0, y_tab++, "huff n. puff", huff_n_puff_proc, NULL);
    menu_add_button(page, 0, y_tab++, "crystal king", crystal_king_proc, NULL);

    /* jr troopa */
    y_tab = 0;
    page = &pages[2];
    menu_add_static(page, 0, y_tab++, "jr. troopa", 0xC0C0C0);
    menu_add_button(page, 0, y_tab++, "playground", jr_playground_proc, NULL);
    menu_add_button(page, 0, y_tab++, "pleasant path", jr_pleasant_path_proc, NULL);
    menu_add_button(page, 0, y_tab++, "forever forest", jr_forever_forest_proc, NULL);
    menu_add_button(page, 0, y_tab++, "toad town port", jr_toad_town_port_proc, NULL);
    menu_add_button(page, 0, y_tab++, "shiver snowfield", jr_shiver_snowfield_proc, NULL);
    menu_add_button(page, 0, y_tab++, "bowser's castle", jr_bowsers_castle_proc, NULL);

    /* minor bosses */
    y_tab = 0;
    page = &pages[3];
    menu_add_static(page, 0, y_tab++, "minor bosses", 0xC0C0C0);
    menu_add_button(page, 0, y_tab++, "goomba bros.", goomba_bros_proc, NULL);
    menu_add_button(page, 0, y_tab++, "tubba's heart", tubbas_heart_proc, NULL);
    menu_add_button(page, 0, y_tab++, "big lantern ghost", lantern_ghost_proc, NULL);
    menu_add_button(page, 0, y_tab++, "fuzzipede", fuzzipede_proc, NULL);
    menu_add_button(page, 0, y_tab++, "lakilester", lakilester_proc, NULL);
    menu_add_button(page, 0, y_tab++, "monstar", monstar_proc, NULL);

    /* optional bosses */
    y_tab = 0;
    page = &pages[4];
    menu_add_static(page, 0, y_tab++, "optional bosses", 0xC0C0C0);
    menu_add_button(page, 0, y_tab++, "blooper", blooper_proc, NULL);
    menu_add_button(page, 0, y_tab++, "electro blooper", electro_blooper_proc, NULL);
    menu_add_button(page, 0, y_tab++, "super blooper", super_blooper_proc, NULL);
    menu_add_button(page, 0, y_tab++, "buzzar", buzzar_proc, NULL);
    menu_add_button(page, 0, y_tab++, "anti guy", anti_guy_proc, NULL);
    menu_add_button(page, 0, y_tab++, "kent c. koopa", kent_c_koopa_proc, NULL);
    menu_add_button(page, 0, y_tab++, "anti guys unit", anti_guys_unit_proc, NULL);

    /* dojo */
    y_tab = 0;
    page = &pages[5];
    menu_add_static(page, 0, y_tab++, "dojo", 0xC0C0C0);
    menu_add_button(page, 0, y_tab++, "chan", chan_proc, NULL);
    menu_add_button(page, 0, y_tab++, "lee", lee_proc, NULL);
    menu_add_button(page, 0, y_tab++, "master 1", master1_proc, NULL);
    menu_add_button(page, 0, y_tab++, "master 2", master2_proc, NULL);
    menu_add_button(page, 0, y_tab++, "master 3", master3_proc, NULL);

    menu_tab_goto(tab, 0);
    menu_add_button(menu, 8, 0, "<", tab_prev_proc, tab);
    menu_add_button(menu, 10, 0, ">", tab_next_proc, tab);
}
