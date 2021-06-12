#ifndef _PM64_H
#define _PM64_H
#include <n64.h>
#include <stdint.h>

#define PM64_SCREEN_WIDTH    320
#define PM64_SCREEN_HEIGHT   240

typedef struct{
    /* 0x0000 */ float x;
    /* 0x0004 */ float y;
    /* 0x0008 */ float z;
}xyz_t; // size: 0x000C

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
}vec3s_t;

typedef struct{
    /* 0x0000 */ int8_t x_cardinal;
    /* 0x0001 */ int8_t y_cardinal;
    union
    {
        struct
        {
            uint16_t a  : 1;
            uint16_t b  : 1;
            uint16_t z  : 1;
            uint16_t s  : 1;
            uint16_t du : 1;
            uint16_t dd : 1;
            uint16_t dl : 1;
            uint16_t dr : 1;
            uint16_t    : 2;
            uint16_t l  : 1;
            uint16_t r  : 1;
            uint16_t cu : 1;
            uint16_t cd : 1;
            uint16_t cl : 1;
            uint16_t cr : 1;
      };
      /* 0x0003 */ uint16_t buttons;
    };
}controller_t; //size: 0x0004

typedef struct{
    /* 0x0000 */ controller_t raw; /* raw input */
    /* 0x0004 */ char unk_0x04[0x0C];
    /* 0x0010 */ controller_t pressed; /* one frame when pressed*/
    /* 0x0014 */ char unk_0x14[0x0C];
    /* 0x0020 */ controller_t pad_held; /* once every 4 frames */
    /* 0x0024 */ char unk_0x24[0x0C];
    /* 0x0030 */ controller_t previous;
    /* 0x0034 */ char unk_0x34[0x0C];
    /* 0x0040 */ int8_t control_x;
    /* 0x0041 */ char unk_0x41[0x03];
    /* 0x0044 */ int8_t control_y;
    /* 0x0045 */ char unk_0x45[0x03];
    /* 0x0048 */ int16_t hold_timer_start_value; /* writes 0F to hold_timer when input is pressed*/
    /* 0x004A */ char unk_0x4A[0x02];
    /* 0x004C */ char unk_0x4C[0x0C];
    /* 0x0058 */ int16_t hold_timer; /*counts from 0F to 00 to start held variable*/
    /* 0x005A */ char unk_0x5A[0x02];
    /* 0x005C */ char unk_0x5C[0x04];
    /* 0x0060 */ int16_t held_timer; /*FFFF when nothing held, when hold_timer hits 0 it will cycle between 0 and 3*/
    /* 0x0062 */ char unk_0x62[0x02];
    /* 0x0064 */ char unk_0x64[0xC];
    /* 0x0070 */ int8_t is_battle;
    /* 0x0071 */ int8_t demo_flag; /*1 for demo. 0 in normal gameplay freezes mario*/
    /* 0x0072 */ int8_t demo_scene; /*0-0x12 for each demo scene*/
    /* 0x0073 */ int8_t controller_plugged; /*needs to be 1 otherwise "no controller" */
    /* 0x0074 */ uint8_t battle_debug; /* 0=normal, 1=enemies can't interact, 2=defeat enemies on contact, 3=auto-defeat enemies in battle, 4=auto run away */
    /* 0x0075 */ uint8_t quizmo_debug; /* 1 to force a quizmo spawn every time */
    /* 0x0076 */ int8_t unk_0x76;
    /* 0x0077 */ char unk_0x77;
    /* 0x0078 */ int8_t disable_scripts;
    /* 0x0079 */ char unk_0x79;
    /* 0x007A */ int8_t music_enabled;
    /* 0x007B */ char unk_0x7B;
    /* 0x007C */ int8_t unk_0x7C;
    /* 0x007D */ int8_t unk_0x7D;
    /* 0x007E */ uint8_t peach_flags; /*bitfield, 1 = isPeach, 2 = isTransformed, 3 = hasUmbrella*/
    /* 0x007F */ int8_t peach_disguise; /*1 = koopatrol, 2 = hammer bro, 3 = clubba */
    /* 0x0080 */ uint8_t peach_anim_idx;
    /* 0x0081 */ char unk_0x84[0x05];
    /* 0x0086 */ uint16_t group_id;
    /* 0x0088 */ uint16_t group_id_copy; /* used for calculating did_area_change */
    /* 0x008A */ int16_t did_area_change;
    /* 0x008C */ uint16_t room_id;
    /* 0x008E */ uint16_t entrance_id;
    /* 0x0090 */ uint16_t unk_0x90;
    /* 0x0092 */ uint16_t unk_0x92;
    /* 0x0094 */ float loading_zone_tangent;
    /* 0x0098 */ xyz_t player_position_copy;
    /* 0x00A4 */ float player_angle_copy;
    /* 0x00A8 */ char unk_A8[0x04];
    /* 0x00AC */ int8_t load_menu_state;
    /* 0x00AD */ int8_t menu_counter;
    /* 0x00AE */ int8_t skip_intro;
    /* 0x00AF */ char unk_AF[0x07];
    /* 0x00B6 */ int16_t boot_alpha;
    /* 0x00B8 */ int16_t boot_blue;
    /* 0x00BA */ int16_t boot_green;
    /* 0x00BC */ int16_t boot_red;
    /* 0x00BE */ char unk_BE[0x6A];
    /* 0x0128 */ xyz_t player_trace_normal;
    /* 0x0134 */ uint16_t frame_counter;
    /* 0x0136 */ char unk_136[0x02];
    /* 0x0138 */ int32_t next_rng;
    /* 0x013C */ int16_t unk_13C;
    /* 0x013E */ char unk_13E[0x0A];
    /* 0x0148 */ int16_t enable_background; /* (bit 2 is also used for something) */
    /* 0x014A */ int16_t background_min_w;
    /* 0x014C */ int16_t background_min_h;
    /* 0x014E */ int16_t background_max_w;
    /* 0x0150 */ int16_t background_max_h;
    /* 0x0152 */ int16_t background_x_offset; /* used for parallax scroll */
    /* 0x0154 */ void *background_raster;
    /* 0x0158 */ void *background_palette;
    /* 0x015C */ int16_t unk_15C;
    /* 0x015E */ uint16_t unk_15E;
    /* 0x0160 */ vec3s_t saved_pos;
    /* 0x0166 */ uint8_t save_slot;
    /* 0x0167 */ uint8_t load_type; /* 0 = from map, 1 = from main menu */
    /* 0x0168 */ int32_t save_count;
}status_ctxt_t;

typedef struct{
    /* 0x0000 */ char unk_0x00[0x10];
    /* 0x0010 */ uint32_t saveblock_freeze; /*00000001 = frozen, loading zones dissapear. can be used for lzs*/
}unk1_ctxt_t;

typedef struct {
    /* 0x00000 */ char unk_0x00[0x230];
    /* 0x00230 */ Gfx buf[0x2000];
    /* 0x10230 */ Gfx buf_work[0x200];
    /* 0x11230 */ Mtx unk_mtx[0x200];
}disp_buf_t; //size: 0x19230

typedef struct{
    /* 0x00000 */ Gfx *p;
    /* 0x00004 */ uint32_t unk;
    /* 0x00008 */ disp_buf_t *disp_buf;
}gfx_ctxt_t; // size: 0x0000C

typedef struct{
    /* 0x0000 */ char unk_0x00[0x84];
    /* 0x0084 */ uint16_t room_change_state;
}unk2_ctxt_t;

typedef struct{
    /* 0x0000 */ uint32_t global_flags[64];
    /* 0x0100 */ int8_t global_bytes[512];
    uint32_t area_flags[8];
}flags_ctxt_t;

typedef struct{
    /* 0x0000 */ int8_t story_progress;
}unk3_ctxt_t;

typedef struct{
    /* 0x0000 */ char unk_0x00[0x03];
    /* 0x0003 */ uint8_t partner_ability;
    /* 0x0004 */ int16_t control_x_overworld;
    /* 0x0006 */ int16_t control_y_overworld;
    /* 0x0008 */ controller_t raw_overworld;
    /* 0x000C */ controller_t pressed1_overworld;
    /* 0x0010 */ controller_t pressed2_overworld;
}overworld_ctxt_t; // size: 0x00014

typedef struct{
    /* 0x00000 */ uint32_t hp_text;
    /* 0x00004 */ uint32_t hp_icon;
    /* 0x00008 */ uint32_t fp_text;
    /* 0x0000C */ uint32_t fp_icon;
    /* 0x00010 */ uint32_t coin_icon_background;
    /* 0x00014 */ uint32_t coin_icon;
    /* 0x00018 */ uint32_t star_points_icon;
    /* 0x0001C */ uint32_t star_points_glare;
    /* 0x00020 */ uint32_t unk_icon1;
    /* 0x00024 */ uint32_t unk_icon2;
    /* 0x00028 */ uint32_t unk_icon3;
    /* 0x0002C */ uint32_t unk_icon4;
    /* 0x00030 */ uint32_t unk_icon5;
    /* 0x00034 */ int16_t hud_x;
    /* 0x00036 */ int16_t hud_y;
    /* 0x00038 */ uint16_t hud_timer; /*hud will show up after 0xF0 frames of idle*/
    /* 0x0003A */ uint8_t hud_hidden; /*0 for shown, 1 for hidden*/
    /* 0x0003B */ char unk_0x3B[0x01];
    /* 0x0003C */ uint8_t hud_shown; /*1 for shown, 0 for hidden. this flag does not affect menu behavior*/
    /* 0x0003D */ int8_t hp_value;
    /* 0x0003E */ int8_t fp_value;
    /* 0x0003F */ char unk_0x3F[0x01];
    /* 0x00040 */ uint16_t coin_value;
    /* 0x00042 */ char unk_0x42[0x02];
    /* 0x00044 */ char unk_0x44[0x04];
    /* 0x00048 */ uint16_t star_power_value;
    /* 0x0004A */ uint8_t hp_is_flashing; /*non-zero value will flash*/
    /* 0x0004B */ uint8_t hp_flash_counter;
    /* 0x0004C */ char unk_0x4C[0x01];
    /* 0x0004D */ uint8_t fp_is_flashing; /*non-zero value will flash*/
    /* 0x0004E */ uint8_t fp_flash_counter;
    /* 0x0004F */ char unk_0x4F[0x01];
    /* 0x00050 */ uint8_t unk_is_flashing; /*think this is star power? cant check rn*/
    /* 0x00051 */ uint8_t unk_flash_counter;
    /* 0x00052 */ uint8_t star_points_is_flashing; /*non-zero value will flash*/
    /* 0x00053 */ uint8_t star_points_flash_counter;
    /* 0x00054 */ uint8_t coin_is_flashing; /*non-zero value will flash*/
    /* 0x00055 */ uint8_t coin_flash_counter;
    /* 0x00056 */ char unk_0x56[0x02];
    /* 0x00058 */ char unk_0x58[0x18];
}hud_ctxt_t; // size: 0x00070

typedef struct{
    /* 0x0000 */ uint8_t boots_upgrade; /*start: 8010F450*/
    /* 0x0001 */ uint8_t hammer_upgrade;
    /* 0x0002 */ uint8_t hp;
    /* 0x0003 */ uint8_t max_hp;
    /* 0x0004 */ uint8_t menu_max_hp; /*gets copied to max_hp when unpausing */
    /* 0x0005 */ uint8_t fp;
    /* 0x0006 */ uint8_t max_fp;
    /* 0x0007 */ uint8_t menu_max_fp; /*gets copied to max_hp when unpausing */
    /* 0x0008 */ uint8_t bp;
    /* 0x0009 */ uint8_t level;
    /* 0x000A */ uint8_t has_action_command;
    /* 0x000B */ char unk_0x0B[0x01];
    /* 0x000C */ int16_t coins;
    /* 0x000E */ uint8_t fortress_keys;
    /* 0x000F */ uint8_t star_pieces;
    /* 0x0010 */ uint8_t star_points;
    /* 0x0011 */ char unk_0x11[0x01];
    /* 0x0012 */ uint8_t current_partner; /*0x00 - 0x0B*/
    /* 0x0013 */ char unk_0x13[0x01];
}stats_t; // size: 0x0014

typedef struct {
    /* 0x00000 */ uint8_t in_party;
    /* 0x00001 */ uint8_t upgrade;
    /* 0x00002 */ char unk_0x02[0x06];
}partner_t; // size: 0x00008

typedef struct{
    /* 0x0000 */ partner_t goombario;
    /* 0x0008 */ partner_t kooper;
    /* 0x0010 */ partner_t bombette;
    /* 0x0018 */ partner_t parakarry;
    /* 0x0020 */ partner_t goompa;
    /* 0x0028 */ partner_t watt;
    /* 0x0030 */ partner_t sushie;
    /* 0x0038 */ partner_t lakilester;
    /* 0x0040 */ partner_t bow;
    /* 0x0048 */ partner_t goombaria;
    /* 0x0050 */ partner_t twink;
}party_t; // size: 0x0058

typedef struct{
    /* 0x0000 */ char unk_0x00[0x01];
    /* 0x0001 */ uint8_t spell_type;
    /* 0x0002 */ uint8_t casts_remaining;
    /* 0x0003 */ char unk_0x03[0x02];
    /* 0x0005 */ uint8_t turns_until_spell;
}merlee_t; // size: 0x0006

typedef struct{
    /* 0x0000 */ uint8_t star_spirits_saved;
    /* 0x0001 */ char unk_0x01[0x01];
    /* 0x0002 */ uint8_t full_bars_filled;
    /* 0x0003 */ uint8_t partial_bars_filled;
    /* 0x0004 */ uint8_t beam_rank; /*1 for star beam, 2 for peach beam*/
    /* 0x0005 */ char unk_0x05[0x01];
}star_power_t; // size: 0x0006

typedef struct{
    /* 0x0000 */ int32_t flags; /*third byte related to cutscenes - write 0 to break free*/
    /* 0x0004 */ int32_t anim_flags;
    /* 0x0008 */ uint16_t idle_timer;
    /* 0x000A */ char unk_0x0A[0x02];
    /* 0x000C */ uint16_t peach_disguise; /*has something to do with peach transforming in ch6*/
    /* 0x000E */ uint16_t transparency; /*0xFF00 when spinning*/
    /* 0x0010 */ uint16_t flash_timer; /*used when running away*/
    /* 0x0012 */ char unk_0x12[0x02];
    /* 0x0014 */ uint16_t busy; /*changed when talking/opening doors/loading zones*/
    /* 0x0016 */ int16_t truncated_x; /*used for hazard respawns*/
    /* 0x0018 */ char unk_0x18[0x02];
    /* 0x001A */ int16_t truncated_z; /*used for hazard respawns*/
    /* 0x001C */ char unk_0x1C[0x0C];
    /* 0x0028 */ xyz_t position;
    /* 0x0034 */ char unk_0x34[0x08];
    /* 0x003C */ float jumped_from_x;
    /* 0x0040 */ float jumped_from_z;
    /* 0x0044 */ float landed_at_x;
    /* 0x0048 */ float landed_at_z;
    /* 0x004C */ float jumped_from_y;
    /* 0x0050 */ float last_jump_height;
    /* 0x0054 */ float speed;
    /* 0x0058 */ float walk_speed; /*constant: 0x40000000 = 2.0*/
    /* 0x005C */ float run_speed; /*constant: 0x40800000 = 4.0*/
    /* 0x0060 */ char unk_0x60[0x0C];
    /* 0x006C */ float jump_const; /*used by jumping func to compare if jump_var_1 less than const*/
    /* 0x0070 */ float y_speed; /*related to rise/fall speed*/
    /* 0x0074 */ float y_acceleration; /*related to height cap*/
    /* 0x0078 */ float y_jerk; /*related to height cap*/
    /* 0x007C */ float y_snap; /*related to height cap*/
    /* 0x0080 */ float movement_angle; /*locking this makes you move in only that direction regardless of control stick angle*/
    /* 0x0084 */ float facing_angle;
    /* 0x0088 */ char unk_0x88[0x08];
    /* 0x0090 */ float body_rotation; /*used for turning effect*/
    /* 0x0094 */ char unk_0x94[0x10];
    /* 0x00A4 */ int32_t sprite_animation; /* 1st byte: back turned=01 | 4th byte: animations 00-32*/
    /* 0x00A8 */ float left_right; /*0.0=left, 180.0=right*/
    /* 0x00AC */ char unk_0xAC[0x04];
    /* 0x00B0 */ int16_t collider_height;
    /* 0x00B2 */ int16_t collider_diameter;
    /* 0x00B4 */ uint8_t action_state;
    /* 0x00B5 */ uint8_t prev_action_state;
    /* 0x00B6 */ int8_t fall_state; /*also used as sleep state in Peach idle action*/
    /* 0x00B7 */ char unk_B7[0x09];
    /* 0x00C0 */ uint16_t frames_in_air;
    /* 0x00C2 */ char unk_0xC2[0x02];
    /* 0x00C4 */ char unk_0xC4[0x02];
    /* 0x00C6 */ uint16_t interactable_id; /*only for doors?*/
    /* 0x00C8 */ uint32_t *talkable_npc;
    /* 0x00CC */ char unk_0xCC[0x08];
    /* 0x00D4 */ float spin_variable;
    /* 0x00D8 */ char unk_0xD8[0x04];
    /* 0x00DC */ controller_t raw;
    /* 0x00E0 */ controller_t previous;
    /* 0x00E4 */ controller_t pad_held;
    /* 0x00E8 */ int32_t pad_x;
    /* 0x00EC */ int32_t pad_y;
    /* 0x00F0 */ controller_t pad_held_list[10];
    /* 0x0118 */ controller_t pad_pressed_list[10];
    /* 0x0140 */ controller_t held_timer_list[10];
    /* 0x0168 */ int32_t pad_x_list[10];
    /* 0x0190 */ int32_t pad_y_list[10];
    /* 0x01B8 */ uint32_t timer;
    /* 0x01BC */ char unk_0x1BC[0xCC];
    /* 0x0288 */ uint8_t spin_cooldown_timer; /*4 frames at the end of spin*/
    /* 0x0289 */ char unk_0x289[0x02];
    /* 0x028B */ uint8_t spin_timer;
    /* 0x028C */ char unk_0x28C[0x20];
    /* 0x02AC */ float spin_speed;
    /* 0x02B0 */ char unk_0x2B0[0x04];
    /* 0x02B4 */ char unk_0x2B4[0x01];
    /* 0x02B5 */ uint8_t spin_duration;
    /* 0x02B6 */ char unk_0x228[0x02];
    /* 0x02B8 */ char unk_0x2B8[0x10];
    /* 0x02C8 */ stats_t stats;
    /* 0x02DC */ char unk_0x14[0x08];
    /* 0x02E4 */ party_t party;
    /* 0x033C */ uint16_t key_items[32];
    /* 0x037C */ uint16_t badges[128];
    /* 0x047C */ uint16_t items[10];
    /* 0x0490 */ uint16_t storage[32];
    /* 0x04D0 */ uint16_t equipped_badges[64];
    /* 0x0550 */ merlee_t merlee;
    /* 0x0556 */ star_power_t star_power;
    /* 0x055C */ int16_t other_hits_taken;
    /* 0x055E */ int16_t unk_55E;
    /* 0x0560 */ int16_t hits_taken;
    /* 0x0562 */ int16_t hits_blocked;
    /* 0x0564 */ int16_t player_first_strikes;
    /* 0x0566 */ int16_t enemy_first_strikes;
    /* 0x0568 */ int16_t power_bounces;
    /* 0x056A */ int16_t battle_count;
    /* 0x056C */ int16_t unk_56C[4];
    /* 0x0574 */ int32_t unk_574[2];
    /* 0x057C */ uint32_t total_coins_earned;
    /* 0x0580 */ int16_t idle_frame_counter; /* frames with no inputs, overflows every ~36 minutes of idling */
    /* 0x0582 */ char unk_582[2];
    /* 0x0584 */ uint32_t frame_counter; /* increases by 2 per frame */
    /* 0x0588 */ int16_t quizzes_answered;
    /* 0x058A */ int16_t quizzes_correct;
    /* 0x058C */ int32_t unk_590[24];
    /* 0x05EC */ int32_t trade_event_start_time;
    /* 0x05F0 */ int32_t unk_5EC;
    /* 0x05F4 */ int16_t star_pieces_collected;
    /* 0x05F6 */ int16_t jump_game_plays;
    /* 0x05F8 */ int32_t jump_game_total; /* all-time winnings, max = 99999 */
    /* 0x05FC */ int16_t jump_game_record;
    /* 0x05FE */ int16_t smash_game_plays;
    /* 0x0600 */ int32_t smash_game_total; /* all-time winnings, max = 99999 */
    /* 0x0604 */ int16_t smash_game_record;
}player_ctxt_t;

typedef struct{
    /* 0x0000 */ char unk_0x00[0x08];
    /* 0x0008 */ uint32_t room_change_ptr;
    /* 0x000C */ char unk_0x0C[0x0C];
}warp_ctxt_t; // size: 0x0018

typedef struct{
    uint32_t effects[96];
}effects_ctxt_t;

typedef struct{
    uint32_t instructions[32];
}ace_ctxt_t;

typedef struct{
    uint16_t last_timer;
}ace_store_ctxt_t;

typedef struct{
    uint32_t rng;
}unk4_ctxt_t;

typedef struct{
    uint32_t vi_frames;
}unk5_ctxt_t;

typedef struct {
    /* 0x0000 */ char unk_0x00[0x03];
    /* 0x0003 */ uint8_t menu_open; /* seems to always be non-zero when a menu is open in battle */
}unk6_ctxt_t;

typedef struct {
    /* 0x0000 */ uint32_t turn;
    /* 0x0004 */ char unk_0x04[4];
    /* 0x0008 */ uint32_t turns_since_wave;
    /* 0x000C */ uint32_t turns_since_shield;
    /* 0x0010 */ uint32_t turns_since_claw;
    /* 0x0014 */ uint32_t turns_since_stomp;
    /* 0x0018 */ uint32_t turns_since_heal;
}bowser_ctxt_t;

typedef struct {
    /* 0x0000 */ uint16_t mash_bar; /* ranges from 0 to 10,000 */
}unk7_ctxt_t;

/* Addresses */
#define pm_FioValidateFileChecksum_addr     0x8002B0B8
#define pm_FioReadFlash_addr                0x8002B828
#define pm_FioWriteFlash_addr               0x8002B908
#define pm_LoadGame_addr                    0x8002B290
#define pm_status_addr                      0x80074004
#define pm_unk4_addr                        0x800743F0
#define pm_unk5_addr                        0x80093B64
#define pm_unk1_addr                        0x8009A5A8
#define pm_gfx_addr                         0x8009A64C
#define pm_unk2_addr                        0x8009E6D0
#define pm_unk6_addr                        0x800AC198
#define pm_effects_addr                     0x800B4378
#define pm_flags_addr                       0x800DBC50
#define pm_unk3_addr                        0x800DBD50
#define pm_overworld_addr                   0x8010ED70
#define pm_hud_addr                         0x8010F118
#define pm_player_addr                      0x8010F188
#define pm_GameUpdate_addr                  0x801181D4 //80112FC4 on US
#define pm_PlayAmbientSounds_addr           0x8014C418
#define pm_PlaySfx_addr                     0x8014ED64
#define pm_warp_addr                        0x80156740
#define pm_unk7_addr                        0x8029FF14
#define pm_SaveGame_addr                    0x802DC150
#define pm_bowser_addr                      0x803DE378
#define pm_ace_addr                         0x807BFFFC
#define pm_ace_store_addr                   0x807D0000

/* Data */
#define pm_status             (*(status_ctxt_t*)      pm_status_addr)
#define pm_unk5               (*(unk5_ctxt_t*)        pm_unk5_addr)
#define pm_unk1               (*(unk1_ctxt_t*)        pm_unk1_addr)
#define pm_gfx                (*(gfx_ctxt_t*)         pm_gfx_addr)
#define pm_unk2               (*(unk2_ctxt_t*)        pm_unk2_addr)
#define pm_unk6               (*(unk6_ctxt_t*)        pm_unk6_addr)
#define pm_unk4               (*(unk4_ctxt_t*)        pm_unk4_addr)
#define pm_effects            (*(effects_ctxt_t*)     pm_effects_addr)
#define pm_flags              (*(flags_ctxt_t*)       pm_flags_addr)
#define pm_unk3               (*(unk3_ctxt_t*)        pm_unk3_addr)
#define pm_overworld          (*(overworld_ctxt_t*)   pm_overworld_addr)
#define pm_hud                (*(hud_ctxt_t*)         pm_hud_addr)
#define pm_player             (*(player_ctxt_t*)      pm_player_addr)
#define pm_warp               (*(warp_ctxt_t*)        pm_warp_addr)
#define pm_unk7               (*(unk7_ctxt_t*)        pm_unk7_addr)
#define pm_bowser             (*(bowser_ctxt_t*)      pm_bowser_addr)
#define pm_ace                (*(ace_ctxt_t*)         pm_ace_addr)
#define pm_ace_store          (*(ace_store_ctxt_t*)   pm_ace_store_addr)

/*Function Prototypes*/
typedef int32_t (*pm_FioValidateFileChecksum_t) (void *buffer);
typedef void (*pm_FioReadFlash_t) (int32_t slot, void *buffer, uint32_t size);
typedef void (*pm_FioWriteFlash_t) (int32_t slot, void *buffer, uint32_t size);
typedef void (*pm_LoadGame_t) (uint8_t slot);
typedef void (*pm_GameUpdate_t) ();
typedef void (*pm_PlaySfx_t) (int32_t sound_id);
typedef void (*pm_PlayAmbientSounds_t) (int32_t sounds_id, int32_t fade_time);
typedef void (*pm_SaveGame_t) ();

/*Functions*/
#define pm_FioValidateFileChecksum  ((pm_FioValidateFileChecksum_t)  pm_FioValidateFileChecksum_addr)
#define pm_FioReadFlash             ((pm_FioReadFlash_t)             pm_FioReadFlash_addr)
#define pm_FioWriteFlash            ((pm_FioWriteFlash_t)            pm_FioWriteFlash_addr)
#define pm_LoadGame                 ((pm_LoadGame_t)                 pm_LoadGame_addr)
#define pm_GameUpdate               ((pm_GameUpdate_t)               pm_GameUpdate_addr)
#define pm_PlaySfx                  ((pm_PlaySfx_t)                  pm_PlaySfx_addr)
#define pm_PlayAmbientSounds        ((pm_PlayAmbientSounds_t)        pm_PlayAmbientSounds_addr)
#define pm_SaveGame                 ((pm_SaveGame_t)                 pm_SaveGame_addr)

#endif
