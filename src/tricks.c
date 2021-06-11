#include <stdlib.h>
#include "menu.h"
#include "fp.h"
#include "tricks.h"
#include "settings.h"

void check_for_hammer() {
    if (pm_player.stats.hammer_upgrade > 2) {
        pm_player.stats.hammer_upgrade = 0;
    }
}

void set_partner(int partner) {
    pm_player.stats.current_partner = partner;
}

void remove_key_item(int item_id) {
    for (int i = 0; i < 32; i++) {
        if (pm_player.key_items[i] == item_id) {
            pm_player.key_items[i] = 0;
        }
    }
}

void load_jr_skip() {
    fp_set_story_progress(0x88);
    fp_warp(0, 3, 0);
}

void load_black_toad_skip() {
    check_for_hammer();
    fp_set_story_progress(0x99);
    fp_warp(1, 2, 2);
}

void load_retrigger_prologue() {
    check_for_hammer();
    fp_set_story_progress(0x9c);
    fp_set_global_flag(0x01e, 0); //dolly not collected
    for (int i = 0; i < 32; i++) {
        if (pm_player.key_items[i] == 0) {
            pm_player.key_items[i] = 0x1c; //weight
        }
    }
    set_partner(KOOPER);
    fp_warp(0, 2, 0);
}

void load_oot_ace() {
    fp_set_story_progress(0x90);
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
    fp_warp(0, 9, 0);
}

void load_staircase_skip() {
    fp_set_story_progress(0xa8);
    fp_warp(7, 4, 0);
}

void load_pit() {
    //TODO: figure out how defeated enemy flags work properly
    uint8_t *enemies_defeated = (uint8_t*)0x800B10F3; //JP address for koopas in pit
    *enemies_defeated = 0;
    fp_set_global_flag(0x29a, 0); //pit not cleared
    fp_warp(7,7,0);
}

void load_pie_jumps() {
    set_partner(KOOPER);
    fp_warp(7, 3, 3);
}

void load_log_skip() {
    check_for_hammer();
    fp_set_story_progress(0xae);
    fp_warp(1, 2, 1);
}

void load_rock_climb() {
    fp_warp(8, 1, 0);
}

void load_early_seed() {
    check_for_hammer();
    set_partner(KOOPER);
    remove_key_item(0x2c); //magical seed 2
    fp_warp(8, 2, 2);
}

void load_buzzar_skip() {
    pm_player.stats.current_partner = 4;
    fp_warp(8, 4, 0);
}

void load_outpost_jump() {
    fp_set_story_progress(0xc0);
    fp_warp(9, 1, 0);
}

void load_spike_shield_early() {
    fp_set_global_flag(0x694, 0); //spike shield
    fp_set_area_flag(0x4, 0); //coffin 1
    fp_set_area_flag(0x5, 0); //coffin 2
    fp_set_area_flag(0x6, 0); //coffin 3
    for (int i = 0; i < 128; i++) {
        if (pm_player.badges[i] == 0xe5) { //spike shield
            pm_player.badges[i] = 0;
        }
    }
    fp_warp(0xb, 0x1, 0x0);
}

void load_slow_go_early() {
    fp_set_global_flag(0x37b, 1); //tutankoopa text
    fp_set_global_flag(0x380, 1); //block gone
    fp_set_global_flag(0x384, 1); //chest open
    pm_player.stats.boots_upgrade = 1;
    fp_warp(0xb, 8, 0);
}

void load_ch2_card_lzs() {
    //TODO: make this not crash when loading after failed attempt
    fp_set_story_progress(0xc8);
    fp_set_global_flag(0x37d, 1); //tutankoopa text in previous room
    fp_warp(0xb, 0xe, 0);
}

void load_record_skip() {
    check_for_hammer();
    fp_set_story_progress(0xd5);
    set_partner(BOMBETTE);
    remove_key_item(0x1c); //weight
    fp_warp(0xd, 6, 0);
}
void load_bow_skip() {
    fp_set_story_progress(0xd9);
    fp_warp(0xc, 0xa, 1);
}

void load_stanley_save() {
    check_for_hammer();
    fp_set_story_progress(0xde);
    set_partner(PARAKARRY);
    fp_warp(0xe, 3, 0);
}

void load_yakkey_trick_shot() {
    fp_set_story_progress(0xe0);
    fp_set_global_flag(0x42f, 0); //yakkey chest
    remove_key_item(0x1f); //mystical key
    fp_warp(0xf, 0x12, 0);
}

void load_quick_escape() {
    //TODO: figure out how to get tubba to appear
    fp_set_story_progress(0xe4);
    set_partner(PARAKARRY);
    fp_warp(0xf, 1, 4);
}

void load_ch3_card_lzs() {
    fp_set_story_progress(0xf0);
    fp_warp(0xe, 4, 1);
}

void load_early_train() {
    check_for_hammer();
    fp_set_story_progress(0xb7);
    set_partner(PARAKARRY);
    fp_set_global_flag(0x12f, 0); //snowman doll
    fp_set_global_flag(0x130, 0); //volt shroom
    fp_set_global_flag(0x121, 0); //toy train
    fp_set_global_flag(0x131, 0); //dizzy dial
    remove_key_item(0x21); //toy train
    fp_warp(1, 5, 0);
}

void load_early_whale_fast_music() {
    check_for_hammer();
    fp_set_story_progress(0x04);
    fp_warp(1, 5, 1);
}

void load_frying_pan_wall_clip() {
    check_for_hammer();
    set_partner(PARAKARRY);
    fp_set_global_flag(0x474, 1); //fuzzy spawn cutscene
    fp_warp(0x10, 6, 0);
}

void load_gourmet_guy_skip() {
    check_for_hammer();
    fp_set_story_progress(0xfb);
    fp_warp(0x10, 5, 0);
}

void load_ch4_card_lzs() {
    fp_set_story_progress(0x04);
    fp_warp(0x10, 0xe, 0);
}

void load_bhs_top() {
    if (pm_unk3.story_progress < 0x6 || pm_unk3.story_progress > 0xf2) {
        fp_set_story_progress(0xf1);
    }
    fp_set_global_flag(0x084, 0); //key collected
    fp_set_global_flag(0x083, 1); //lock opened
    remove_key_item(0x6b); //odd key
    fp_warp(1, 3, 2);
}

void load_bhs_bottom() {
    if (pm_unk3.story_progress < 0x6 || pm_unk3.story_progress > 0xf2) {
        fp_set_story_progress(0xf1);
    }
    fp_set_global_flag(0x084, 0); //key collected
    fp_set_global_flag(0x083, 1); //lock opened
    remove_key_item(0x6b); //odd key
    fp_warp(1, 3, 3);
}

void load_early_whale_slow_music() {
    check_for_hammer();
    fp_set_story_progress(0x0d);
    fp_warp(1, 5, 1);
}

void load_early_fuzzipede() {
    fp_set_story_progress(0x09);
    set_partner(WATT);
    fp_warp(0x3, 0x1, 0x0);
}

void load_raph_skip() {
    fp_set_story_progress(0x0f);
    set_partner(PARAKARRY);
    fp_warp(0x11, 0x14, 0);
}

void load_piranha_first_strike() {
    fp_set_story_progress(0x0f);
    set_partner(BOMBETTE);
    fp_set_global_flag(0x4f8, 0); //yellow yoshi text
    fp_set_global_flag(0x4cf, 0); //yellow yoshi saved
    fp_warp(0x11, 7, 0);
}

void load_lava_platform_cycle() {
    fp_set_story_progress(0x19);
    fp_warp(0x12, 1, 0);
}

void load_ultra_hammer_skip() {
    check_for_hammer();
    fp_set_story_progress(0x1a);
    fp_warp(0x12, 0x2, 0);
}

void load_lava_puzzle_skip() {
    check_for_hammer();
    fp_set_story_progress(0x1a);
    set_partner(PARAKARRY);
    fp_warp(0x12, 5, 0);
}

void load_ultra_hammer_early() {
    fp_set_global_flag(0x522, 0); //block not destroyed
    fp_set_global_flag(0x523, 0); //chest closed
    fp_warp(0x12, 6, 0);
}

void load_flarakarry() {
    fp_set_story_progress(0x1e);
    set_partner(PARAKARRY);
    fp_warp(0x12, 0xb, 0);
}

void load_lava_piranha_skip() {
    check_for_hammer();
    fp_set_story_progress(0x22);
    fp_set_global_flag(0x52f, 1); //talked to kolorado
    fp_warp(0x12, 0xd, 0);
}

void load_ch5_card_lzs() {
    fp_set_story_progress(0x23);
    fp_warp(0x12, 0xd, 1);
}

void load_early_laki() {
    check_for_hammer();
    fp_set_story_progress(0x2b);
    fp_warp(0x13, 0, 1);
}

void load_yellow_berry_skip() {
    check_for_hammer();
    fp_set_story_progress(0x2d);
    fp_set_global_flag(0x554, 0); //yellow flower gate
    fp_warp(0x13, 0, 6);
}

void load_peach_warp() {
    check_for_hammer();
    fp_set_story_progress(0x33);
    set_partner(LAKILESTER);
    fp_warp(0x12, 0xc, 0);
}

void load_sushie_peach_warp() {
    check_for_hammer();
    fp_set_story_progress(0x33);
    set_partner(BOMBETTE);
    pm_player.party.bombette.in_party = 1;
    pm_player.party.sushie.in_party = 1;
    pm_player.stats.boots_upgrade = 1;
    fp_warp(0x12, 0x2, 2);
}

void load_ch6_card_lzs() {
    fp_set_story_progress(0x38);
    fp_warp(0x13, 0xf, 0);
}

void load_clippy_boots() {
    fp_set_global_flag(0x18d, 1); //super block broken
    fp_set_global_flag(0x18e, 0); //ultra block unbroken
    set_partner(LAKILESTER);
    pm_player.party.sushie.in_party = 1;
    pm_player.party.lakilester.in_party = 1;
    fp_warp(2, 0x13, 0);
}

void load_murder_solved_early() {
    set_partner(LAKILESTER);
    fp_set_story_progress(0x3e);
    fp_warp(0x14, 0xa, 0);
}

void load_sushie_glitch() {
    set_partner(SUSHIE);
    fp_set_story_progress(0x43);
    fp_warp(0x14, 0xa, 0);
}

void load_ice_staircase_skip() {
    set_partner(LAKILESTER);
    fp_set_story_progress(0x4b);
    fp_warp(0x14, 9, 0);
}

void load_mirror_clip() {
    set_partner(LAKILESTER);
    fp_set_story_progress(0x4e);
    fp_warp(0x15, 1, 1);
}

void load_kooper_puzzle_skip() {
    set_partner(KOOPER);
    fp_set_story_progress(0x51);
    fp_warp(0x15, 0xf, 0);
}

void load_fast_basement_first() {
    fp_set_story_progress(0x5e);
    fp_set_global_flag(0x60e, 0); //first switch
    fp_set_global_flag(0x60f, 0); //second switch
    fp_set_global_flag(0x615, 0); //basement fight
    fp_set_global_flag(0x614, 0); //hardened lava
    pm_player.stats.boots_upgrade = 2;
    fp_warp(0x16, 7, 0);
}

void load_fast_basement_second() {
    fp_set_story_progress(0x5e);
    fp_set_global_flag(0x60e, 0); //first switch
    fp_set_global_flag(0x60f, 0); //second switch
    fp_set_global_flag(0x615, 0); //basement fight
    fp_set_global_flag(0x614, 0); //hardened lava
    pm_player.stats.boots_upgrade = 2;
    fp_warp(0x16, 8, 0);
}

void load_basement_skip() {
    set_partner(PARAKARRY);
    fp_set_global_flag(0x614, 1); //hardened lava
    fp_warp(0x16, 0x17, 2);
}

void load_blind_basement() {
    set_partner(PARAKARRY);
    fp_warp(0x16, 0, 0);
}

void load_fast_flood_room() {
    set_partner(LAKILESTER);
    pm_player.party.kooper.in_party = 1;
    pm_player.party.lakilester.in_party = 1;
    fp_set_global_flag(0x632, 0); //key not collected
    fp_set_global_flag(0x633, 0); //spring still in wall
    fp_set_global_byte(0x129, 0); //water level 0
    fp_set_area_flag(0x8, 0); //blue switch not pressed
    remove_key_item(0x01a); //castle key 2
    fp_warp(0x16, 0x31, 1);
}

void load_cannonless() {
    set_partner(LAKILESTER);
    fp_warp(0x16, 0x2f, 0);
}

void load_luigi_skip() {
    set_partner(NONE);
    fp_set_story_progress(0x60);
    fp_warp(0x0, 0xb, 0x3);
}

void load_trick(int8_t trick) {
    switch (trick) {
        case JR_SKIP:                   load_jr_skip();                     break;
        case BLACK_TOAD_SKIP:           load_black_toad_skip();             break;
        case RETRIGGER_PROLOGUE:        load_retrigger_prologue();          break;
        case OOT_ACE:                   load_oot_ace();                     break;

        case STAIRCASE_SKIP:            load_staircase_skip();              break;
        case PIT:                       load_pit();                         break;
        case PIE_JUMPS:                 load_pie_jumps();                   break;
        case LOG_SKIP:                  load_log_skip();                    break;

        case ROCK_CLIMB:                load_rock_climb();                  break;
        case EARLY_SEED:                load_early_seed();                  break;
        case BUZZAR_SKIP:               load_buzzar_skip();                 break;
        case OUTPOST_JUMP:              load_outpost_jump();                break;
        case SPIKE_SHIELD_EARLY:        load_spike_shield_early();          break;
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
        case GOURMET_GUY_SKIP:          load_gourmet_guy_skip();            break;
        case CH4_CARD_LZS:              load_ch4_card_lzs();                break;

        case BHS_TOP:                   load_bhs_top();                     break;
        case BHS_BOTTOM:                load_bhs_bottom();                  break;
        case EARLY_WHALE_SLOW_MUSIC:    load_early_whale_slow_music();      break;
        case EARLY_FUZZIPEDE:           load_early_fuzzipede();             break;
        case RAPH_SKIP:                 load_raph_skip();                   break;
        case PIRANHA_FIRST_STRIKE:      load_piranha_first_strike();        break;
        case LAVA_PLATFORM_CYCLE:       load_lava_platform_cycle();         break;
        case ULTRA_HAMMER_SKIP:         load_ultra_hammer_skip();           break;
        case LAVA_PUZZLE_SKIP:          load_lava_puzzle_skip();            break;
        case ULTRA_HAMMER_EARLY:        load_ultra_hammer_early();          break;
        case FLARAKARRY:                load_flarakarry();                  break;
        case LAVA_PIRANHA_SKIP:         load_lava_piranha_skip();           break;
        case CH5_CARD_LZS:              load_ch5_card_lzs();                break;

        case EARLY_LAKI:                load_early_laki();                  break;
        case YELLOW_BERRY_SKIP:         load_yellow_berry_skip();           break;
        case PEACH_WARP:                load_peach_warp();                  break;
        case SUSHIE_PEACH_WARP:         load_sushie_peach_warp();           break;
        case CH6_CARD_LZS:              load_ch6_card_lzs();                break;

        case CLIPPY_BOOTS:              load_clippy_boots();                break;
        case MURDER_SOLVED_EARLY:       load_murder_solved_early();         break;
        case SUSHIE_GLITCH:             load_sushie_glitch();               break;
        case ICE_STAIRCASE_SKIP:        load_ice_staircase_skip();          break;
        case MIRROR_CLIP:               load_mirror_clip();                 break;
        case KOOPER_PUZZLE_SKIP:        load_kooper_puzzle_skip();          break;

        case FAST_BASEMENT_FIRST:       load_fast_basement_first();         break;
        case FAST_BASEMENT_SECOND:      load_fast_basement_second();        break;
        case BASEMENT_SKIP:             load_basement_skip();               break;
        case BLIND_BASEMENT:            load_blind_basement();              break;
        case FAST_FLOOD_ROOM:           load_fast_flood_room();             break;
        case CANNONLESS:                load_cannonless();                  break;
        case LUIGI_SKIP:                load_luigi_skip();                  break;
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

static void pit_proc(struct menu_item *item, void *data) {
    fp.saved_trick = PIT;
    load_pit();
}

static void pie_jumps_proc(struct menu_item *item, void *data) {
    fp.saved_trick = PIE_JUMPS;
    load_pie_jumps();
}

static void log_skip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = LOG_SKIP;
    load_log_skip();
}

static void rock_climb_proc(struct menu_item *item, void *data) {
    fp.saved_trick = ROCK_CLIMB;
    load_rock_climb();
}

static void early_seed_proc(struct menu_item *item, void *data) {
    fp.saved_trick = EARLY_SEED;
    load_early_seed();
}

static void buzzar_skip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = BUZZAR_SKIP;
    load_buzzar_skip();
}

static void outpost_jump_proc(struct menu_item *item, void *data) {
    fp.saved_trick = OUTPOST_JUMP;
    load_outpost_jump();
}

static void spike_shield_early_proc(struct menu_item *item, void *data) {
    fp.saved_trick = SPIKE_SHIELD_EARLY;
    load_spike_shield_early();
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

static void gourmet_guy_skip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = GOURMET_GUY_SKIP;
    load_gourmet_guy_skip();
}

static void ch4_card_lzs_proc(struct menu_item *item, void *data) {
    fp.saved_trick = CH4_CARD_LZS;
    load_ch4_card_lzs();
}

static void bhs_top_proc(struct menu_item *item, void *data) {
    fp.saved_trick = BHS_TOP;
    load_bhs_top();
}

static void bhs_bottom_proc(struct menu_item *item, void *data) {
    fp.saved_trick = BHS_BOTTOM;
    load_bhs_bottom();
}

static void early_whale_slow_music_proc(struct menu_item *item, void *data) {
    fp.saved_trick = EARLY_WHALE_SLOW_MUSIC;
    load_early_whale_slow_music();
}

static void early_fuzzipede(struct menu_item *item, void *data) {
    fp.saved_trick = EARLY_FUZZIPEDE;
    load_early_fuzzipede();
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

static void ultra_hammer_skip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = ULTRA_HAMMER_SKIP;
    load_ultra_hammer_skip();
}

static void lava_puzzle_skip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = LAVA_PUZZLE_SKIP;
    load_lava_puzzle_skip();
}

static void ultra_hammer_early_proc(struct menu_item *item, void *data) {
    fp.saved_trick = ULTRA_HAMMER_EARLY;
    load_ultra_hammer_early();
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

static void sushie_peach_warp_proc(struct menu_item *item, void *data) {
    fp.saved_trick = SUSHIE_PEACH_WARP;
    load_sushie_peach_warp();
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

static void fast_basement_first_proc(struct menu_item *item, void *data) {
    fp.saved_trick = FAST_BASEMENT_FIRST;
    load_fast_basement_first();
}

static void fast_basement_second_proc(struct menu_item *item, void *data) {
    fp.saved_trick = FAST_BASEMENT_SECOND;
    load_fast_basement_second();
}

static void basement_skip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = BASEMENT_SKIP;
    load_basement_skip();
}

static void blind_basement_proc(struct menu_item *item, void *data) {
    fp.saved_trick = BLIND_BASEMENT;
    load_blind_basement();
}

static void fast_flood_room_proc(struct menu_item *item, void *data) {
    fp.saved_trick = FAST_FLOOD_ROOM;
    load_fast_flood_room();
}

static void cannonless_proc(struct menu_item *item, void *data) {
    fp.saved_trick = CANNONLESS;
    load_cannonless();
}

static void luigi_skip_proc(struct menu_item *item, void *data) {
    fp.saved_trick = LUIGI_SKIP;
    load_luigi_skip();
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
    menu_add_button(page, 0, y_tab++, "the pit", pit_proc, NULL);
    menu_add_button(page, 0, y_tab++, "pie jumps", pie_jumps_proc, NULL);
    menu_add_button(page, 0, y_tab++, "log skip", log_skip_proc, NULL);

    /* chapter 2 */
    y_tab = 0;
    page = &pages[2];
    menu_add_static(page, 0, y_tab++, "chapter 2", 0xC0C0C0);
    menu_add_button(page, 0, y_tab++, "rock climb", rock_climb_proc, NULL);
    menu_add_button(page, 0, y_tab++, "early seed", early_seed_proc, NULL);
    menu_add_button(page, 0, y_tab++, "buzzar skip", buzzar_skip_proc, NULL);
    menu_add_button(page, 0, y_tab++, "outpost jump", outpost_jump_proc, NULL);
    menu_add_button(page, 0, y_tab++, "spike shield early", spike_shield_early_proc, NULL);
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
    menu_add_button(page, 0, y_tab++, "gourmet guy skip", gourmet_guy_skip_proc, NULL);
    menu_add_button(page, 0, y_tab++, "ch4 card lzs", ch4_card_lzs_proc, NULL);

    /* chapter 5 */
    y_tab = 0;
    page = &pages[5];
    menu_add_static(page, 0, y_tab++, "chapter 5", 0xC0C0C0);
    menu_add_button(page, 0, y_tab++, "blue house skip (top)", bhs_top_proc, NULL);
    menu_add_button(page, 0, y_tab++, "blue house skip (bottom)", bhs_bottom_proc, NULL);
    menu_add_button(page, 0, y_tab++, "early whale (slow music)", early_whale_slow_music_proc, NULL);
    menu_add_button(page, 0, y_tab++, "early fuzzipede", early_fuzzipede, NULL);
    menu_add_button(page, 0, y_tab++, "raph/yoshi skip", raph_skip_proc, NULL);
    menu_add_button(page, 0, y_tab++, "putrid piranha first strike", piranha_first_strike_proc, NULL);
    menu_add_button(page, 0, y_tab++, "lava platform cycle", lava_platform_cycle_proc, NULL);
    menu_add_button(page, 0, y_tab++, "ultra hammer skip", ultra_hammer_skip_proc, NULL);
    menu_add_button(page, 0, y_tab++, "lava puzzle skip", lava_puzzle_skip_proc, NULL);
    menu_add_button(page, 0, y_tab++, "ultra hammer early", ultra_hammer_early_proc, NULL);
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
    menu_add_button(page, 0, y_tab++, "sushie peach warp", sushie_peach_warp_proc, NULL);
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
    menu_add_button(page, 0, y_tab++, "fast basement (first room)", fast_basement_first_proc, NULL);
    menu_add_button(page, 0, y_tab++, "fast basement (second room)", fast_basement_second_proc, NULL);
    menu_add_button(page, 0, y_tab++, "basement skip", basement_skip_proc, NULL);
    menu_add_button(page, 0, y_tab++, "blind basement", blind_basement_proc, NULL);
    menu_add_button(page, 0, y_tab++, "fast flood room", fast_flood_room_proc, NULL);
    menu_add_button(page, 0, y_tab++, "cannonless", cannonless_proc, NULL);
    menu_add_button(page, 0, y_tab++, "luigi skip", luigi_skip_proc, NULL);

    menu_tab_goto(tab, 0);
    menu_add_button(menu, 8, 0, "<", tab_prev_proc, tab);
    menu_add_button(menu, 10, 0, ">", tab_next_proc, tab);
}
