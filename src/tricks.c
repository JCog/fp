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

void set_partner(int partner) {
    pm_player.stats.current_partner = partner;
}

void set_story_progress(int story_progress) {
    pm_unk3.story_progress = story_progress;
}

void load_jr_skip() {
    set_story_progress(0x88);
    warp(0, 3, 0);
}

void load_black_toad_skip() {
    check_for_hammer();
    set_story_progress(0x99);
    warp(1, 2, 2);
}

void load_retrigger_prologue() {
    check_for_hammer();
    set_story_progress(0x9c);
    set_global_flag(0x01e, 0); //dolly not collected
    for (int i = 0; i < 32; i++) {
        if (pm_player.key_items[i] == 0) {
            pm_player.key_items[i] = 0x1c; //weight
        }
    }
    set_partner(KOOPER);
    warp(0, 2, 0);
}

void load_oot_ace() {
    set_story_progress(0x90);
    pm_player.stats.hammer_upgrade = 0;
    set_partner(GOOMBARIO);
    pm_player.party.goombario.in_party = 1;
    pm_player.party.kooper.in_party = 0;
    pm_player.party.bombette.in_party = 0;
    pm_player.party.parakarry.in_party = 0;
    pm_player.party.bow.in_party = 0;
    pm_player.party.watt.in_party = 0;
    pm_player.party.sushie.in_party = 0;
    pm_player.party.lakilester.in_party = 0;
    int i;
    for (i = 0; i < 9; i++) {
        pm_player.items[i] = 0;
    }
    pm_player.items[9] = 0x93; //whacka's bump
    pm_player.stats.star_points = 50;
    pm_player.stats.hp = pm_player.stats.max_hp;
    pm_player.stats.fp = pm_player.stats.max_fp;
    pm_ace_store.last_timer = 0;
    warp(0, 9, 0);
}

void load_staircase_skip() {
    set_story_progress(0xa8);
    warp(7, 4, 0);
}

void load_pie_jumps() {
    set_partner(KOOPER);
    warp(7, 3, 3);
}

void load_log_skip() {
    check_for_hammer();
    set_story_progress(0xae);
    warp(1, 2, 3);
}

void load_early_seed() {
    check_for_hammer();
    set_partner(KOOPER);
    warp(8, 2, 2);
}

void load_buzzar_skip() {
    pm_player.stats.current_partner = 4;
    warp(8, 4, 0);
}

void load_slow_go_early() {
    set_global_flag(0x37b, 1); //tutankoopa text
    set_global_flag(0x380, 1); //block gone
    set_global_flag(0x384, 1); //chest open
    warp(0xb, 8, 0);
}

void load_ch2_card_lzs() {
    //TODO: make this not crash when loading after failed attempt
    set_story_progress(0xc8);
    set_global_flag(0x37d, 1); //tutankoopa text in previous room
    warp(0xb, 0xe, 0);
}

void load_record_skip() {
    check_for_hammer();
    set_story_progress(0xd5);
    set_partner(BOMBETTE);
    warp(0xd, 6, 0);
}
void load_bow_skip() {
    set_story_progress(0xd9);
    warp(0xc, 0xa, 1);
}

void load_stanley_save() {
    check_for_hammer();
    set_story_progress(0xde);
    set_partner(PARAKARRY);
    warp(0xe, 3, 0);
}

void load_yakkey_trick_shot() {
    set_story_progress(0xe0);
    set_global_flag(0x42f, 0); //yakkey chest
    warp(0xf, 0x12, 0);
}

void load_quick_escape() {
    //TODO: figure out how to get tubba to appear
    set_story_progress(0xe4);
    set_partner(PARAKARRY);
    warp(0xf, 1, 4);
}

void load_ch3_card_lzs() {
    set_story_progress(0xf0);
    warp(0xe, 4, 1);
}

void load_early_train() {
    check_for_hammer();
    set_story_progress(0xb7);
    set_partner(PARAKARRY);
    set_global_flag(0x12f, 0); //snowman doll
    set_global_flag(0x130, 0); //volt shroom
    set_global_flag(0x121, 0); //toy train
    set_global_flag(0x131, 0); //dizzy dial
    warp(1, 5, 0);
}

void load_early_whale_fast_music() {
    check_for_hammer();
    set_story_progress(0x04);
    warp(1, 5, 1);
}

void load_frying_pan_wall_clip() {
    check_for_hammer();
    set_partner(PARAKARRY);
    set_global_flag(0x474, 1); //fuzzy spawn cutscene
    warp(0x10, 6, 0);
}

void load_ch4_card_lzs() {
    set_story_progress(0x04);
    warp(0x10, 0xe, 0);
}

void load_bhs() {
    set_global_flag(0x084, 0); //key collected
    set_global_flag(0x083, 1); //lock opened
    warp(1, 3, 0);
}

void load_early_whale_slow_music() {
    check_for_hammer();
    set_story_progress(0x0d);
    warp(1, 5, 1);
}

void load_raph_skip() {
    set_story_progress(0x0f);
    set_partner(PARAKARRY);
    warp(0x11, 0x14, 0);
}

void load_piranha_first_strike() {
    set_story_progress(0x0f);
    set_partner(BOMBETTE);
    set_global_flag(0x4f8, 0); //yellow yoshi text
    set_global_flag(0x4cf, 0); //yellow yoshi saved
    warp(0x11, 7, 0);
}

void load_lava_platform_cycle() {
    set_story_progress(0x19);
    warp(0x12, 1, 0);
}

void load_lava_puzzle_skip() {
    check_for_hammer();
    set_story_progress(0x1a);
    set_partner(PARAKARRY);
    warp(0x12, 5, 0);
}

void load_flarakarry() {
    set_story_progress(0x1e);
    set_partner(PARAKARRY);
    warp(0x12, 0xb, 0);
}

void load_lava_piranha_skip() {
    check_for_hammer();
    set_story_progress(0x22);
    set_global_flag(0x52f, 1); //talked to kolorado
    warp(0x12, 0xd, 0);
}

void load_ch5_card_lzs() {
    set_story_progress(0x23);
    warp(0x12, 0xd, 1);
}

void load_early_laki() {
    check_for_hammer();
    set_story_progress(0x2b);
    warp(0x13, 0, 1);
}

void load_yellow_berry_skip() {
    check_for_hammer();
    set_story_progress(0x2d);
    set_global_flag(0x554, 0); //yellow flower gate
    warp(0x13, 0, 6);
}

void load_peach_warp() {
    check_for_hammer();
    set_story_progress(0x33);
    set_partner(LAKILESTER);
    warp(0x12, 0xc, 1);
}

void load_ch6_card_lzs() {
    set_story_progress(0x38);
    warp(0x13, 0xf, 0);
}

void load_clippy_boots() {
    set_global_flag(0x18d, 1); //super block broken
    set_global_flag(0x18e, 0); //ultra block unbroken
    set_partner(LAKILESTER);
    pm_player.party.sushie.in_party = 1;
    pm_player.party.lakilester.in_party = 1;
    warp(2, 0x13, 0);
}

void load_murder_solved_early() {
    set_partner(LAKILESTER);
    set_story_progress(0x3e);
    warp(0x14, 0xa, 0);
}

void load_sushie_glitch() {
    set_partner(SUSHIE);
    set_story_progress(0x43);
    warp(0x14, 0xa, 0);
}

void load_ice_staircase_skip() {
    set_partner(LAKILESTER);
    set_story_progress(0x4b);
    warp(0x14, 9, 0);
}

void load_mirror_clip() {
    set_partner(LAKILESTER);
    set_story_progress(0x4e);
    warp(0x15, 1, 1);
}

void load_kooper_puzzle_skip() {
    set_partner(KOOPER);
    set_story_progress(0x51);
    warp(0x15, 0xf, 0);
}

///////////////////////////////////////////////////////////////////////////////

void load_fast_basement() {
    set_story_progress(0x5e);
    set_global_flag(0x60e, 0); //first switch
    set_global_flag(0x60f, 0); //second switch
    set_global_flag(0x615, 0); //basement fight
    set_global_flag(0x614, 0); //hardened lava
    warp(0x16, 7, 0);
}

void load_trick(int8_t trick) {
    switch (trick) {
        case JR_SKIP:                   load_jr_skip();                     break;
        case BLACK_TOAD_SKIP:           load_black_toad_skip();             break;
        case RETRIGGER_PROLOGUE:        load_retrigger_prologue();          break;
        case OOT_ACE:                   load_oot_ace();                     break;

        case STAIRCASE_SKIP:            load_staircase_skip();              break;
        case PIE_JUMPS:                 load_pie_jumps();                   break;
        case LOG_SKIP:                  load_log_skip();                    break;

        case EARLY_SEED:                load_early_seed();                  break;
        case BUZZAR_SKIP:               load_buzzar_skip();                 break;
        case SLOW_GO_EARLY:             load_slow_go_early();               break;
        case CH2_CARD_LZS:              load_ch2_card_lzs();                break;

        case RECORD_SKIP:               load_record_skip();                 break;
        case BOW_SKIP:                  load_bow_skip();                    break;
        case STANLEY_SAVE:              load_stanley_save();                break;
        case YAKKEY_TRICK_SHOT:         load_yakkey_trick_shot();           break;
        case QUICK_ESCAPE:              load_quick_escape();                break;
        case CH3_CARD_LZS:              load_ch3_card_lzs();                break;

        case EARLY_TRAIN:               load_early_train();                 break;
        case EARLY_WHALE_FAST_MUSIC:    load_early_whale_fast_music();      break;
        case FRYING_PAN_WALL_CLIP:      load_frying_pan_wall_clip();        break;
        case CH4_CARD_LZS:              load_ch4_card_lzs();                break;

        case BHS:                       load_bhs();                         break;
        case EARLY_WHALE_SLOW_MUSIC:    load_early_whale_slow_music();      break;
        case RAPH_SKIP:                 load_raph_skip();                   break;
        case PIRANHA_FIRST_STRIKE:      load_piranha_first_strike();        break;
        case LAVA_PLATFORM_CYCLE:       load_lava_platform_cycle();         break;
        case LAVA_PUZZLE_SKIP:          load_lava_puzzle_skip();            break;
        case FLARAKARRY:                load_flarakarry();                  break;
        case LAVA_PIRANHA_SKIP:         load_lava_piranha_skip();           break;
        case CH5_CARD_LZS:              load_ch5_card_lzs();                break;

        case EARLY_LAKI:                load_early_laki();                  break;
        case YELLOW_BERRY_SKIP:         load_yellow_berry_skip();           break;
        case PEACH_WARP:                load_peach_warp();                  break;
        case CH6_CARD_LZS:              load_ch6_card_lzs();                break;

        case CLIPPY_BOOTS:              load_clippy_boots();                break;
        case MURDER_SOLVED_EARLY:       load_murder_solved_early();         break;
        case SUSHIE_GLITCH:             load_sushie_glitch();               break;
        case ICE_STAIRCASE_SKIP:        load_ice_staircase_skip();          break;
        case MIRROR_CLIP:               load_mirror_clip();                 break;
        case KOOPER_PUZZLE_SKIP:        load_kooper_puzzle_skip();          break;

        case FAST_BASEMENT:             load_fast_basement();               break;
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

static void retrigger_prologue_proc(struct menu_item *item, void *data) {
    fp.saved_trick = RETRIGGER_PROLOGUE;
    load_retrigger_prologue();
}

static void oot_ace_proc(struct menu_item *item, void *data) {
    fp.saved_trick = OOT_ACE;
    load_oot_ace();
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

static void early_seed_proc(struct menu_item *item, void *data) {
    fp.saved_trick = EARLY_SEED;
    load_early_seed();
}

static void buzzar_skip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = BUZZAR_SKIP;
    load_buzzar_skip();
}

static void slow_go_early_proc(struct menu_item *item, void *data) {
    fp.saved_trick = SLOW_GO_EARLY;
    load_slow_go_early();
}

static void ch2_card_lzs_proc(struct menu_item *item, void *data) {
    fp.saved_trick = CH2_CARD_LZS;
    load_ch2_card_lzs();
}

static void record_skip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = RECORD_SKIP;
    load_record_skip();
}

static void bow_skip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = BOW_SKIP;
    load_bow_skip();
}

static void stanley_save_proc(struct menu_item *item, void *data) {
    fp.saved_trick = STANLEY_SAVE;
    load_stanley_save();
}

static void yakkey_trick_shot_proc(struct menu_item *item, void *data) {
    fp.saved_trick = YAKKEY_TRICK_SHOT;
    load_yakkey_trick_shot();
}

static void quick_escape_proc(struct menu_item *item, void *data) {
    fp.saved_trick = QUICK_ESCAPE;
    load_quick_escape();
}

static void ch3_card_lzs_proc(struct menu_item *item, void *data) {
    fp.saved_trick = CH3_CARD_LZS;
    load_ch3_card_lzs();
}

static void early_train_proc(struct menu_item *item, void *data) {
    fp.saved_trick = EARLY_TRAIN;
    load_early_train();
}

static void early_whale_fast_music_proc(struct menu_item *item, void *data) {
    fp.saved_trick = EARLY_WHALE_FAST_MUSIC;
    load_early_whale_fast_music();
}

static void frying_pan_wall_clip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = FRYING_PAN_WALL_CLIP;
    load_frying_pan_wall_clip();
}

static void ch4_card_lzs_proc(struct menu_item *item, void *data) {
    fp.saved_trick = CH4_CARD_LZS;
    load_ch4_card_lzs();
}

static void bhs_proc(struct menu_item *item, void *data) {
    fp.saved_trick = BHS;
    load_bhs();
}

static void early_whale_slow_music_proc(struct menu_item *item, void *data) {
    fp.saved_trick = EARLY_WHALE_SLOW_MUSIC;
    load_early_whale_slow_music();
}

static void raph_skip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = RAPH_SKIP;
    load_raph_skip();
}

static void piranha_first_strike_proc(struct menu_item *item, void *data) {
    fp.saved_trick = PIRANHA_FIRST_STRIKE;
    load_piranha_first_strike();
}

static void lava_platform_cycle_proc(struct menu_item *item, void *data) {
    fp.saved_trick = LAVA_PLATFORM_CYCLE;
    load_lava_platform_cycle();
}

static void lava_puzzle_skip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = LAVA_PUZZLE_SKIP;
    load_lava_puzzle_skip();
}

static void flarakarry_proc(struct menu_item *item, void *data) {
    fp.saved_trick = FLARAKARRY;
    load_flarakarry();
}

static void lava_piranha_skip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = LAVA_PIRANHA_SKIP;
    load_lava_piranha_skip();
}

static void ch5_card_lzs_proc(struct menu_item *item, void *data) {
    fp.saved_trick = CH5_CARD_LZS;
    load_ch5_card_lzs();
}

static void early_laki_proc(struct menu_item *item, void *data) {
    fp.saved_trick = EARLY_LAKI;
    load_early_laki();
}

static void yellow_berry_skip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = YELLOW_BERRY_SKIP;
    load_yellow_berry_skip();
}

static void peach_warp_proc(struct menu_item *item, void *data) {
    fp.saved_trick = PEACH_WARP;
    load_peach_warp();
}

static void ch6_card_lzs_proc(struct menu_item *item, void *data) {
    fp.saved_trick = CH6_CARD_LZS;
    load_ch6_card_lzs();
}

static void clippy_boots_proc(struct menu_item *item, void *data) {
    fp.saved_trick = CLIPPY_BOOTS;
    load_clippy_boots();
}

static void murder_solved_early_proc(struct menu_item *item, void *data) {
    fp.saved_trick = MURDER_SOLVED_EARLY;
    load_murder_solved_early();
}

static void sushie_glitch_proc(struct menu_item *item, void *data) {
    fp.saved_trick = SUSHIE_GLITCH;
    load_sushie_glitch();
}

static void ice_staircase_skip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = ICE_STAIRCASE_SKIP;
    load_ice_staircase_skip();
}

static void mirror_clip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = MIRROR_CLIP;
    load_mirror_clip();
}

static void kooper_puzzle_skip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = KOOPER_PUZZLE_SKIP;
    load_kooper_puzzle_skip();
}

///////////////////////////////////////////////////////////////////////////////

static void fast_basement_proc(struct menu_item *item, void *data) {
    fp.saved_trick = FAST_BASEMENT;
    load_fast_basement();
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
    menu_add_button(page, 0, y_tab++, "retrigger prologue", retrigger_prologue_proc, NULL);
    menu_add_button(page, 0, y_tab++, "oot ace", oot_ace_proc, NULL);

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
    menu_add_button(page, 0, y_tab++, "early seed", early_seed_proc, NULL);
    menu_add_button(page, 0, y_tab++, "buzzar skip", buzzar_skip_proc, NULL);
    menu_add_button(page, 0, y_tab++, "slow go early", slow_go_early_proc, NULL);
    menu_add_button(page, 0, y_tab++, "ch2 card lzs", ch2_card_lzs_proc, NULL);

    /* chapter 3 */
    y_tab = 0;
    page = &pages[3];
    menu_add_static(page, 0, y_tab++, "chapter 3", 0xC0C0C0);
    menu_add_button(page, 0, y_tab++, "record skip", record_skip_proc, NULL);
    menu_add_button(page, 0, y_tab++, "bow skip", bow_skip_proc, NULL);
    menu_add_button(page, 0, y_tab++, "stanley save", stanley_save_proc, NULL);
    menu_add_button(page, 0, y_tab++, "yakkey trick shot", yakkey_trick_shot_proc, NULL);
    menu_add_button(page, 0, y_tab++, "quick escape", quick_escape_proc, NULL);
    menu_add_button(page, 0, y_tab++, "ch3 card lzs", ch3_card_lzs_proc, NULL);

    /* chapter 4 */
    y_tab = 0;
    page = &pages[4];
    menu_add_static(page, 0, y_tab++, "chapter 4", 0xC0C0C0);
    menu_add_button(page, 0, y_tab++, "early train", early_train_proc, NULL);
    menu_add_button(page, 0, y_tab++, "early whale (fast music)", early_whale_fast_music_proc, NULL);
    menu_add_button(page, 0, y_tab++, "frying pan wall clip", frying_pan_wall_clip_proc, NULL);
    menu_add_button(page, 0, y_tab++, "ch4 card lzs", ch4_card_lzs_proc, NULL);

    /* chapter 5 */
    y_tab = 0;
    page = &pages[5];
    menu_add_static(page, 0, y_tab++, "chapter 5", 0xC0C0C0);
    menu_add_button(page, 0, y_tab++, "blue house skip", bhs_proc, NULL);
    menu_add_button(page, 0, y_tab++, "early whale (slow music)", early_whale_slow_music_proc, NULL);
    menu_add_button(page, 0, y_tab++, "raph/yoshi skip", raph_skip_proc, NULL);
    menu_add_button(page, 0, y_tab++, "putrid piranha first strike", piranha_first_strike_proc, NULL);
    menu_add_button(page, 0, y_tab++, "lava platform cycle", lava_platform_cycle_proc, NULL);
    menu_add_button(page, 0, y_tab++, "lava puzzle skip", lava_puzzle_skip_proc, NULL);
    menu_add_button(page, 0, y_tab++, "flarakarry", flarakarry_proc, NULL);
    menu_add_button(page, 0, y_tab++, "lava piranha skip/oob", lava_piranha_skip_proc, NULL);
    menu_add_button(page, 0, y_tab++, "ch5 card lzs", ch5_card_lzs_proc, NULL);

    /* chapter 6 */
    y_tab = 0;
    page = &pages[6];
    menu_add_static(page, 0, y_tab++, "chapter 6", 0xC0C0C0);
    menu_add_button(page, 0, y_tab++, "early laki", early_laki_proc, NULL);
    menu_add_button(page, 0, y_tab++, "yellow berry skip", yellow_berry_skip_proc, NULL);
    menu_add_button(page, 0, y_tab++, "peach warp", peach_warp_proc, NULL);
    menu_add_button(page, 0, y_tab++, "ch6 card lzs", ch6_card_lzs_proc, NULL);

    /* chapter 7 */
    y_tab = 0;
    page = &pages[7];
    menu_add_static(page, 0, y_tab++, "chapter 7", 0xC0C0C0);
    menu_add_button(page, 0, y_tab++, "clippy boots", clippy_boots_proc, NULL);
    menu_add_button(page, 0, y_tab++, "murder solved early", murder_solved_early_proc, NULL);
    menu_add_button(page, 0, y_tab++, "sushie glitch", sushie_glitch_proc, NULL);
    menu_add_button(page, 0, y_tab++, "ice staircase skip", ice_staircase_skip_proc, NULL);
    menu_add_button(page, 0, y_tab++, "mirror clip", mirror_clip_proc, NULL);
    menu_add_button(page, 0, y_tab++, "kooper puzzle skip", kooper_puzzle_skip_proc, NULL);

    /* chapter 8 */
    y_tab = 0;
    page = &pages[8];
    menu_add_static(page, 0, y_tab++, "chapter 8", 0xC0C0C0);
    menu_add_button(page, 0, y_tab++, "fast basement", fast_basement_proc, NULL);

    menu_tab_goto(tab, 0);
    menu_add_button(menu, 8, 0, "<", tab_prev_proc, tab);
    menu_add_button(menu, 10, 0, ">", tab_next_proc, tab);
}