#include <stdlib.h>
#include "menu.h"
#include "fp.h"
#include "tricks.h"
#include "settings.h"

void set_global_flag(int flag_index, _Bool value) {
    int word_index = flag_index / 32;
    int bit = flag_index % 32;
    uint32_t *p = pm_flags.global_flags;
    if (value)
        p[word_index] |= (1 << bit);
    else
        p[word_index] &= ~(1 << bit);
}

void warp(int group, int room, int entrance) {
    pm_status.group_id    = group;
    pm_status.room_id     = room;
    pm_status.entrance_id = entrance;

    pm_unk2.room_change_state = 1;

    uint32_t val = 0x80035DFC;
    pm_warp.room_change_ptr = val;
}

void check_for_hammer() {
    if (pm_player.stats.hammer_upgrade > 2) {
        pm_player.stats.hammer_upgrade = 0;
    }
}

void load_jr_skip() {
    pm_unk3.story_progress = 0x88;
    warp(0, 3, 0);
}

void load_black_toad_skip() {
    check_for_hammer();
    pm_unk3.story_progress = 0x99;
    warp(1, 2, 2);
}

void load_staircase_skip() {
    pm_unk3.story_progress = 0xa8;
    warp(7, 4, 0);
}

void load_pie_jumps() {
    pm_player.stats.current_partner = 2;
    warp(7, 3, 3);
}

void load_log_skip() {
    check_for_hammer();
    pm_unk3.story_progress = 0xae;
    warp(1, 2, 3);
}

void load_trick(int8_t trick) {
    switch (trick) {
        case JR_SKIP:           load_jr_skip();             break;
        case BLACK_TOAD_SKIP:   load_black_toad_skip();     break;

        case STAIRCASE_SKIP:    load_staircase_skip();      break;
        case PIE_JUMPS:         load_pie_jumps();           break;
        case LOG_SKIP:          load_log_skip();            break;
    }
}

//////////////////////////////////////////////////////////////////////////////

static void tab_prev_proc(struct menu_item *item, void *data) {
    menu_tab_previous(data);
}

static void tab_next_proc(struct menu_item *item, void *data) {
    menu_tab_next(data);
}

static void jr_skip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = JR_SKIP;
    load_jr_skip();
}

static void black_toad_skip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = BLACK_TOAD_SKIP;
    load_black_toad_skip();
}

static void staircase_skip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = STAIRCASE_SKIP;
    load_staircase_skip();
}

static void pie_jumps_proc(struct menu_item *item, void *data) {
    fp.saved_trick = PIE_JUMPS;
    load_pie_jumps();
}

static void log_skip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = LOG_SKIP;
    load_log_skip();
}

void create_tricks_menu(struct menu *menu)
{
    int y_main = 0;

    /* initialize menu */
    menu_init(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu->selector = menu_add_submenu(menu, 0, y_main++, NULL, "return");


    int page_count = 9;
    struct menu *pages = malloc(sizeof(*pages) * page_count);
    struct menu_item *tab = menu_add_tab(menu, 0, y_main++, pages, page_count);
//    for (int i = 0; i < page_count; ++i) {
//        struct menu *page = &pages[i];
//        menu_init(page, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
//        y_value = 0;
//        for (int j = 0; j < page_size; ++j) {
//            char buffer[4];
//            int item_index = j + i * page_size;
//            sprintf(buffer, "%02d:", item_index);
//
//            menu_add_static(page, KEY_ITEMS_X_0, y_value, buffer, 0xC0C0C0);
//            menu_add_intinput(page, KEY_ITEMS_X_1, y_value, 16, 3, item_int_proc, &pm_player.key_items[item_index]);
//            menu_add_option(page, KEY_ITEMS_X_2, y_value++, ITEM_LIST, item_option_proc, &pm_player.key_items[item_index]);
//        }
//    }
    for (int i = 0; i < page_count; ++i) {
        struct menu *page = &pages[i];
        menu_init(page, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    }

    /* prologue */
    int y_tab = 0;
    struct menu *page = &pages[0];
    menu_add_static(page, 0, y_tab++, "prologue", 0xC0C0C0);
    menu_add_button(page, 0, y_tab++, "jr skip", jr_skip_proc, NULL);
    menu_add_button(page, 0, y_tab++, "black toad skip", black_toad_skip_proc, NULL);

    /* chapter 1 */
    y_tab = 0;
    page = &pages[1];
    menu_add_static(page, 0, y_tab++, "chapter 1", 0xC0C0C0);
    menu_add_button(page, 0, y_tab++, "staircase skip", staircase_skip_proc, NULL);
    menu_add_button(page, 0, y_tab++, "pie jumps", pie_jumps_proc, NULL);
    menu_add_button(page, 0, y_tab++, "log skip", log_skip_proc, NULL);

    /* chapter 2 */
    y_tab = 0;
    page = &pages[2];
    menu_add_static(page, 0, y_tab++, "chapter 2", 0xC0C0C0);

    /* chapter 3 */
    y_tab = 0;
    page = &pages[3];
    menu_add_static(page, 0, y_tab++, "chapter 3", 0xC0C0C0);

    /* chapter 4 */
    y_tab = 0;
    page = &pages[4];
    menu_add_static(page, 0, y_tab++, "chapter 4", 0xC0C0C0);

    /* chapter 5 */
    y_tab = 0;
    page = &pages[5];
    menu_add_static(page, 0, y_tab++, "chapter 5", 0xC0C0C0);

    /* chapter 6 */
    y_tab = 0;
    page = &pages[6];
    menu_add_static(page, 0, y_tab++, "chapter 6", 0xC0C0C0);

    /* chapter 7 */
    y_tab = 0;
    page = &pages[7];
    menu_add_static(page, 0, y_tab++, "chapter 7", 0xC0C0C0);

    /* chapter 8 */
    y_tab = 0;
    page = &pages[8];
    menu_add_static(page, 0, y_tab++, "chapter 8", 0xC0C0C0);

    menu_tab_goto(tab, 0);
    menu_add_button(menu, 8, 0, "<", tab_prev_proc, tab);
    menu_add_button(menu, 10, 0, ">", tab_next_proc, tab);
}