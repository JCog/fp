#ifndef PM64_H
#define PM64_H
#include <n64.h>
#include <stdint.h>
#include "enums.h"

#define PM64_SCREEN_WIDTH    320
#define PM64_SCREEN_HEIGHT   240

#ifndef PM64_VERSION
#error no pm64 version specified
#endif

#define PM64U    0x00
#define PM64J    0x01

typedef struct{
    /* 0x0000 */ float x;
    /* 0x0004 */ float y;
    /* 0x0008 */ float z;
} xyz_t; // size: 0x000C

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} vec3s_t;

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
} controller_t; //size: 0x0004

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
    /* 0x007E */ uint8_t peach_flags; /*bitfield, 1 = isPeach, 2 = isTransformed, 3 = hasParasol*/
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
} status_ctxt_t;

typedef struct{
    /* 0x0000 */ char unk_0x00[0x10];
    /* 0x0010 */ uint32_t saveblock_freeze; /*00000001 = frozen, loading zones dissapear. can be used for lzs*/
} unk1_ctxt_t;

typedef struct{
    /* 0x0000 */ uint8_t enable_partner_ability;
    /* 0x0001 */ char unk_0x01[0x02];
    /* 0x0003 */ uint8_t partner_ability;
    /* 0x0004 */ int16_t control_x_overworld;
    /* 0x0006 */ int16_t control_y_overworld;
    /* 0x0008 */ controller_t raw_overworld;
    /* 0x000C */ controller_t pressed1_overworld;
    /* 0x0010 */ controller_t pressed2_overworld;
} overworld_ctxt_t; // size: 0x00014

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
} hud_ctxt_t; // size: 0x00070

typedef struct {
    /* 0x00000 */ uint8_t in_party;
    /* 0x00001 */ uint8_t upgrade;
    /* 0x00002 */ char unk_0x02[0x06];
} partner_t; // size: 0x00008

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
} party_t; // size: 0x0058

typedef struct{
    /* 0x0000 */ char unk_0x00[0x01];
    /* 0x0001 */ uint8_t spell_type;
    /* 0x0002 */ uint8_t casts_remaining;
    /* 0x0003 */ char unk_0x03[0x02];
    /* 0x0005 */ uint8_t turns_until_spell;
} merlee_t; // size: 0x0006

typedef struct{
    /* 0x0000 */ uint8_t star_spirits_saved;
    /* 0x0001 */ char unk_0x01[0x01];
    /* 0x0002 */ uint8_t full_bars_filled;
    /* 0x0003 */ uint8_t partial_bars_filled;
    /* 0x0004 */ uint8_t beam_rank; /*1 for star beam, 2 for peach beam*/
    /* 0x0005 */ char unk_0x05[0x01];
} star_power_t; // size: 0x0006

typedef struct {
    /* 0x000 */ uint8_t boots_upgrade; /*start: 8010F450*/
    /* 0x001 */ uint8_t hammer_upgrade;
    /* 0x002 */ uint8_t hp;
    /* 0x003 */ uint8_t max_hp;
    /* 0x004 */ uint8_t menu_max_hp; /*gets copied to max_hp when unpausing */
    /* 0x005 */ uint8_t fp;
    /* 0x006 */ uint8_t max_fp;
    /* 0x007 */ uint8_t menu_max_fp; /*gets copied to max_hp when unpausing */
    /* 0x008 */ uint8_t bp;
    /* 0x009 */ uint8_t level;
    /* 0x00A */ uint8_t has_action_command;
    /* 0x00B */ char unk_0x0B[0x01];
    /* 0x00C */ int16_t coins;
    /* 0x00E */ uint8_t fortress_keys;
    /* 0x00F */ uint8_t star_pieces;
    /* 0x010 */ uint8_t star_points;
    /* 0x011 */ char unk_0x11[0x01];
    /* 0x012 */ uint8_t current_partner; /*0x00 - 0x0B*/
    /* 0x013 */ char unk_0x13[0x01];
    /* 0x014 */ char unk_14[8];
    /* 0x01C */ party_t party;
    /* 0x074 */ uint16_t key_items[32];
    /* 0x0B4 */ uint16_t badges[128];
    /* 0x1B4 */ uint16_t items[10];
    /* 0x1C8 */ uint16_t stored_items[32];
    /* 0x208 */ uint16_t equipped_badges[64];
    /* 0x288 */ merlee_t merlee;
    /* 0x28E */ star_power_t star_power;
    /* 0x294 */ int16_t other_hits_taken;
    /* 0x296 */ int16_t unk_296;
    /* 0x298 */ int16_t hits_taken;
    /* 0x29A */ int16_t hits_blocked;
    /* 0x29C */ int16_t player_first_strikes;
    /* 0x29E */ int16_t enemy_first_strikes;
    /* 0x2A0 */ int16_t power_bounces;
    /* 0x2A2 */ int16_t battle_count;
    /* 0x2A4 */ int16_t unk_2A4[4];
    /* 0x2AC */ int32_t unk_2AC[2];
    /* 0x2B4 */ uint32_t total_coins_earned;
    /* 0x2B8 */ int16_t idle_frame_counter; /* frames with no inputs, overflows every ~36 minutes of idling */
    /* 0x2BA */ char unk_2BA[2];
    /* 0x2BC */ uint32_t frame_counter; /* increases by 2 per frame */
    /* 0x2C0 */ int16_t quizzes_answered;
    /* 0x2C2 */ int16_t quizzes_correct;
    /* 0x2C4 */ int32_t unk_2C4[24];
    /* 0x324 */ int32_t trade_event_start_time;
    /* 0x328 */ int32_t unk_328;
    /* 0x32C */ int16_t star_pieces_collected;
    /* 0x32E */ int16_t jump_game_plays;
    /* 0x330 */ int32_t jump_game_total; /* all-time winnings, max = 99999 */
    /* 0x334 */ int16_t jump_game_record;
    /* 0x336 */ int16_t smash_game_plays;
    /* 0x338 */ int32_t smash_game_total; /* all-time winnings, max = 99999 */
    /* 0x33C */ int16_t smash_game_record;
    /* 0x33E */ char unk_606[2];
} player_data_t; // size: 0x340

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
    /* 0x02C8 */ player_data_t player_data;
} player_ctxt_t;

typedef struct {
    /* 0x0000 */ char magic_string[16]; /* "Mario Story 006" */
    /* 0x0010 */ int8_t padding[32]; /* always zero */
    /* 0x0030 */ int32_t crc1;
    /* 0x0034 */ int32_t crc2;
    /* 0x0038 */ int32_t save_slot;
    /* 0x003C */ int32_t save_count;
    /* 0x0040 */ player_data_t player_data;
    /* 0x0380 */ char unk_380[0xE8];
    /* 0x0468 */ int16_t group_id;
    /* 0x046A */ int16_t room_id;
    /* 0x046C */ int16_t entrance_id;
    /* 0x046E */ char unk_46E[2];
    /* 0x0470 */ uint32_t enemy_defeat_flags[600];
    /* 0x0DD0 */ uint32_t unk_DD0[120];
    /* 0x0FB0 */ uint32_t global_flags[64];
    /* 0x10B0 */ int8_t global_bytes[512];
    /* 0x12B0 */ uint32_t area_flags[8];
    /* 0x12D0 */ int8_t area_bytes[16];
    /* 0x12E0 */ char unk_12E0[6];
    /* 0x12E6 */ vec3s_t position;
    /* 0x12EC */ int32_t unk_12EC;
    /* 0x12F0 */ int8_t unk_12F0[12]; /* player name starts at 4th char */
    /* 0x12FC */ int32_t unk_12FC;
    /* 0x1300 */ int32_t unk_1300;
    /* 0x1304 */ char unk_1304[0x7C];
} save_data_ctxt_t;

typedef struct {
    /* 0x00 */ int32_t flags;
    /* 0x04 */ int32_t *read_pos;
    /* 0x08 */ int32_t *start_read_pos;
    /* 0x0C */ int32_t *ptr_property_list;
    /* 0x10 */ int32_t *image_addr;
    /* 0x14 */ int32_t *palette_addr;
    /* 0x18 */ int32_t mem_offset;
    /* 0x1C */ int32_t *hud_transform;
    /* 0x20 */ float unk_20;
    /* 0x24 */ float unk_24;
    /* 0x28 */ float unk_img_scale[2];
    /* 0x30 */ float uniform_scale;
    /* 0x34 */ float width_scale_f; /* X.10 fmt (divide by 1024.0 to get float) */
    /* 0x38 */ float height_scale_f; /* X.10 fmt (divide by 1024.0 to get float) */
    /* 0x3C */ int16_t render_pos_x;
    /* 0x3E */ int16_t render_pos_y;
    /* 0x40 */ uint8_t screen_pos_offset[2];
    /* 0x42 */ uint8_t world_pos_offset[3];
    /* 0x45 */ int8_t draw_size_preset;
    /* 0x46 */ int8_t tile_size_preset;
    /* 0x47 */ uint8_t update_timer;
    /* 0x48 */ uint8_t size_x; /* screen size? */
    /* 0x49 */ uint8_t size_y; /* screen size? */
    /* 0x4A */ uint8_t opacity;
    /* 0x4B */ int8_t tint[3];
    /* 0x4E */ int8_t custom_image_size[2];
    /* 0x40 */ int8_t custom_draw_size[2];
} hud_element_t; // size = 0x54

typedef struct {
    /* 0x00 */ int32_t  flags;
    /* 0x04 */ char unk_04;
    /* 0x05 */ uint8_t  type;
    /* 0x06 */ uint8_t  level;
    /* 0x07 */ uint8_t  max_hp;
    /* 0x08 */ int16_t  part_count;
    /* 0x0A */ char unk_0A[2];
    /* 0x0C */ void **parts_data;
    /* 0x10 */ void *script;
    /* 0x14 */ int32_t *status_table;
    /* 0x18 */ uint8_t  escape_chance;
    /* 0x19 */ uint8_t  air_lift_chance;
    /* 0x1A */ uint8_t  spook_chance;
    /* 0x1B */ uint8_t  base_status_chance;
    /* 0x1C */ uint8_t  up_and_away_chance;
    /* 0x1D */ uint8_t  spin_smash_req;
    /* 0x1E */ uint8_t  power_bounce_chance;
    /* 0x1F */ uint8_t  coin_reward;
    /* 0x20 */ int8_t size_x;
    /* 0x20 */ int8_t size_y;
    /* 0x22 */ int8_t hp_bar_offset_x;
    /* 0x23 */ int8_t hp_bar_offset_y;
    /* 0x24 */ int8_t status_icon_offset_x;
    /* 0x25 */ int8_t status_icon_offset_y;
    /* 0x26 */ int8_t status_message_offset_x;
    /* 0x27 */ int8_t status_message_offset_y;
} actor_desc_t; // size = 0x28

typedef struct {
    /* 0x00 */ xyz_t current_pos;
    /* 0x0C */ xyz_t goal_pos;
    /* 0x18 */ xyz_t unk_18;
    /* 0x24 */ char unk_24[24];
    /* 0x3C */ float acceleration;
    /* 0x40 */ float speed;
    /* 0x44 */ float velocity;
    /* 0x48 */ float angle;
    /* 0x4C */ float distance;
} actor_movement_t; // size = 0x50;

typedef struct {
    /* 0x00 */ xyz_t current_pos;
    /* 0x0C */ xyz_t goal_pos;
    /* 0x18 */ xyz_t unk_18;
    /* 0x24 */ char unk_24[24];
    /* 0x3C */ float acceleration;
    /* 0x40 */ float speed;
    /* 0x44 */ float velocity;
    /* 0x48 */ float angle;
    /* 0x4C */ float distance;
    /* 0x50 */ float bounce_divisor;
    /* 0x54 */ char unk_54[0x4];
    /* 0x58 */ int32_t  anim_jump_rise;
    /* 0x5C */ int32_t  anim_jump_fall;
    /* 0x60 */ int32_t  anim_jump_land;
} actor_movement_walk_t; // size = 0x64;

typedef struct {
    /* 0x00 */ int16_t actor_id;
    /* 0x02 */ int16_t part_id; /* sometimes loaded as byte from 0x3 */
    /* 0x04 */ int16_t pos[3];
    /* 0x0A */ char unk_0A[7];
    /* 0x11 */ uint8_t home_col; /* from xpos --> 0-3 */
    /* 0x12 */ uint8_t home_row; /* from ypos --> 0-3 */
    /* 0x13 */ uint8_t layer; /* from zpos? --> 0-1 */
} selectable_target_t; // size = 0x14

typedef struct {
    /* 0x000 */ int32_t flags;
    /* 0x004 */ char unk_04[4];
    /* 0x008 */ actor_desc_t *static_actor_data;
    /* 0x00C */ actor_movement_walk_t walk;
    /* 0x070 */ int16_t move_time;
    /* 0x072 */ int16_t move_arc_amplitude;
    /* 0x074 */ char unk_74[3];
    /* 0x077 */ uint8_t jump_part_index;
    /* 0x078 */ char unk_78[16];
    /* 0x088 */ int32_t var_table[16];
    /* 0x0C8 */ actor_movement_t fly;
    /* 0x118 */ float fly_elapsed;
    /* 0x11C */ char unk_11C[4];
    /* 0x120 */ int16_t fly_time;
    /* 0x122 */ int16_t fly_arc_amplitude;
    /* 0x124 */ char unk_124[17];
    /* 0x135 */ uint8_t foot_step_counter;
    /* 0x136 */ uint8_t actor_type;
    /* 0x137 */ char unk_137;
    /* 0x138 */ xyz_t home_pos;
    /* 0x144 */ xyz_t current_pos;
    /* 0x150 */ int16_t head_offset_x;
    /* 0x152 */ int16_t head_offset_y;
    /* 0x154 */ int16_t head_offset_z;
    /* 0x156 */ int16_t health_bar_position_x;
    /* 0x158 */ int16_t health_bar_position_y;
    /* 0x15A */ int16_t health_bar_position_z;
    /* 0x15C */ xyz_t rotation;
    /* 0x168 */ int16_t rotation_pivot_offset_x;
    /* 0x16A */ int16_t rotation_pivot_offset_y;
    /* 0x16C */ int16_t rotation_pivot_offset_z;
    /* 0x16E */ char unk_16E[2];
    /* 0x170 */ xyz_t scale;
    /* 0x17C */ xyz_t scale_modifier; /* multiplies normal scale factors componentwise */
    /* 0x188 */ float scaling_factor;
    /* 0x18C */ float yaw;
    /* 0x190 */ uint8_t size_x;
    /* 0x191 */ uint8_t size_y;
    /* 0x192 */ int16_t actor_id;
    /* 0x194 */ int8_t unk_194;
    /* 0x195 */ int8_t unk_195;
    /* 0x196 */ int8_t unk_196;
    /* 0x197 */ int8_t unk_197;
    /* 0x198 */ int8_t unk_198;
    /* 0x199 */ int8_t unk_199;
    /* 0x19A */ int8_t unk_19A;
    /* 0x19B */ char unk_19B[1];
    /* 0x19C */ int32_t actor_type_data1[6]; /* 4 = jump sound */
    /* 0x1B4 */ int16_t actor_type_data1b[2];
    /* 0x1B8 */ int8_t current_hp;
    /* 0x1B9 */ int8_t max_hp;
    /* 0x1BA */ char unk_1BA[2];
    /* 0x1BC */ uint8_t hp_fraction; /* used to render HP bar */
    /* 0x1BD */ char unk_1BD[3];
    /* 0x1C0 */ void *idle_script_source;
    /* 0x1C4 */ void *take_turn_script_source;
    /* 0x1C8 */ void *on_hit_script_source;
    /* 0x1CC */ void *on_turn_chance_script_source;
    /* 0x1D0 */ void *idle_script;
    /* 0x1D4 */ void *take_turn_script;
    /* 0x1D8 */ void *on_hit_script;
    /* 0x1DC */ void *on_turn_change_script;
    /* 0x1E0 */ int32_t idle_script_id;
    /* 0x1E4 */ int32_t take_turn_id;
    /* 0x1E8 */ int32_t on_hit_id;
    /* 0x1EC */ int32_t on_turn_change_id;
    /* 0x1F0 */ int8_t last_event_type;
    /* 0x1F1 */ uint8_t turn_priority;
    /* 0x1F2 */ uint8_t enemy_index; /* actorID = this | 200 */
    /* 0x1F3 */ uint8_t num_parts;
    /* 0x1F4 */ void *parts_table;
    /* 0x1F8 */ int16_t last_damage_taken;
    /* 0x1FA */ int16_t hp_change_counter;
    /* 0x1FC */ int16_t damage_counter;
    /* 0x1FE */ char unk_1FE[2];
    /* 0x200 */ int32_t** unk_200; // Probably a struct but not sure what yet
    /* 0x204 */ char unk_204[3];
    /* 0x207 */ uint8_t extra_coin_bonus;
    /* 0x208 */ int8_t unk_208;
    /* 0x209 */ char unk_209[3];
    /* 0x20C */ uint32_t *status_table;
    /* 0x210 */ int8_t debuff;
    /* 0x211 */ int8_t debuff_duration;
    /* 0x212 */ int8_t static_status; /* 0B = yes */
    /* 0x213 */ int8_t static_duration;
    /* 0x214 */ int8_t stone_status; /* 0C = yes */
    /* 0x215 */ int8_t stone_duration;
    /* 0x216 */ int8_t ko_status; /* 0D = yes */
    /* 0x217 */ int8_t ko_duration;
    /* 0x218 */ int8_t trans_status; /* 0E = yes */
    /* 0x219 */ int8_t trans_duration;
    /* 0x21A */ char unk_21A[2];
    /* 0x21C */ uint8_t status;
    /* 0x21D */ char unk_21D[3];
    /* 0x220 */ int8_t is_glowing;
    /* 0x221 */ uint8_t attack_boost;
    /* 0x222 */ int8_t defense_boost;
    /* 0x223 */ uint8_t chill_out_amount; /* attack reduction */
    /* 0x224 */ uint8_t chill_out_turns;
    /* 0x225 */ char unk_225[7];
    /* 0x22C */ selectable_target_t target_data[24];
    /* 0x40C */ int8_t target_list_length;
    /* 0x40D */ int8_t target_index_list[24]; /* into targetData */
    /* 0x425 */ int8_t selected_target_index; /* into target index list */
    /* 0x426 */ int8_t target_part_index;
    /* 0x427 */ char unk_427;
    /* 0x428 */ int16_t target_actor_id;
    /* 0x42A */ char unk_42A[2];
    /* 0x42C */ void *shadow; /* might be shadow ID */
    /* 0x430 */ float shadow_scale; /* = actor size / 24.0 */
    /* 0x434 */ int16_t render_mode; /* initially 0xD, set to 0x22 if any part is transparent */
    /* 0x436 */ int16_t unk_436;
    /* 0x438 */ int32_t x[2]; /* ??? see FUN_80253974 */
    /* 0x440 */ hud_element_t *ptr_defuff_icon;
} actor_t; // size = 0x444

typedef struct {
    /* 0x000 */ int32_t flags1;
    /* 0x004 */ int32_t flags2;
    /* 0x008 */ int32_t var_table[16];
    /* 0x048 */ uint8_t current_submenu;
    /* 0x049 */ char unk_49[3];
    /* 0x04C */ int8_t unk_4C;
    /* 0x04D */ int8_t unk_4D;
    /* 0x04E */ int8_t unk_4E;
    /* 0x04F */ int8_t unk_4F;
    /* 0x050 */ int8_t unk_50;
    /* 0x051 */ int8_t unk_51;
    /* 0x052 */ int8_t unk_52;
    /* 0x053 */ int8_t strats_last_cursor_pos;
    /* 0x054 */ char unk_54[8];
    /* 0x05C */ int8_t unk_5C;
    /* 0x05D */ int8_t unk_5D;
    /* 0x05E */ char unk_5E[4];
    /* 0x062 */ int8_t unk_62;
    /* 0x063 */ int8_t unk_63;
    /* 0x064 */ char unk_64[12];
    /* 0x070 */ int16_t unk_70;
    /* 0x072 */ char unk_72[2];
    /* 0x074 */ int32_t unk_74;
    /* 0x078 */ uint8_t total_star_points;
    /* 0x079 */ uint8_t pending_star_points; /* how many to add */
    /* 0x07A */ uint8_t increment_star_point_delay; /* related to star points, set to 0x28 when they are dropped */
    /* 0x07B */ uint8_t damage_taken;
    /* 0x07C */ uint8_t change_partner_allowed;
    /* 0x07D */ char unk_7D[4];
    /* 0x081 */ int8_t action_success;
    /* 0x082 */ char unk_82;
    /* 0x083 */ int8_t unk_83;
    /* 0x084 */ int8_t unk_84;
    /* 0x085 */ int8_t unk_85;
    /* 0x086 */ int8_t unk_86;
    /* 0x087 */ int8_t block_result; /* 0 = fail, 1 = success, -1 = mashed */
    /* 0x088 */ uint8_t item_uses_left; /* set to 2 for doublke dip, 3 for triple */
    /* 0x089 */ uint8_t hp_drain_count;
    /* 0x08A */ char unk_8A;
    /* 0x08B */ uint8_t hustle_turns; /* numTurns from hustle drink, normally 0 */
    /* 0x08C */ char unk_8C;
    /* 0x08D */ int8_t unk_8D;
    /* 0x08E */ uint8_t initial_enemy_count; /* used for SP award bonus */
    /* 0x08F */ char unk_8F[3];
    /* 0x092 */ int8_t unk_92;
    /* 0x093 */ char unk_93;
    /* 0x094 */ int8_t unk_94;
    /* 0x095 */ char unk_95;
    /* 0x096 */ int8_t hammer_charge;
    /* 0x097 */ int8_t jump_charge;
    /* 0x098 */ char unk_98;
    /* 0x099 */ uint8_t danger_flags; /* 1 = danger, 2 = peril */
    /* 0x09A */ int8_t outta_sight_active;
    /* 0x09B */ int8_t turbo_charge_turns_left;
    /* 0x09C */ uint8_t turbo_charge_amount; /* unused? */
    /* 0x09D */ int8_t water_block_turns_left;
    /* 0x09E */ uint8_t water_block_amount; /* unused? */
    /* 0x09F */ char unk_9F;
    /* 0x0A0 */ int32_t *unk_A0;
    /* 0x0A4 */ int8_t cloud_nine_turns_left;
    /* 0x0A5 */ uint8_t cloud_nine_dodge_chance; /* = 50% */
    /* 0x0A6 */ char unk_A6[2];
    /* 0x0A8 */ int32_t cloud_nine_effect;
    /* 0x0AC */ char unk_AC[2];
    /* 0x0AE */ uint8_t hammer_loss_turns;
    /* 0x0AF */ uint8_t jump_loss_turns;
    /* 0x0B0 */ uint8_t item_loss_turns;
    /* 0x0B1 */ char unk_B1[3];
    /* 0x0B4 */ void *pre_update_callback;
    /* 0x0B8 */ char unk_B8[4];
    /* 0x0BC */ void *control_script; /* control handed over to this when changing partners */
    /* 0x0C0 */ int32_t control_script_id;
    /* 0x0C4 */ void *cam_movement_script;
    /* 0x0C8 */ int32_t cam_movement_script_id;
    /* 0x0CC */ char unk_CC[12];
    /* 0x0D8 */ actor_t *player_actor;
    /* 0x0DC */ actor_t *partner_actor;
    /* 0x0E0 */ actor_t *enemy_actors[24];
    /* 0x140 */ int16_t enemy_ids[24];
    /* 0x170 */ char unk_170;
    /* 0x171 */ uint8_t num_enemy_actors;
    /* 0x172 */ char unk_172[6];
    /* 0x178 */ int8_t move_category;
    /* 0x179 */ char unk_179;
    /* 0x17A */ int16_t selected_item_id;
    /* 0x17C */ int16_t selected_move_id;
    /* 0x17E */ int16_t current_attack_damage;
    /* 0x180 */ int16_t last_attack_damage;
    /* 0x182 */ char unk_182[2];
    /* 0x184 */ int32_t current_target_list_flags; /* set when creating a target list, also obtain from the flags field of moves */
    /* 0x188 */ int32_t current_attack_element;
    /* 0x18C */ int32_t current_attack_event_suppression;
    /* 0x190 */ int32_t current_attack_status;
    /* 0x194 */ uint8_t status_chance;
    /* 0x195 */ uint8_t status_duration;
    /* 0x196 */ char unk_196;
    /* 0x197 */ uint8_t target_home_index; /* some sort of home idnex used for target list construction */
    /* 0x198 */ uint8_t power_bounce_counter;
    /* 0x199 */ int8_t was_status_inflicted; /* during last attack */
    /* 0x19A */ uint8_t unk_19A;
    /* 0x19B */ char unk_19B[5];
    /* 0x1A0 */ int16_t current_target_id; /* selected? */
    /* 0x1A2 */ uint8_t current_target_part; /* selected? */
    /* 0x1A3 */ char unk_1A3;
    /* 0x1A4 */ int16_t current_target_id2;
    /* 0x1A6 */ uint8_t current_target_part2;
    /* 0x1A7 */ int8_t battle_phase;
    /* 0x1A8 */ int16_t attacker_actor_id;
    /* 0x1AA */ char unk_1AA[4];
    /* 0x1AE */ int16_t submenu_icons[24]; /* icon IDs */
    /* 0x1DE */ uint8_t submenu_moves[24]; /* move IDs */
    /* 0x1F6 */ uint8_t submenu_enabled[24];
    /* 0x20E */ uint8_t submenu_move_count;
    /* 0x20F */ char unk_20F;
    /* 0x210 */ int32_t current_buttons_down;
    /* 0x214 */ int32_t current_buttons_pressed;
    /* 0x218 */ int32_t current_buttons_held;
    /* 0x21C */ int32_t stick_x;
    /* 0x220 */ int32_t stick_y;
    /* 0x224 */ int32_t input_bitmask;
    /* 0x228 */ int32_t dpad_x; /* 0-360 */
    /* 0x22C */ int32_t dpad_y; /* 0-60 */
    /* 0x230 */ int32_t hold_input_buffer[64];
    /* 0x330 */ int32_t push_input_buffer[64];
    /* 0x430 */ uint8_t hold_input_buffer_pos;
    /* 0x431 */ uint8_t input_buffer_pos;
    /* 0x432 */ int8_t unk_432;
    /* 0x433 */ char unk_433;
    /* 0x434 */ int32_t unk_434;
    /* 0x438 */ void *foreground_model_data;
    /* 0x43C */ void *unk_43C;
    /* 0x440 */ uint8_t tattle_flags[27];
    /* 0x45B */ char unk_45B[5];
    /* 0x460 */ int32_t unk_460;
    /* 0x464 */ int32_t unk_464;
    /* 0x468 */ char unk_468[4];
    /* 0x46C */ int32_t battle_state; /* 0 = load assets, 1 = create actors, 4 = start scripts, 7 & 8 = unk */
    /* 0x470 */ int32_t unk_470;
    /* 0x474 */ int32_t unk_474;
    /* 0x478 */ int8_t unk_478;
    /* 0x479 */ char unk_479;
    /* 0x47A */ uint8_t current_battle_section;
    /* 0x47B */ uint8_t unk_47B;
    /* 0x47C */ int32_t unk_47C;
    /* 0x480 */ int32_t unk_480;
    /* 0x484 */ int32_t unk_484;
    /* 0x488 */ int32_t unk_488;
    /* 0x48C */ void *unk_48C;
} battle_status_ctxt_t; // size = 0x490

typedef struct {
    /* 0x00 */ int32_t flags;
    /* 0x04 */ int32_t effectIndex;
    /* 0x08 */ int32_t matrixTotal;
} EffectInstance; // size = 0x20

typedef struct {
    /* 0x00 */ char unk_00[0x44];
    /* 0x44 */ uint16_t mash_bar;
    /* 0x4A */ int16_t unk_4A; // current action command id?
    /* 0x4C */ int16_t unk_4C;
    /* 0x4E */ char unk_4E[0x10];
    /* 0x5E */ int8_t unk_5E;
    /* 0x5F */ int8_t unk_5F;
    /* 0x60 */ int8_t unk_60;
    /* 0x61 */ int8_t unk_61;
    /* 0x62 */ int16_t unk_62;
    /* 0x64 */ int16_t unk_64;
    /* 0x66 */ int16_t unk_66;
    /* 0x68 */ int16_t unk_68;
    /* 0x6A */ int16_t unk_6A;
    /* 0x6C */ int16_t unk_6C;
    /* 0x6E */ int16_t hitsTakenIsMax;
    /* 0x70 */ char unk_70[2];
    /* 0x72 */ char unk_72[2];
    /* 0x74 */ int16_t unk_74;
    /* 0x76 */ int16_t unk_76;
    /* 0x78 */ int16_t unk_78;
    /* 0x7A */ int16_t unk_7A;
    /* 0x7C */ int16_t unk_7C;
    /* 0x7E */ int16_t unk_7E;
    /* 0x80 */ int8_t unk_80;
} ActionCommandState; // size unknown, taken from decomp

typedef struct {
    /* 0x0000 */ uint32_t script_list_ptr;
    /* 0x0004 */ uint32_t unk_04;
    /* 0x0008 */ int32_t script_index_list[128];
    /* 0x0208 */ int32_t script_id_list[128];
    /* 0x0408 */ uint32_t script_list_count;
} script_list_ctxt_t;

typedef struct {
    /* 0x0000 */ int32_t logical_save_info[4][2];
    /* 0x0020 */ int32_t physical_save_info[6][2];
    /* 0x0050 */ int32_t next_available_save_page;
} save_info_ctxt_t;

typedef __OSEventState __osEventStateTab_t[];

/* Data */
#define extern_data extern __attribute__ ((section(".data")))
extern_data status_ctxt_t pm_status;
extern_data uint32_t pm_ViFrames;
extern_data int32_t pm_TimeFreezeMode;
extern_data Gfx* pm_MasterGfxPos;
extern_data save_info_ctxt_t pm_save_info;
extern_data int16_t pm_RoomChangeState;
extern_data uint32_t pm_enemy_defeat_flags[600];
extern_data int32_t pm_RandSeed;
extern_data EffectInstance *pm_effects[96];
extern_data save_data_ctxt_t pm_save_data;
extern_data int32_t pm_battle_state_2;
extern_data battle_status_ctxt_t pm_battle_status;
extern_data int32_t pm_popup_menu_var;
extern_data overworld_ctxt_t pm_overworld;
extern_data hud_ctxt_t pm_hud;
extern_data player_ctxt_t pm_player;
extern_data ActionCommandState pm_ActionCommandState;
extern_data script_list_ctxt_t pm_curr_script_lst;

/* Functions */
void osSyncPrintf(const char *fmt, ...);
void __osPiGetAccess(void);
void __osPiRelAccess(void);
void osCreateMesgQueue(OSMesgQueue *queue, OSMesg *msg, int32_t unk);
int32_t osRecvMesg(OSMesgQueue *queue, OSMesg *msg, int32_t flag);
int32_t pm_FioValidateFileChecksum(void *buffer);
_Bool pm_FioFetchSavedFileInfo(void);
void pm_FioDeserializeState(void);
void pm_SetCurtainScaleGoal(float goal);
void pm_SetCurtainDrawCallback(void *callback);
void pm_SetCurtainFadeGoal(float goal);
void pm_AddBadge(Badge badgeID);
void pm_HidePopupMenu(void);
void pm_DestroyPopupMenu(void);
void pm_SetGameMode(int32_t mode);
void pm_RemoveEffect(EffectInstance *effect);
void pm_FioReadFlash(int32_t slot, void *buffer, uint32_t size);
void pm_FioWriteFlash(int32_t slot, void *buffer, uint32_t size);
void pm_GameUpdate(void);
int32_t pm_SetMapTransitionEffect(int32_t transition);
void pm_PlaySfx(int32_t sound_id);
void pm_BgmSetSong(int32_t player_index, int32_t song_id, int32_t variation, int32_t fade_out_time, int16_t volume);
void pm_SfxStopSound(int32_t sound_id);
void pm_PlayAmbientSounds(int32_t sounds_id, int32_t fade_time);
void pm_SaveGame(void);
void pm_func_802A472C(void);

/* Convenience Values */
#define STORY_PROGRESS pm_save_data.global_bytes[0]

#endif
