#ifndef PM64_H
#define PM64_H
#include <n64.h>
#include <stdint.h>
#include <stdarg.h>
#include "enums.h"

#define JP 0x00
#define US 0x01

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#define ICON_PALETTE_SIZE 32
#if PM64_VERSION == US
#define ICONS_ITEMS_ROM_START      0x1CC310
#define ICONS_PARTNERS_ROM_START   0x97890
#else
#define ICONS_ITEMS_ROM_START      0x1D4720
#define ICONS_PARTNERS_ROM_START   0x97A20
#endif


typedef s32 HudScript[0];

typedef s32 OSPri;
typedef s32 OSId;
typedef union {
    struct {
        f32 f_odd;
        f32 f_even;
    } f;
    f64 d;
} __OSfp;

typedef struct {
    u8 r, g, b;
} Color_RGB8;

typedef struct Vec2b {
    /* 0x00 */ s8 x;
    /* 0x01 */ s8 y;
} Vec2b; // size = 0x02

typedef struct Vec2bu {
    /* 0x00 */ u8 x;
    /* 0x01 */ u8 y;
} Vec2bu; // size = 0x02

typedef struct Vec3b {
    /* 0x00 */ s8 x;
    /* 0x01 */ s8 y;
    /* 0x02 */ s8 z;
} Vec3b; // size = 0x03

typedef struct Vec2s {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
} Vec2s; // size = 0x04

typedef struct Vec2su {
    /* 0x00 */ u16 x;
    /* 0x02 */ u16 y;
} Vec2su; // size = 0x04

typedef struct Vec3s {
    /* 0x00 */ s16 x;
    /* 0x02 */ s16 y;
    /* 0x04 */ s16 z;
} Vec3s; // size = 0x06

typedef struct Vec2i {
    /* 0x00 */ s32 x;
    /* 0x04 */ s32 y;
} Vec2i; // size = 0x08

typedef struct Vec3i {
    /* 0x00 */ s32 x;
    /* 0x04 */ s32 y;
    /* 0x08 */ s32 z;
} Vec3i; // size = 0x0C

typedef struct Vec2f {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
} Vec2f; // size = 0x08

typedef struct Vec2XZf {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 z;
} Vec2XZf; // size = 0x08

typedef struct Vec3f {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
} Vec3f; // size = 0x0C

typedef struct Vec4f {
    /* 0x00 */ f32 x;
    /* 0x04 */ f32 y;
    /* 0x08 */ f32 z;
    /* 0x0C */ f32 yaw;
} Vec4f; // size = 0x10

typedef struct {
    /* 0x0000 */ s8 x_cardinal;
    /* 0x0001 */ s8 y_cardinal;
    union {
        struct {
            u16 a  : 1;
            u16 b  : 1;
            u16 z  : 1;
            u16 s  : 1;
            u16 du : 1;
            u16 dd : 1;
            u16 dl : 1;
            u16 dr : 1;
            u16    : 2;
            u16 l  : 1;
            u16 r  : 1;
            u16 cu : 1;
            u16 cd : 1;
            u16 cl : 1;
            u16 cr : 1;
        };
        /* 0x0002 */ u16 buttons;
    };
} controller_t; // size: 0x0004

typedef struct {
    /* 0x0000 */ controller_t raw; /* raw input */
    /* 0x0004 */ char unk_0x04[0x0C];
    /* 0x0010 */ controller_t pressed; /* one frame when pressed*/
    /* 0x0014 */ char unk_0x14[0x0C];
    /* 0x0020 */ controller_t pad_held; /* once every 4 frames */
    /* 0x0024 */ char unk_0x24[0x0C];
    /* 0x0030 */ controller_t previous;
    /* 0x0034 */ char unk_0x34[0x0C];
    /* 0x0040 */ s8 control_x;
    /* 0x0041 */ char unk_0x41[0x03];
    /* 0x0044 */ s8 control_y;
    /* 0x0045 */ char unk_0x45[0x03];
    /* 0x0048 */ s16 hold_timer_start_value; /* writes 0F to hold_timer when input is pressed*/
    /* 0x004A */ char unk_0x4A[0x02];
    /* 0x004C */ char unk_0x4C[0x0C];
    /* 0x0058 */ s16 hold_timer; /*counts from 0F to 00 to start held variable*/
    /* 0x005A */ char unk_0x5A[0x02];
    /* 0x005C */ char unk_0x5C[0x04];
    /* 0x0060 */ s16 held_timer; /*FFFF when nothing held, when hold_timer hits 0 it will cycle between 0 and 3*/
    /* 0x0062 */ char unk_0x62[0x02];
    /* 0x0064 */ char unk_0x64[0xC];
    /* 0x0070 */ s8 is_battle;
    /* 0x0071 */ s8 demo_flag;          /*1 for demo. 0 in normal gameplay freezes mario*/
    /* 0x0072 */ s8 demo_scene;         /*0-0x12 for each demo scene*/
    /* 0x0073 */ s8 controller_plugged; /*needs to be 1 otherwise "no controller" */
    /* 0x0074 */ u8 battle_debug; /* 0=normal, 1=enemies can't interact, 2=defeat enemies on contact, 3=auto-defeat
                                          enemies in battle, 4=auto run away */
    /* 0x0075 */ u8 quizmo_debug; /* 1 to force a quizmo spawn every time */
    /* 0x0076 */ s8 unk_0x76;
    /* 0x0077 */ char unk_0x77;
    /* 0x0078 */ s8 disable_scripts;
    /* 0x0079 */ char unk_0x79;
    /* 0x007A */ s8 music_enabled;
    /* 0x007B */ char unk_0x7B;
    /* 0x007C */ s8 unk_0x7C;
    /* 0x007D */ s8 unk_0x7D;
    /* 0x007E */ u8 peach_flags;    /*bitfield, 1 = isPeach, 2 = isTransformed, 3 = hasParasol*/
    /* 0x007F */ s8 peach_disguise; /*1 = koopatrol, 2 = hammer bro, 3 = clubba */
    /* 0x0080 */ u8 peach_anim_idx;
    /* 0x0081 */ char unk_0x84[0x05];
    /* 0x0086 */ u16 area_id;
    /* 0x0088 */ u16 area_id_copy; /* used for calculating did_area_change */
    /* 0x008A */ s16 did_area_change;
    /* 0x008C */ u16 map_id;
    /* 0x008E */ u16 entrance_id;
    /* 0x0090 */ u16 unk_0x90;
    /* 0x0092 */ u16 unk_0x92;
    /* 0x0094 */ f32 loading_zone_tangent;
    /* 0x0098 */ Vec3f player_position_copy;
    /* 0x00A4 */ f32 player_angle_copy;
    /* 0x00A8 */ char unk_A8[0x04];
    /* 0x00AC */ s8 load_menu_state;
    /* 0x00AD */ s8 menu_counter;
    /* 0x00AE */ s8 skip_intro;
    /* 0x00AF */ char unk_AF[0x07];
    /* 0x00B6 */ s16 boot_alpha;
    /* 0x00B8 */ s16 boot_blue;
    /* 0x00BA */ s16 boot_green;
    /* 0x00BC */ s16 boot_red;
    /* 0x00BE */ char unk_BE[0x6A];
    /* 0x0128 */ Vec3f player_trace_normal;
    /* 0x0134 */ u16 frame_counter;
    /* 0x0136 */ char unk_136[0x02];
    /* 0x0138 */ s32 next_rng;
    /* 0x013C */ s16 unk_13C;
    /* 0x013E */ char unk_13E[0x0A];
    /* 0x0148 */ s16 enable_background; /* (bit 2 is also used for something) */
    /* 0x014A */ s16 background_min_w;
    /* 0x014C */ s16 background_min_h;
    /* 0x014E */ s16 background_max_w;
    /* 0x0150 */ s16 background_max_h;
    /* 0x0152 */ s16 background_x_offset; /* used for parallax scroll */
    /* 0x0154 */ void *background_raster;
    /* 0x0158 */ void *background_palette;
    /* 0x015C */ s16 unk_15C;
    /* 0x015E */ u16 unk_15E;
    /* 0x0160 */ Vec3s saved_pos;
    /* 0x0166 */ u8 save_slot;
    /* 0x0167 */ u8 load_type; /* 0 = from map, 1 = from main menu */
    /* 0x0168 */ s32 save_count;
} status_ctxt_t;

typedef struct {
    /* 0x0000 */ u8 enable_partner_ability;
    /* 0x0001 */ char unk_0x01[0x02];
    /* 0x0003 */ u8 partner_ability;
    /* 0x0004 */ s16 control_x_overworld;
    /* 0x0006 */ s16 control_y_overworld;
    /* 0x0008 */ controller_t raw_overworld;
    /* 0x000C */ controller_t pressed1_overworld;
    /* 0x0010 */ controller_t pressed2_overworld;
} overworld_ctxt_t; // size: 0x00014

typedef struct {
    /* 0x00000 */ u32 hp_text;
    /* 0x00004 */ u32 hp_icon;
    /* 0x00008 */ u32 fp_text;
    /* 0x0000C */ u32 fp_icon;
    /* 0x00010 */ u32 coin_icon_background;
    /* 0x00014 */ u32 coin_icon;
    /* 0x00018 */ u32 star_points_icon;
    /* 0x0001C */ u32 star_points_glare;
    /* 0x00020 */ u32 unk_icon1;
    /* 0x00024 */ u32 unk_icon2;
    /* 0x00028 */ u32 unk_icon3;
    /* 0x0002C */ u32 unk_icon4;
    /* 0x00030 */ u32 unk_icon5;
    /* 0x00034 */ s16 hud_x;
    /* 0x00036 */ s16 hud_y;
    /* 0x00038 */ u16 hud_timer; /*hud will show up after 0xF0 frames of idle*/
    /* 0x0003A */ u8 hud_hidden; /*0 for shown, 1 for hidden*/
    /* 0x0003B */ char unk_0x3B[0x01];
    /* 0x0003C */ u8 hud_shown; /*1 for shown, 0 for hidden. this flag does not affect menu behavior*/
    /* 0x0003D */ s8 hp_value;
    /* 0x0003E */ s8 fp_value;
    /* 0x0003F */ char unk_0x3F[0x01];
    /* 0x00040 */ u16 coin_value;
    /* 0x00042 */ char unk_0x42[0x02];
    /* 0x00044 */ char unk_0x44[0x04];
    /* 0x00048 */ u16 star_power_value;
    /* 0x0004A */ u8 hp_is_flashing; /*non-zero value will flash*/
    /* 0x0004B */ u8 hp_flash_counter;
    /* 0x0004C */ char unk_0x4C[0x01];
    /* 0x0004D */ u8 fp_is_flashing; /*non-zero value will flash*/
    /* 0x0004E */ u8 fp_flash_counter;
    /* 0x0004F */ char unk_0x4F[0x01];
    /* 0x00050 */ u8 unk_is_flashing; /*think this is star power? cant check rn*/
    /* 0x00051 */ u8 unk_flash_counter;
    /* 0x00052 */ u8 star_points_is_flashing; /*non-zero value will flash*/
    /* 0x00053 */ u8 star_points_flash_counter;
    /* 0x00054 */ u8 coin_is_flashing; /*non-zero value will flash*/
    /* 0x00055 */ u8 coin_flash_counter;
    /* 0x00056 */ char unk_0x56[0x02];
    /* 0x00058 */ char unk_0x58[0x18];
} hud_ctxt_t; // size: 0x00070

typedef struct {
    /* 0x00000 */ u8 in_party;
    /* 0x00001 */ u8 upgrade;
    /* 0x00002 */ char unk_0x02[0x06];
} partner_t; // size: 0x00008

typedef struct {
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

typedef struct {
    /* 0x0000 */ char unk_0x00[0x01];
    /* 0x0001 */ u8 spell_type;
    /* 0x0002 */ u8 casts_remaining;
    /* 0x0003 */ char unk_0x03[0x02];
    /* 0x0005 */ u8 turns_until_spell;
} merlee_t; // size: 0x0006

typedef struct {
    /* 0x0000 */ u8 star_spirits_saved;
    /* 0x0001 */ char unk_0x01[0x01];
    /* 0x0002 */ u8 full_bars_filled;
    /* 0x0003 */ u8 partial_bars_filled;
    /* 0x0004 */ u8 beam_rank; /*1 for star beam, 2 for peach beam*/
    /* 0x0005 */ char unk_0x05[0x01];
} star_power_t; // size: 0x0006

typedef struct {
    /* 0x000 */ u8 boots_upgrade; /*start: 8010F450*/
    /* 0x001 */ u8 hammer_upgrade;
    /* 0x002 */ u8 hp;
    /* 0x003 */ u8 max_hp;
    /* 0x004 */ u8 menu_max_hp; /*gets copied to max_hp when unpausing */
    /* 0x005 */ u8 fp;
    /* 0x006 */ u8 max_fp;
    /* 0x007 */ u8 menu_max_fp; /*gets copied to max_hp when unpausing */
    /* 0x008 */ u8 bp;
    /* 0x009 */ u8 level;
    /* 0x00A */ u8 has_action_command;
    /* 0x00B */ char unk_0x0B[0x01];
    /* 0x00C */ s16 coins;
    /* 0x00E */ u8 fortress_keys;
    /* 0x00F */ u8 star_pieces;
    /* 0x010 */ u8 star_points;
    /* 0x011 */ char unk_0x11[0x01];
    /* 0x012 */ u8 current_partner; /*0x00 - 0x0B*/
    /* 0x013 */ char unk_0x13[0x01];
    /* 0x014 */ char unk_14[8];
    /* 0x01C */ party_t party;
    /* 0x074 */ u16 key_items[32];
    /* 0x0B4 */ u16 badges[128];
    /* 0x1B4 */ u16 items[10];
    /* 0x1C8 */ u16 stored_items[32];
    /* 0x208 */ u16 equipped_badges[64];
    /* 0x288 */ merlee_t merlee;
    /* 0x28E */ star_power_t star_power;
    /* 0x294 */ s16 other_hits_taken;
    /* 0x296 */ s16 unk_296;
    /* 0x298 */ s16 hits_taken;
    /* 0x29A */ s16 hits_blocked;
    /* 0x29C */ s16 player_first_strikes;
    /* 0x29E */ s16 enemy_first_strikes;
    /* 0x2A0 */ s16 power_bounces;
    /* 0x2A2 */ s16 battle_count;
    /* 0x2A4 */ s16 unk_2A4[4];
    /* 0x2AC */ s32 unk_2AC[2];
    /* 0x2B4 */ u32 total_coins_earned;
    /* 0x2B8 */ s16 idle_frame_counter; /* frames with no inputs, overflows every ~36 minutes of idling */
    /* 0x2BA */ char unk_2BA[2];
    /* 0x2BC */ u32 frame_counter; /* increases by 2 per frame */
    /* 0x2C0 */ s16 quizzes_answered;
    /* 0x2C2 */ s16 quizzes_correct;
    /* 0x2C4 */ s32 unk_2C4[24];
    /* 0x324 */ s32 trade_event_start_time;
    /* 0x328 */ s32 unk_328;
    /* 0x32C */ s16 star_pieces_collected;
    /* 0x32E */ s16 jump_game_plays;
    /* 0x330 */ s32 jump_game_total; /* all-time winnings, max = 99999 */
    /* 0x334 */ s16 jump_game_record;
    /* 0x336 */ s16 smash_game_plays;
    /* 0x338 */ s32 smash_game_total; /* all-time winnings, max = 99999 */
    /* 0x33C */ s16 smash_game_record;
    /* 0x33E */ char unk_606[2];
} player_data_t; // size: 0x340

typedef struct {
    /* 0x0000 */ s32 flags; /*third byte related to cutscenes - write 0 to break free*/
    /* 0x0004 */ s32 anim_flags;
    /* 0x0008 */ u16 idle_timer;
    /* 0x000A */ char unk_0x0A[0x02];
    /* 0x000C */ u16 peach_disguise; /*has something to do with peach transforming in ch6*/
    /* 0x000E */ u16 transparency;   /*0xFF00 when spinning*/
    /* 0x0010 */ u16 flash_timer;    /*used when running away*/
    /* 0x0012 */ char unk_0x12[0x02];
    /* 0x0014 */ u16 busy;        /*changed when talking/opening doors/loading zones*/
    /* 0x0016 */ s16 truncated_x; /*used for hazard respawns*/
    /* 0x0018 */ s16 truncated_y;
    /* 0x001A */ s16 truncated_z; /*used for hazard respawns*/
    /* 0x001C */ char unk_0x1C[0x0C];
    /* 0x0028 */ Vec3f position;
    /* 0x0034 */ char unk_0x34[0x08];
    /* 0x003C */ f32 jumped_from_x;
    /* 0x0040 */ f32 jumped_from_z;
    /* 0x0044 */ f32 landed_at_x;
    /* 0x0048 */ f32 landed_at_z;
    /* 0x004C */ f32 jumped_from_y;
    /* 0x0050 */ f32 last_jump_height;
    /* 0x0054 */ f32 speed;
    /* 0x0058 */ f32 walk_speed; /*constant: 0x40000000 = 2.0*/
    /* 0x005C */ f32 run_speed;  /*constant: 0x40800000 = 4.0*/
    /* 0x0060 */ char unk_0x60[0x0C];
    /* 0x006C */ f32 jump_const;     /*used by jumping func to compare if jump_var_1 less than const*/
    /* 0x0070 */ f32 y_speed;        /*related to rise/fall speed*/
    /* 0x0074 */ f32 y_acceleration; /*related to height cap*/
    /* 0x0078 */ f32 y_jerk;         /*related to height cap*/
    /* 0x007C */ f32 y_snap;         /*related to height cap*/
    /* 0x0080 */ f32
        movement_angle; /*locking this makes you move in only that direction regardless of control stick angle*/
    /* 0x0084 */ f32 facing_angle;
    /* 0x0088 */ char unk_0x88[0x08];
    /* 0x0090 */ f32 body_rotation; /*used for turning effect*/
    /* 0x0094 */ char unk_0x94[0x10];
    /* 0x00A4 */ s32 sprite_animation; /* 1st byte: back turned=01 | 4th byte: animations 00-32*/
    /* 0x00A8 */ f32 left_right;       /*0.0=left, 180.0=right*/
    /* 0x00AC */ char unk_0xAC[0x04];
    /* 0x00B0 */ s16 collider_height;
    /* 0x00B2 */ s16 collider_diameter;
    /* 0x00B4 */ u8 action_state;
    /* 0x00B5 */ u8 prev_action_state;
    /* 0x00B6 */ s8 fall_state; /*also used as sleep state in Peach idle action*/
    /* 0x00B7 */ char unk_B7[0x09];
    /* 0x00C0 */ u16 frames_in_air;
    /* 0x00C2 */ char unk_0xC2[0x02];
    /* 0x00C4 */ char unk_0xC4[0x02];
    /* 0x00C6 */ u16 interactable_id; /*only for doors?*/
    /* 0x00C8 */ u32 *talkable_npc;
    /* 0x00CC */ char unk_0xCC[0x08];
    /* 0x00D4 */ f32 spin_variable;
    /* 0x00D8 */ char unk_0xD8[0x04];
    /* 0x00DC */ controller_t raw;
    /* 0x00E0 */ controller_t previous;
    /* 0x00E4 */ controller_t pad_held;
    /* 0x00E8 */ s32 pad_x;
    /* 0x00EC */ s32 pad_y;
    /* 0x00F0 */ controller_t pad_held_list[10];
    /* 0x0118 */ controller_t pad_pressed_list[10];
    /* 0x0140 */ controller_t held_timer_list[10];
    /* 0x0168 */ s32 pad_x_list[10];
    /* 0x0190 */ s32 pad_y_list[10];
    /* 0x01B8 */ u32 timer;
    /* 0x01BC */ char unk_0x1BC[0xCC];
    /* 0x0288 */ u8 spin_cooldown_timer; /*4 frames at the end of spin*/
    /* 0x0289 */ char unk_0x289[0x02];
    /* 0x028B */ u8 spin_timer;
    /* 0x028C */ char unk_0x28C[0x20];
    /* 0x02AC */ f32 spin_speed;
    /* 0x02B0 */ char unk_0x2B0[0x04];
    /* 0x02B4 */ char unk_0x2B4[0x01];
    /* 0x02B5 */ u8 spin_duration;
    /* 0x02B6 */ char unk_0x228[0x02];
    /* 0x02B8 */ char unk_0x2B8[0x10];
    /* 0x02C8 */ player_data_t player_data;
} player_ctxt_t;

typedef struct {
    /* 0x0000 */ char magic_string[16]; /* "Mario Story 006" */
    /* 0x0010 */ s8 padding[32];        /* always zero */
    /* 0x0030 */ s32 crc1;
    /* 0x0034 */ s32 crc2;
    /* 0x0038 */ s32 save_slot;
    /* 0x003C */ s32 save_count;
    /* 0x0040 */ player_data_t player_data;
    /* 0x0380 */ char unk_380[0xE8];
    /* 0x0468 */ s16 area_id;
    /* 0x046A */ s16 map_id;
    /* 0x046C */ s16 entrance_id;
    /* 0x046E */ char unk_46E[2];
    /* 0x0470 */ u32 enemy_defeat_flags[600];
    /* 0x0DD0 */ u32 unk_DD0[120];
    /* 0x0FB0 */ u32 global_flags[64];
    /* 0x10B0 */ s8 global_bytes[512];
    /* 0x12B0 */ u32 area_flags[8];
    /* 0x12D0 */ s8 area_bytes[16];
    /* 0x12E0 */ char unk_12E0[6];
    /* 0x12E6 */ Vec3s position;
    /* 0x12EC */ s32 unk_12EC;
    /* 0x12F0 */ s8 unk_12F0[12]; /* player name starts at 4th char */
    /* 0x12FC */ s32 unk_12FC;
    /* 0x1300 */ s32 unk_1300;
    /* 0x1304 */ char unk_1304[0x7C];
} save_data_ctxt_t;

typedef struct {
    /* 0x00 */ s32 flags;
    /* 0x04 */ s32 *read_pos;
    /* 0x08 */ s32 *start_read_pos;
    /* 0x0C */ s32 *ptr_property_list;
    /* 0x10 */ s32 *image_addr;
    /* 0x14 */ s32 *palette_addr;
    /* 0x18 */ s32 mem_offset;
    /* 0x1C */ s32 *hud_transform;
    /* 0x20 */ f32 unk_20;
    /* 0x24 */ f32 unk_24;
    /* 0x28 */ f32 unk_img_scale[2];
    /* 0x30 */ f32 uniform_scale;
    /* 0x34 */ f32 width_scale_f;  /* X.10 fmt (divide by 1024.0 to get float) */
    /* 0x38 */ f32 height_scale_f; /* X.10 fmt (divide by 1024.0 to get float) */
    /* 0x3C */ s16 render_pos_x;
    /* 0x3E */ s16 render_pos_y;
    /* 0x40 */ u8 screen_pos_offset[2];
    /* 0x42 */ u8 world_pos_offset[3];
    /* 0x45 */ s8 draw_size_preset;
    /* 0x46 */ s8 tile_size_preset;
    /* 0x47 */ u8 update_timer;
    /* 0x48 */ u8 size_x; /* screen size? */
    /* 0x49 */ u8 size_y; /* screen size? */
    /* 0x4A */ u8 opacity;
    /* 0x4B */ s8 tint[3];
    /* 0x4E */ s8 custom_image_size[2];
    /* 0x40 */ s8 custom_draw_size[2];
} hud_element_t; // size = 0x54

typedef struct {
    /* 0x00 */ s32 flags;
    /* 0x04 */ char unk_04;
    /* 0x05 */ u8 type;
    /* 0x06 */ u8 level;
    /* 0x07 */ u8 max_hp;
    /* 0x08 */ s16 part_count;
    /* 0x0A */ char unk_0A[2];
    /* 0x0C */ void **parts_data;
    /* 0x10 */ void *script;
    /* 0x14 */ s32 *status_table;
    /* 0x18 */ u8 escape_chance;
    /* 0x19 */ u8 air_lift_chance;
    /* 0x1A */ u8 spook_chance;
    /* 0x1B */ u8 base_status_chance;
    /* 0x1C */ u8 up_and_away_chance;
    /* 0x1D */ u8 spin_smash_req;
    /* 0x1E */ u8 power_bounce_chance;
    /* 0x1F */ u8 coin_reward;
    /* 0x20 */ s8 size_x;
    /* 0x20 */ s8 size_y;
    /* 0x22 */ s8 hp_bar_offset_x;
    /* 0x23 */ s8 hp_bar_offset_y;
    /* 0x24 */ s8 status_icon_offset_x;
    /* 0x25 */ s8 status_icon_offset_y;
    /* 0x26 */ s8 status_message_offset_x;
    /* 0x27 */ s8 status_message_offset_y;
} actor_desc_t; // size = 0x28

typedef struct {
    /* 0x00 */ Vec3f current_pos;
    /* 0x0C */ Vec3f goal_pos;
    /* 0x18 */ Vec3f unk_18;
    /* 0x24 */ char unk_24[24];
    /* 0x3C */ f32 acceleration;
    /* 0x40 */ f32 speed;
    /* 0x44 */ f32 velocity;
    /* 0x48 */ f32 angle;
    /* 0x4C */ f32 distance;
} actor_movement_t; // size = 0x50;

typedef struct {
    /* 0x00 */ Vec3f current_pos;
    /* 0x0C */ Vec3f goal_pos;
    /* 0x18 */ Vec3f unk_18;
    /* 0x24 */ char unk_24[24];
    /* 0x3C */ f32 acceleration;
    /* 0x40 */ f32 speed;
    /* 0x44 */ f32 velocity;
    /* 0x48 */ f32 angle;
    /* 0x4C */ f32 distance;
    /* 0x50 */ f32 bounce_divisor;
    /* 0x54 */ char unk_54[0x4];
    /* 0x58 */ s32 anim_jump_rise;
    /* 0x5C */ s32 anim_jump_fall;
    /* 0x60 */ s32 anim_jump_land;
} actor_movement_walk_t; // size = 0x64;

typedef struct {
    /* 0x00 */ s16 actor_id;
    /* 0x02 */ s16 part_id; /* sometimes loaded as byte from 0x3 */
    /* 0x04 */ s16 pos[3];
    /* 0x0A */ char unk_0A[7];
    /* 0x11 */ u8 home_col; /* from xpos --> 0-3 */
    /* 0x12 */ u8 home_row; /* from ypos --> 0-3 */
    /* 0x13 */ u8 layer;    /* from zpos? --> 0-1 */
} selectable_target_t;      // size = 0x14

typedef struct {
    /* 0x000 */ s32 flags;
    /* 0x004 */ char unk_04[4];
    /* 0x008 */ actor_desc_t *static_actor_data;
    /* 0x00C */ actor_movement_walk_t walk;
    /* 0x070 */ s16 move_time;
    /* 0x072 */ s16 move_arc_amplitude;
    /* 0x074 */ char unk_74[3];
    /* 0x077 */ u8 jump_part_index;
    /* 0x078 */ char unk_78[16];
    /* 0x088 */ s32 var_table[16];
    /* 0x0C8 */ actor_movement_t fly;
    /* 0x118 */ f32 fly_elapsed;
    /* 0x11C */ char unk_11C[4];
    /* 0x120 */ s16 fly_time;
    /* 0x122 */ s16 fly_arc_amplitude;
    /* 0x124 */ char unk_124[17];
    /* 0x135 */ u8 foot_step_counter;
    /* 0x136 */ u8 actor_type;
    /* 0x137 */ char unk_137;
    /* 0x138 */ Vec3f home_pos;
    /* 0x144 */ Vec3f current_pos;
    /* 0x150 */ s16 head_offset_x;
    /* 0x152 */ s16 head_offset_y;
    /* 0x154 */ s16 head_offset_z;
    /* 0x156 */ s16 health_bar_position_x;
    /* 0x158 */ s16 health_bar_position_y;
    /* 0x15A */ s16 health_bar_position_z;
    /* 0x15C */ Vec3f rotation;
    /* 0x168 */ s16 rotation_pivot_offset_x;
    /* 0x16A */ s16 rotation_pivot_offset_y;
    /* 0x16C */ s16 rotation_pivot_offset_z;
    /* 0x16E */ char unk_16E[2];
    /* 0x170 */ Vec3f scale;
    /* 0x17C */ Vec3f scale_modifier; /* multiplies normal scale factors componentwise */
    /* 0x188 */ f32 scaling_factor;
    /* 0x18C */ f32 yaw;
    /* 0x190 */ u8 size_x;
    /* 0x191 */ u8 size_y;
    /* 0x192 */ s16 actor_id;
    /* 0x194 */ s8 unk_194;
    /* 0x195 */ s8 unk_195;
    /* 0x196 */ s8 unk_196;
    /* 0x197 */ s8 unk_197;
    /* 0x198 */ s8 unk_198;
    /* 0x199 */ s8 unk_199;
    /* 0x19A */ s8 unk_19A;
    /* 0x19B */ char unk_19B[1];
    /* 0x19C */ s32 actor_type_data1[6]; /* 4 = jump sound */
    /* 0x1B4 */ s16 actor_type_data1b[2];
    /* 0x1B8 */ s8 current_hp;
    /* 0x1B9 */ s8 max_hp;
    /* 0x1BA */ char unk_1BA[2];
    /* 0x1BC */ u8 hp_fraction; /* used to render HP bar */
    /* 0x1BD */ char unk_1BD[3];
    /* 0x1C0 */ void *idle_script_source;
    /* 0x1C4 */ void *take_turn_script_source;
    /* 0x1C8 */ void *on_hit_script_source;
    /* 0x1CC */ void *on_turn_chance_script_source;
    /* 0x1D0 */ void *idle_script;
    /* 0x1D4 */ void *take_turn_script;
    /* 0x1D8 */ void *on_hit_script;
    /* 0x1DC */ void *on_turn_change_script;
    /* 0x1E0 */ s32 idle_script_id;
    /* 0x1E4 */ s32 take_turn_id;
    /* 0x1E8 */ s32 on_hit_id;
    /* 0x1EC */ s32 on_turn_change_id;
    /* 0x1F0 */ s8 last_event_type;
    /* 0x1F1 */ u8 turn_priority;
    /* 0x1F2 */ u8 enemy_index; /* actorID = this | 200 */
    /* 0x1F3 */ u8 num_parts;
    /* 0x1F4 */ void *parts_table;
    /* 0x1F8 */ s16 last_damage_taken;
    /* 0x1FA */ s16 hp_change_counter;
    /* 0x1FC */ s16 damage_counter;
    /* 0x1FE */ char unk_1FE[2];
    /* 0x200 */ s32 **unk_200; // Probably a struct but not sure what yet
    /* 0x204 */ char unk_204[3];
    /* 0x207 */ u8 extra_coin_bonus;
    /* 0x208 */ s8 unk_208;
    /* 0x209 */ char unk_209[3];
    /* 0x20C */ u32 *status_table;
    /* 0x210 */ s8 debuff;
    /* 0x211 */ s8 debuff_duration;
    /* 0x212 */ s8 static_status; /* 0B = yes */
    /* 0x213 */ s8 static_duration;
    /* 0x214 */ s8 stone_status; /* 0C = yes */
    /* 0x215 */ s8 stone_duration;
    /* 0x216 */ s8 ko_status; /* 0D = yes */
    /* 0x217 */ s8 ko_duration;
    /* 0x218 */ s8 trans_status; /* 0E = yes */
    /* 0x219 */ s8 trans_duration;
    /* 0x21A */ char unk_21A[2];
    /* 0x21C */ u8 status;
    /* 0x21D */ char unk_21D[3];
    /* 0x220 */ s8 is_glowing;
    /* 0x221 */ u8 attack_boost;
    /* 0x222 */ s8 defense_boost;
    /* 0x223 */ u8 chill_out_amount; /* attack reduction */
    /* 0x224 */ u8 chill_out_turns;
    /* 0x225 */ char unk_225[7];
    /* 0x22C */ selectable_target_t target_data[24];
    /* 0x40C */ s8 target_list_length;
    /* 0x40D */ s8 target_index_list[24]; /* into targetData */
    /* 0x425 */ s8 selected_target_index; /* into target index list */
    /* 0x426 */ s8 target_part_index;
    /* 0x427 */ char unk_427;
    /* 0x428 */ s16 target_actor_id;
    /* 0x42A */ char unk_42A[2];
    /* 0x42C */ void *shadow;     /* might be shadow ID */
    /* 0x430 */ f32 shadow_scale; /* = actor size / 24.0 */
    /* 0x434 */ s16 render_mode;  /* initially 0xD, set to 0x22 if any part is transparent */
    /* 0x436 */ s16 unk_436;
    /* 0x438 */ s32 x[2]; /* ??? see FUN_80253974 */
    /* 0x440 */ hud_element_t *ptr_defuff_icon;
} actor_t; // size = 0x444

typedef struct {
    /* 0x000 */ s32 flags1;
    /* 0x004 */ s32 flags2;
    /* 0x008 */ s32 var_table[16];
    /* 0x048 */ u8 current_submenu;
    /* 0x049 */ char unk_49[3];
    /* 0x04C */ s8 unk_4C;
    /* 0x04D */ s8 unk_4D;
    /* 0x04E */ s8 unk_4E;
    /* 0x04F */ s8 unk_4F;
    /* 0x050 */ s8 unk_50;
    /* 0x051 */ s8 unk_51;
    /* 0x052 */ s8 unk_52;
    /* 0x053 */ s8 strats_last_cursor_pos;
    /* 0x054 */ char unk_54[8];
    /* 0x05C */ s8 unk_5C;
    /* 0x05D */ s8 unk_5D;
    /* 0x05E */ char unk_5E[4];
    /* 0x062 */ s8 unk_62;
    /* 0x063 */ s8 unk_63;
    /* 0x064 */ char unk_64[12];
    /* 0x070 */ s16 unk_70;
    /* 0x072 */ char unk_72[2];
    /* 0x074 */ s32 unk_74;
    /* 0x078 */ u8 total_star_points;
    /* 0x079 */ u8 pending_star_points;        /* how many to add */
    /* 0x07A */ u8 increment_star_point_delay; /* related to star points, set to 0x28 when they are dropped */
    /* 0x07B */ u8 damage_taken;
    /* 0x07C */ u8 change_partner_allowed;
    /* 0x07D */ char unk_7D[4];
    /* 0x081 */ s8 action_success;
    /* 0x082 */ char unk_82;
    /* 0x083 */ s8 unk_83;
    /* 0x084 */ s8 unk_84;
    /* 0x085 */ s8 unk_85;
    /* 0x086 */ s8 unk_86;
    /* 0x087 */ s8 block_result;   /* 0 = fail, 1 = success, -1 = mashed */
    /* 0x088 */ u8 item_uses_left; /* set to 2 for doublke dip, 3 for triple */
    /* 0x089 */ u8 hp_drain_count;
    /* 0x08A */ char unk_8A;
    /* 0x08B */ u8 hustle_turns; /* numTurns from hustle drink, normally 0 */
    /* 0x08C */ char unk_8C;
    /* 0x08D */ s8 unk_8D;
    /* 0x08E */ u8 initial_enemy_count; /* used for SP award bonus */
    /* 0x08F */ char unk_8F[3];
    /* 0x092 */ s8 unk_92;
    /* 0x093 */ char unk_93;
    /* 0x094 */ s8 unk_94;
    /* 0x095 */ char unk_95;
    /* 0x096 */ s8 hammer_charge;
    /* 0x097 */ s8 jump_charge;
    /* 0x098 */ char unk_98;
    /* 0x099 */ u8 danger_flags; /* 1 = danger, 2 = peril */
    /* 0x09A */ s8 outta_sight_active;
    /* 0x09B */ s8 turbo_charge_turns_left;
    /* 0x09C */ u8 turbo_charge_amount; /* unused? */
    /* 0x09D */ s8 water_block_turns_left;
    /* 0x09E */ u8 water_block_amount; /* unused? */
    /* 0x09F */ char unk_9F;
    /* 0x0A0 */ s32 *unk_A0;
    /* 0x0A4 */ s8 cloud_nine_turns_left;
    /* 0x0A5 */ u8 cloud_nine_dodge_chance; /* = 50% */
    /* 0x0A6 */ char unk_A6[2];
    /* 0x0A8 */ s32 cloud_nine_effect;
    /* 0x0AC */ char unk_AC[2];
    /* 0x0AE */ u8 hammer_loss_turns;
    /* 0x0AF */ u8 jump_loss_turns;
    /* 0x0B0 */ u8 item_loss_turns;
    /* 0x0B1 */ char unk_B1[3];
    /* 0x0B4 */ void *pre_update_callback;
    /* 0x0B8 */ char unk_B8[4];
    /* 0x0BC */ void *control_script; /* control handed over to this when changing partners */
    /* 0x0C0 */ s32 control_script_id;
    /* 0x0C4 */ void *cam_movement_script;
    /* 0x0C8 */ s32 cam_movement_script_id;
    /* 0x0CC */ char unk_CC[12];
    /* 0x0D8 */ actor_t *player_actor;
    /* 0x0DC */ actor_t *partner_actor;
    /* 0x0E0 */ actor_t *enemy_actors[24];
    /* 0x140 */ s16 enemy_ids[24];
    /* 0x170 */ char unk_170;
    /* 0x171 */ u8 num_enemy_actors;
    /* 0x172 */ char unk_172[6];
    /* 0x178 */ s8 move_category;
    /* 0x179 */ char unk_179;
    /* 0x17A */ s16 selected_item_id;
    /* 0x17C */ s16 selected_move_id;
    /* 0x17E */ s16 current_attack_damage;
    /* 0x180 */ s16 last_attack_damage;
    /* 0x182 */ char unk_182[2];
    /* 0x184 */ s32
        current_target_list_flags; /* set when creating a target list, also obtain from the flags field of moves */
    /* 0x188 */ s32 current_attack_element;
    /* 0x18C */ s32 current_attack_event_suppression;
    /* 0x190 */ s32 current_attack_status;
    /* 0x194 */ u8 status_chance;
    /* 0x195 */ u8 status_duration;
    /* 0x196 */ char unk_196;
    /* 0x197 */ u8 target_home_index; /* some sort of home idnex used for target list construction */
    /* 0x198 */ u8 power_bounce_counter;
    /* 0x199 */ s8 was_status_inflicted; /* during last attack */
    /* 0x19A */ u8 unk_19A;
    /* 0x19B */ char unk_19B[5];
    /* 0x1A0 */ s16 current_target_id;  /* selected? */
    /* 0x1A2 */ u8 current_target_part; /* selected? */
    /* 0x1A3 */ char unk_1A3;
    /* 0x1A4 */ s16 current_target_id2;
    /* 0x1A6 */ u8 current_target_part2;
    /* 0x1A7 */ s8 battle_phase;
    /* 0x1A8 */ s16 attacker_actor_id;
    /* 0x1AA */ char unk_1AA[4];
    /* 0x1AE */ s16 submenu_icons[24]; /* icon IDs */
    /* 0x1DE */ u8 submenu_moves[24];  /* move IDs */
    /* 0x1F6 */ u8 submenu_enabled[24];
    /* 0x20E */ u8 submenu_move_count;
    /* 0x20F */ char unk_20F;
    /* 0x210 */ s32 current_buttons_down;
    /* 0x214 */ s32 current_buttons_pressed;
    /* 0x218 */ s32 current_buttons_held;
    /* 0x21C */ s32 stick_x;
    /* 0x220 */ s32 stick_y;
    /* 0x224 */ s32 input_bitmask;
    /* 0x228 */ s32 dpad_x; /* 0-360 */
    /* 0x22C */ s32 dpad_y; /* 0-60 */
    /* 0x230 */ s32 hold_input_buffer[64];
    /* 0x330 */ s32 push_input_buffer[64];
    /* 0x430 */ u8 hold_input_buffer_pos;
    /* 0x431 */ u8 input_buffer_pos;
    /* 0x432 */ s8 unk_432;
    /* 0x433 */ char unk_433;
    /* 0x434 */ s32 *unk_434;
    /* 0x438 */ void *foreground_model_data;
    /* 0x43C */ void *unk_43C;
    /* 0x440 */ u8 tattle_flags[27];
    /* 0x45B */ char unk_45B[5];
    /* 0x460 */ s32 unk_460;
    /* 0x464 */ s32 unk_464;
    /* 0x468 */ char unk_468[4];
    /* 0x46C */ s32 battle_state; /* 0 = load assets, 1 = create actors, 4 = start scripts, 7 & 8 = unk */
    /* 0x470 */ s32 unk_470;
    /* 0x474 */ s32 unk_474;
    /* 0x478 */ s8 unk_478;
    /* 0x479 */ char unk_479;
    /* 0x47A */ u8 current_battle_section;
    /* 0x47B */ u8 unk_47B;
    /* 0x47C */ s32 unk_47C;
    /* 0x480 */ s32 unk_480;
    /* 0x484 */ s32 unk_484;
    /* 0x488 */ s32 unk_488;
    /* 0x48C */ void *unk_48C;
} battle_status_ctxt_t; // size = 0x490

typedef struct {
    /* 0x000 */ s32 flags;
    /* 0x004 */ s8 first_strike; /* 0 = none, 1 = player, 2 = enemy */
    /* 0x005 */ s8 hit_type;     /* 1 = none/enemy, 2 = jump */
    /* 0x006 */ s8 hit_tier;     /* 0 = normal, 1 = super, 2 = ultra */
    /* 0x007 */ char unk_07;
    /* 0x008 */ s8 unk_08;
    /* 0x009 */ s8 battle_outcome; /* 0 = won, 1 = lost */
    /* 0x00A */ s8 unk_0A;
    /* 0x00B */ s8 merlee_coin_bonus; /* triple coins when != 0 */
    /* 0x00C */ u8 damage_taken;      /* valid after battle */
    /* 0x00D */ char unk_0D;
    /* 0x00E */ s16 coins_earned; /* valid after battle */
    /* 0x010 */ char unk_10;
    /* 0x011 */ u8 allow_fleeing;
    /* 0x012 */ s8 unk_12;
    /* 0x013 */ u8 drop_whacka_bump;
    /* 0x014 */ s32 song_id;
    /* 0x018 */ s32 unk_18;
    /* 0x01C */ s8 num_encounters; /* number of encounters for current map (in list) */
    /* 0x01D */ s8 current_area_index;
    /* 0x01E */ u8 current_map_index;
    /* 0x01F */ u8 current_entry_index;
    /* 0x020 */ s8 map_id;
    /* 0x021 */ s8 reset_map_encounter_flags;
    /* 0x022 */ char unk_22[2];
    /* 0x024 */ s32 *npc_group_list;
    /* 0x028 */ s32 *unk_28[24];    /* struct Encounter* encounter_list[24]; */
    /* 0x088 */ s32 *unk_88;        /* struct Encounter* current_encounter; */
    /* 0x08C */ s32 *current_enemy; /* struct Enemy* current_enemy; */
    /* 0x090 */ s32 fade_out_amount;
    /* 0x094 */ s32 unk_94;
    /* 0x098 */ s32 fade_out_accel;
    /* 0x09C */ s32 battle_start_countdown;
    /* 0x0A0 */ char unk_A0[16];
    /* 0x0B0 */ s32 defeat_flags[60][12];
    /* 0xFB0 */ s16 recent_maps[2];
    /* 0xFB4 */ char unk_FB4[4];
} encounter_status_ctxt_t; // size = 0xFB8

typedef struct Camera {
    /* 0x000 */ u16 flags;
    /* 0x002 */ s16 moveFlags;
    /* 0x004 */ s16 updateMode;
    /* 0x006 */ s16 unk_06;
    /* 0x008 */ s16 changingMap;
    /* 0x00A */ s16 viewportW;
    /* 0x00C */ s16 viewportH;
    /* 0x00E */ s16 viewportStartX;
    /* 0x010 */ s16 viewportStartY;
    /* 0x012 */ s16 nearClip;
    /* 0x014 */ s16 farClip;
    /* 0x016 */ char unk_16[2];
    /* 0x018 */ f32 vfov;
    /* 0x01C */ s16 unk_1C;
    /* 0x01E */ s16 unk_1E;
    /* 0x020 */ s16 unk_20;
    /* 0x022 */ s16 unk_22;
    /* 0x024 */ s16 unk_24;
    /* 0x026 */ s16 unk_26;
    /* 0x028 */ s16 unk_28;
    /* 0x02A */ s16 zoomPercent;
    /* 0x02C */ s16 bgColor[3];
    /* 0x032 */ Vec3s targetScreenCoords;
    /* 0x038 */ u16 perspNorm;
    /* 0x03A */ char unk_3A[2];
    /* 0x03C */ Vec3f lookAt_eye;
    /* 0x048 */ Vec3f lookAt_obj;
    /* 0x054 */ f32 unk_54;
    /* 0x058 */ f32 unk_58;
    /* 0x05C */ f32 unk_5C;
    /* 0x060 */ Vec3f targetPos;
    /* 0x06C */ f32 currentYaw;
    /* 0x070 */ f32 unk_70;
    /* 0x074 */ f32 currentBoomYaw;
    /* 0x078 */ f32 currentBoomLength;
    /* 0x07C */ f32 currentYOffset;
    /* 0x080 */ char unk_80[4];
    /* 0x084 */ Vec3f trueRotation;
    /* 0x090 */ f32 currentBlendedYawNegated;
    /* 0x094 */ f32 currentPitch;
} Camera; // size = 0x558

typedef struct {
    /* 0x00 */ s32 flags;
    /* 0x04 */ s32 effectIndex;
    /* 0x08 */ s32 matrixTotal;
} EffectInstance; // size = 0x20

typedef struct {
    /* 0x00 */ s32 unk_00;
    /* 0x04 */ s32 hudElements[15];
    /* 0x40 */ char unk_40[0x4];
    /* 0x44 */ s16 barFillLevel;
    /* 0x46 */ s16 unk_46;
    /* 0x48 */ s16 unk_48;
    /* 0x4A */ s16 actionCommandID; // current action command id?
    /* 0x4C */ s16 state;
    /* 0x4E */ s16 unk_4E;
    /* 0x50 */ s16 unk_50;
    /* 0x52 */ s16 unk_52;
    /* 0x54 */ s16 unk_54;
    /* 0x56 */ s16 hudElementX;
    /* 0x58 */ s16 hudElementY;
    /* 0x5A */ s16 unk_5A;
    /* 0x5C */ s8 unk_5C;
    /* 0x5D */ s8 unk_5D;
    /* 0x5E */ s8 autoSucceed;
    /* 0x5F */ s8 unk_5F;
    /* 0x60 */ s8 unk_60;
    /* 0x61 */ s8 unk_61;
    /* 0x62 */ s8 unk_62;
    /* 0x63 */ s8 unk_63;
    /* 0x64 */ s16 unk_64;
    /* 0x66 */ s16 unk_66;
    /* 0x68 */ s16 unk_68;
    /* 0x6A */ s16 unk_6A;
    /* 0x6C */ s16 unk_6C;
    /* 0x6E */ s16 hitsTakenIsMax;
    /* 0x70 */ s16 unk_70;
    /* 0x72 */ s16 unk_72;
    /* 0x74 */ s16 mashMeterCutoffs[6]; // upper bounds for each interval
    /* 0x80 */ s8 mashMeterIntervals;
} ActionCommandStatus; // size = 0x84

typedef struct {
    /* 0x0000 */ u32 script_list_ptr;
    /* 0x0004 */ u32 unk_04;
    /* 0x0008 */ s32 script_index_list[128];
    /* 0x0208 */ s32 script_id_list[128];
    /* 0x0408 */ u32 script_list_count;
} script_list_ctxt_t;

typedef struct {
    /* 0x0000 */ s32 logical_save_info[4][2];
    /* 0x0020 */ s32 physical_save_info[6][2];
    /* 0x0050 */ s32 next_available_save_page;
} save_info_ctxt_t;

typedef struct ItemData {
    /* 0x00 */ s32 nameMsg;
    /* 0x04 */ s16 hudElemID;
    /* 0x06 */ s16 sortValue;
    /* 0x08 */ s32 targetFlags;
    /* 0x0C */ s16 sellValue;
    /* 0x0E */ char unk_0E[2];
    /* 0x10 */ s32 fullDescMsg;
    /* 0x14 */ s32 shortDescMsg;
    /* 0x18 */ s16 typeFlags;
    /* 0x1A */ u8 moveID;
    /* 0x1B */ s8 potencyA;
    /* 0x1C */ s8 potencyB;
    /* 0x1D */ char unk_1D[3];
} ItemData; // size = 0x20

typedef struct VtxRect {
    Vtx vtx[4];
} VtxRect; // size = 0x40

typedef struct HudTransform {
    /* 0x00 */ s32 foldIdx;
    /* 0x04 */ Vec3f position;
    /* 0x10 */ Vec3f rotation;
    /* 0x1C */ Vec3f scale;
    /* 0x28 */ Vec2s pivot;
    /* 0x30 */ VtxRect unk_30[3];
} HudTransform; // size = 0xF0

typedef struct HudElement {
    /* 0x00 */ u32 flags;
    /* 0x04 */ HudScript *readPos;
    /* 0x08 */ HudScript *anim;
    /* 0x0C */ HudScript *loopStartPos;
    /* 0x10 */ u8 *rasterAddr;
    /* 0x14 */ u8 *paletteAddr;
    /* 0x18 */ s32 memOffset;
    /* 0x1C */ HudTransform *hudTransform;
    /* 0x20 */ f32 deltaSizeX;
    /* 0x24 */ f32 deltaSizeY;
    /* 0x28 */ f32 unkImgScale[2];
    /* 0x30 */ f32 uniformScale;
    /* 0x34 */ s32 widthScale;  ///< X10
    /* 0x38 */ s32 heightScale; ///< X10
    /* 0x3C */ s16 renderPosX;
    /* 0x3E */ s16 renderPosY;
    /* 0x40 */ Vec2b screenPosOffset;
    /* 0x42 */ Vec3b worldPosOffset;
    /* 0x45 */ s8 drawSizePreset;
    /* 0x46 */ s8 tileSizePreset;
    /* 0x47 */ s8 updateTimer;
    /* 0x48 */ u8 sizeX; /* screen size? */
    /* 0x49 */ u8 sizeY; /* screen size? */
    /* 0x4A */ u8 opacity;
    /* 0x4B */ Color_RGB8 tint;
    /* 0x4E */ Vec2bu customImageSize;
    /* 0x50 */ Vec2bu customDrawSize;
} HudElement; // size = 0x54

typedef struct HudElementSize {
    s16 width;
    s16 height;
    s16 size;
} HudElementSize;

typedef struct IconHudScriptPair {
    /* 0x00 */ HudScript *enabled;
    /* 0x04 */ HudScript *disabled;
} IconHudScriptPair; // size = 0x08

typedef __OSEventState __osEventStateTab_t[];
typedef void *(*PrintCallback)(void *, const char *, u32);

/* Data */
#define extern_data extern __attribute__((section(".data")))
extern_data status_ctxt_t pm_status;
extern_data u32 pm_ViFrames;
extern_data s32 pm_TimeFreezeMode;
extern_data Gfx *pm_MasterGfxPos;
extern_data save_info_ctxt_t pm_save_info;
extern_data s16 pm_MapChangeState;
extern_data u32 pm_enemy_defeat_flags[600];
extern_data ItemData gItemTable[0x16C];
extern_data IconHudScriptPair gItemHudScripts[337];
extern_data HudElementSize gHudElementSizes[26];
extern_data s32 pm_RandSeed;
extern_data EffectInstance *pm_effects[96];
extern_data save_data_ctxt_t pm_save_data;
extern_data s32 pm_battle_state;
extern_data s32 pm_battle_state_2;
extern_data battle_status_ctxt_t pm_battle_status;
extern_data s32 pm_popup_menu_var;
extern_data overworld_ctxt_t pm_overworld;
extern_data hud_ctxt_t pm_hud;
extern_data player_ctxt_t pm_player;
extern_data ActionCommandStatus pm_ActionCommandStatus;
extern_data script_list_ctxt_t pm_curr_script_lst;
extern_data encounter_status_ctxt_t pm_encounter_status;
extern_data Camera pm_gCameras[4];
extern_data s32 pm_gCurrentCameraID;
extern_data s16 pm_GameMode;
extern_data s8 D_800A0900;
extern_data u16 *nuGfxCfb_ptr;
extern_data u32 osMemSize;
extern_data s32 pm_GameState;

/* Functions */
void osSyncPrintf(const char *fmt, ...);
void __osPiGetAccess(void);
void __osPiRelAccess(void);
void osCreateMesgQueue(OSMesgQueue *queue, OSMesg *msg, s32 unk);
s32 osRecvMesg(OSMesgQueue *queue, OSMesg *msg, s32 flag);
s32 dma_copy(u32 romStart, u32 romEnd, void *vramDest);
s32 pm_FioValidateFileChecksum(void *buffer);
_Bool pm_FioFetchSavedFileInfo(void);
void pm_FioDeserializeState(void);
void pm_SetCurtainScaleGoal(f32 goal);
void pm_SetCurtainDrawCallback(void *callback);
void pm_SetCurtainFadeGoal(f32 goal);
void pm_AddBadge(s32 badgeID);
void pm_HidePopupMenu(void);
void pm_DestroyPopupMenu(void);
void pm_SetGameMode(s32 mode);
void pm_RemoveEffect(EffectInstance *effect);
void nuPiReadRom(u32 rom_addr, void *buf_ptr, u32 size);
void pm_FioReadFlash(s32 slot, void *buffer, u32 size);
void pm_FioWriteFlash(s32 slot, void *buffer, u32 size);
s32 draw_ci_image_with_clipping(u8 *texture, s32 width, s32 height, s32 fmt, s32 size, u16 *palette, s16 posX, s16 posY,
                                u16 clipULx, u16 clipULy, u16 clipLRx, u16 clipRLy, u8 opacity);
s32 pm_SetMapTransitionEffect(s32 transition);
void pm_PlaySfx(s32 sound_id);
void pm_BgmSetSong(s32 player_index, s32 song_id, s32 variation, s32 fade_out_time, s16 volume);
void pm_SfxStopSound(s32 sound_id);
void pm_SaveGame(void);
void pm_func_802A472C(void);
void osSetTime(u64);
u64 osGetTime();
s32 _Printf(PrintCallback pfn, void *arg, const char *fmt, va_list ap);
void osWritebackDCacheAll(void);
void osViBlack(u8);
void osViRepeatLine(u8);
void osViSwapBuffer(void *);
void osSetEventMesg(OSEvent, OSMesgQueue *, OSMesg);
void osStopThread(OSThread *);
OSThread *osGetActiveQueue(void);
void state_render_frontUI(void);
void step_game_loop(void);
void pm_disable_player_input(void);
void update_camera_mode_6(Camera *camera);
void update_player_input(void);
s32 pm_is_ability_active(s32 arg0);
void set_screen_overlay_alpha(s32, f32);
void pm_state_step_end_battle(void);
s32 pm_func_800554A4(s32);

/* Convenience Values */
#define STORY_PROGRESS pm_save_data.global_bytes[0]

#endif
