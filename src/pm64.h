#ifndef _PM64_H
#define _PM64_H
#include <n64.h>
#include <stdint.h>

#define PM64_SCREEN_WIDTH    320
#define PM64_SCREEN_HEIGHT   240

typedef struct{
    float           x;                          /* 0x0000 */
    float           y;                          /* 0x0004 */
    float           z;                          /* 0x0008 */
                                                /* size: 0x000C */
}xyz_t;

typedef struct {
    int16_t         x;
    int16_t         y;
    int16_t         z;
}vec3s_t;

typedef struct{
    int8_t          x_cardinal;                 /* 0x0000 */
    int8_t          y_cardinal;                 /* 0x0001 */
    union
    {
        struct
        {
            uint16_t    a       : 1;
            uint16_t    b       : 1;
            uint16_t    z       : 1;
            uint16_t    s       : 1;
            uint16_t    du      : 1;
            uint16_t    dd      : 1;
            uint16_t    dl      : 1;
            uint16_t    dr      : 1;
            uint16_t            : 2;
            uint16_t    l       : 1;
            uint16_t    r       : 1;
            uint16_t    cu      : 1;
            uint16_t    cd      : 1;
            uint16_t    cl      : 1;
            uint16_t    cr      : 1;
      };
      uint16_t          buttons;                /* 0x0003 */
                                                /* size: 0x0004 */        
    };
}controller_t;

typedef struct{
    controller_t    raw;                        /* 0x0000 */ /* raw input */
    char            unk_0x04[0x0C];             /* 0x0004 */            
    controller_t    pressed;                    /* 0x0010 */ /* one frame when pressed*/
    char            unk_0x14[0x0C];             /* 0x0014 */ 
    controller_t    pad_held;                   /* 0x0020 */ /* once every 4 frames */
    char            unk_0x24[0x0C];             /* 0x0024 */
    controller_t    previous;                   /* 0x0030 */
    char            unk_0x34[0x0C];             /* 0x0034 */
    int8_t          control_x;                  /* 0x0040 */
    char            unk_0x41[0x03];             /* 0x0041 */
    int8_t          control_y;                  /* 0x0044 */
    char            unk_0x45[0x03];             /* 0x0045 */
    int16_t         hold_timer_start_value;     /* 0x0048 */ /* writes 0F to hold_timer when input is pressed*/
    char            unk_0x4A[0x02];             /* 0x004A */
    char            unk_0x4C[0x0C];             /* 0x004C */
    int16_t         hold_timer;                 /* 0x0058 */  /*counts from 0F to 00 to start held variable*/
    char            unk_0x5A[0x02];             /* 0x005A */            
    char            unk_0x5C[0x04];             /* 0x005C */            
    int16_t         held_timer;                 /* 0x0060 */ /*FFFF when nothing held, when hold_timer hits 0 it will cycle between 0 and 3*/
    char            unk_0x62[0x02];             /* 0x0062 */ 
    char            unk_0x64[0xC];              /* 0x0064 */
    int8_t          is_battle;                  /* 0x0070 */
    int8_t          demo_flag;                  /* 0x0071 */ /*1 for demo. 0 in normal gameplay freezes mario*/
    int8_t          demo_scene;                 /* 0x0072 */ /*0-0x12 for each demo scene*/
    int8_t          controller_plugged;         /* 0x0073 */ /*needs to be 1 otherwise "no controller" */
    uint8_t         battle_debug;               /* 0x0074 */ /* 0=normal, 1=enemies can't interact, 2=defeat enemies on contact, 3=auto-defeat enemies in battle, 4=auto run away */
    uint8_t         quizmo_debug;               /* 0x0075 */ /* 1 to force a quizmo spawn every time */
    int8_t          unk_0x76;                   /* 0x0076 */
    char            unk_0x77;                   /* 0x0077 */
    int8_t          disable_scripts;            /* 0x0078 */
    char            unk_0x79;                   /* 0x0079 */
    int8_t          music_enabled;              /* 0x007A */
    char            unk_0x7B;                   /* 0x007B */
    int8_t          unk_0x7C;                   /* 0x007C */
    int8_t          unk_0x7D;                   /* 0x007D */
    uint8_t         peach_flags;                /* 0x007E */ /*bitfield, 1 = isPeach, 2 = isTransformed, 3 = hasUmbrella*/
    int8_t          peach_disguise;             /* 0x007F */ /*1 = koopatrol, 2 = hammer bro, 3 = clubba */
    uint8_t         peach_anim_idx;             /* 0x0080 */
    char            unk_0x84[0x05];             /* 0x0081 */
    uint16_t        group_id;                   /* 0x0086 */
    uint16_t        group_id_copy;              /* 0x0088 */ /* used for calculating did_area_change */
    int16_t         did_area_change;            /* 0x008A */
    uint16_t        room_id;                    /* 0x008C */
    uint16_t        entrance_id;                /* 0x008E */
    uint16_t        unk_0x90;                   /* 0x0090 */
    uint16_t        unk_0x92;                   /* 0x0092 */
    float           loading_zone_tangent;       /* 0x0094 */
    xyz_t           player_position_copy;       /* 0x0098 */
    float           player_angle_copy;          /* 0x00A4 */
    char            unk_A8[0x04];               /* 0x00A8 */
    int8_t          load_menu_state;            /* 0x00AC */
    int8_t          menu_counter;               /* 0x00AD */
    int8_t          skip_intro;                 /* 0x00AE */
    char            unk_AF[0x07];               /* 0x00AF */
    int16_t         boot_alpha;                 /* 0x00B6 */
    int16_t         boot_blue;                  /* 0x00B8 */
    int16_t         boot_green;                 /* 0x00BA */
    int16_t         boot_red;                   /* 0x00BC */
    char            unk_BE[0x6A];               /* 0x00BE */
    xyz_t           player_trace_normal;        /* 0x0128 */
    uint16_t        frame_counter;              /* 0x0134 */
    char            unk_136[0x02];              /* 0x0136 */
    int32_t         next_rng;                   /* 0x0138 */
    int16_t         unk_13C;                    /* 0x013C */
    char            unk_13E[0x0A];              /* 0x013E */
    int16_t         enable_background;          /* 0x0148 */ /* (bit 2 is also used for something) */
    int16_t         background_min_w;           /* 0x014A */
    int16_t         background_min_h;           /* 0x014C */
    int16_t         background_max_w;           /* 0x014E */
    int16_t         background_max_h;           /* 0x0150 */
    int16_t         background_x_offset;        /* 0x0152 */ /* used for parallax scroll */
    void            *background_raster;         /* 0x0154 */
    void            *background_palette;        /* 0x0158 */
    int16_t         unk_15C;                    /* 0x015C */
    uint16_t        unk_15E;                    /* 0x015E */
    vec3s_t         saved_pos;                  /* 0x0160 */
    uint8_t         save_slot;                  /* 0x0166 */
    uint8_t         load_type;                  /* 0x0167 */ /* 0 = from map, 1 = from main menu */
    int32_t         save_count;                 /* 0x0168 */

}status_ctxt_t;

typedef struct{
    char            unk_0x00[0x10];             /* 0x0000 */
    uint32_t        saveblock_freeze;           /* 0x0010 */ /*00000001 = frozen, loading zones dissapear. can be used for lzs*/
}unk1_ctxt_t;

typedef struct {
    char            unk_0x00[0x230];            /* 0x00000 */
    Gfx             buf[0x2000];                /* 0x00230 */
    Gfx             buf_work[0x200];            /* 0x10230 */
    Mtx             unk_mtx[0x200];             /* 0x11230 */
                                                /* size: 0x19230 */
}disp_buf_t;

typedef struct{
    Gfx             *p;                         /* 0x00000 */
    uint32_t        unk;                        /* 0x00004 */
    disp_buf_t      *disp_buf;                  /* 0x00008 */
                                                /* size: 0x0000C */
}gfx_ctxt_t;

typedef struct{
    char            unk_0x00[0x84];             /* 0x0000 */
    uint16_t        room_change_state;          /* 0x0084 */
}unk2_ctxt_t;

typedef struct{
    uint32_t        global_flags[64];           /* 0x0000 */
    int8_t          global_bytes[512];          /* 0x0100 */
    uint32_t        area_flags[8];
}flags_ctxt_t;

typedef struct{
    int8_t          story_progress;             /* 0x0000 */
}unk3_ctxt_t;

typedef struct{
    char            unk_0x00[0x03];             /* 0x0000 */
    uint8_t         partner_ability;            /* 0x0003 */
    int16_t         control_x_overworld;        /* 0x0004 */
    int16_t         control_y_overworld;        /* 0x0006 */
    controller_t    raw_overworld;              /* 0x0008 */
    controller_t    pressed1_overworld;         /* 0x000C */
    controller_t    pressed2_overworld;         /* 0x0010 */
                                                /* size: 0x00014 */
}overworld_ctxt_t;

typedef struct{
    uint32_t        hp_text;                    /* 0x00000 */
    uint32_t        hp_icon;                    /* 0x00004 */
    uint32_t        fp_text;                    /* 0x00008 */
    uint32_t        fp_icon;                    /* 0x0000C */
    uint32_t        coin_icon_background;       /* 0x00010 */
    uint32_t        coin_icon;                  /* 0x00014 */
    uint32_t        star_points_icon;           /* 0x00018 */
    uint32_t        star_points_glare;          /* 0x0001C */
    uint32_t        unk_icon1;                  /* 0x00020 */
    uint32_t        unk_icon2;                  /* 0x00024 */
    uint32_t        unk_icon3;                  /* 0x00028 */
    uint32_t        unk_icon4;                  /* 0x0002C */
    uint32_t        unk_icon5;                  /* 0x00030 */
    int16_t         hud_x;                      /* 0x00034 */
    int16_t         hud_y;                      /* 0x00036 */
    uint16_t        hud_timer;                  /* 0x00038 */ /*hud will show up after 0xF0 frames of idle*/
    uint8_t         hud_hidden;                 /* 0x0003A */ /*0 for shown, 1 for hidden*/
    char            unk_0x3B[0x01];             /* 0x0003B */
    uint8_t         hud_shown;                  /* 0x0003C */ /*1 for shown, 0 for hidden. this flag does not affect menu behavior*/
    int8_t          hp_value;                   /* 0x0003D */
    int8_t          fp_value;                   /* 0x0003E */
    char            unk_0x3F[0x01];             /* 0x0003F */
    uint16_t        coin_value;                 /* 0x00040 */
    char            unk_0x42[0x02];             /* 0x00042 */
    char            unk_0x44[0x04];             /* 0x00044 */
    uint16_t        star_power_value;           /* 0x00048 */
    uint8_t         hp_is_flashing;             /* 0x0004A */ /*non-zero value will flash*/
    uint8_t         hp_flash_counter;           /* 0x0004B */
    char            unk_0x4C[0x01];             /* 0x0004C */
    uint8_t         fp_is_flashing;             /* 0x0004D */ /*non-zero value will flash*/
    uint8_t         fp_flash_counter;           /* 0x0004E */
    char            unk_0x4F[0x01];             /* 0x0004F */
    uint8_t         unk_is_flashing;            /* 0x00050 */ /*think this is star power? cant check rn*/
    uint8_t         unk_flash_counter;          /* 0x00051 */
    uint8_t         star_points_is_flashing;    /* 0x00052 */ /*non-zero value will flash*/
    uint8_t         star_points_flash_counter;  /* 0x00053 */
    uint8_t         coin_is_flashing;           /* 0x00054 */ /*non-zero value will flash*/
    uint8_t         coin_flash_counter;         /* 0x00055 */
    char            unk_0x56[0x02];             /* 0x00056 */
    char            unk_0x58[0x18];             /* 0x00058 */
                                                /* size: 0x00070 */
}hud_ctxt_t;

typedef struct{
    uint8_t         boots_upgrade;              /* 0x0000 */ /*start: 8010F450*/
    uint8_t         hammer_upgrade;             /* 0x0001 */
    uint8_t         hp;                         /* 0x0002 */
    uint8_t         max_hp;                     /* 0x0003 */
    uint8_t         menu_max_hp;                /* 0x0004 */ /*gets copied to max_hp when unpausing */
    uint8_t         fp;                         /* 0x0005 */
    uint8_t         max_fp;                     /* 0x0006 */
    uint8_t         menu_max_fp;                /* 0x0007 */ /*gets copied to max_hp when unpausing */
    uint8_t         bp;                         /* 0x0008 */
    uint8_t         level;                      /* 0x0009 */
    uint8_t         has_action_command;         /* 0x000A */
    char            unk_0x0B[0x01];             /* 0x000B */
    int16_t         coins;                      /* 0x000C */
    uint8_t         fortress_keys;              /* 0x000E */
    uint8_t         star_pieces;                /* 0x000F */
    uint8_t         star_points;                /* 0x0010 */
    char            unk_0x11[0x01];             /* 0x0011 */
    uint8_t         current_partner;            /* 0x0012 */ /*0x00 - 0x0B*/
    char            unk_0x13[0x01];             /* 0x0013 */
                                                /* size: 0x0014 */
}stats_t;

typedef struct {
    uint8_t         in_party;                   /* 0x00000 */
    uint8_t         upgrade;                    /* 0x00001 */
    char            unk_0x02[0x06];             /* 0x00002 */
                                                /* size: 0x00008 */
}partner_t;

typedef struct{
    partner_t       goombario;                  /* 0x0000 */
    partner_t       kooper;                     /* 0x0008 */
    partner_t       bombette;                   /* 0x0010 */
    partner_t       parakarry;                  /* 0x0018 */
    partner_t       goompa;                     /* 0x0020 */
    partner_t       watt;                       /* 0x0028 */
    partner_t       sushie;                     /* 0x0030 */
    partner_t       lakilester;                 /* 0x0038 */
    partner_t       bow;                        /* 0x0040 */
    partner_t       goombaria;                  /* 0x0048 */
    partner_t       twink;                      /* 0x0050 */
                                                /* size: 0x0058 */
}party_t;

typedef struct{
    char            unk_0x00[0x01];             /* 0x0000 */
    uint8_t         spell_type;                 /* 0x0001 */
    uint8_t         casts_remaining;            /* 0x0002 */
    char            unk_0x03[0x02];             /* 0x0003 */
    uint8_t         turns_until_spell;          /* 0x0005 */
                                                /* size: 0x0006 */
}merlee_t;

typedef struct{
    uint8_t         star_spirits_saved;         /* 0x0000 */
    char            unk_0x01[0x01];             /* 0x0001 */
    uint8_t         full_bars_filled;           /* 0x0002 */
    uint8_t         partial_bars_filled;        /* 0x0003 */
    uint8_t         beam_rank;                  /* 0x0004 */ /*1 for star beam, 2 for peach beam*/
    char            unk_0x05[0x01];             /* 0x0005 */
                                                /* size: 0x0006 */
}star_power_t;

typedef struct{
    int32_t         flags;                      /* 0x0000 */ /*third byte related to cutscenes - write 0 to break free*/
    int32_t         anim_flags;                 /* 0x0004 */
    uint16_t        idle_timer;                 /* 0x0008 */
    char            unk_0x0A[0x02];             /* 0x000A */
    uint16_t        peach_disguise;             /* 0x000C */ /*has something to do with peach transforming in ch6*/
    uint16_t        transparency;               /* 0x000E */ /*0xFF00 when spinning*/
    uint16_t        flash_timer;                /* 0x0010 */ /*used when running away*/
    char            unk_0x12[0x02];             /* 0x0012 */
    uint16_t        busy;                       /* 0x0014 */ /*changed when talking/opening doors/loading zones*/
    int16_t         truncated_x;                /* 0x0016 */ /*used for hazard respawns*/
    char            unk_0x18[0x02];             /* 0x0018 */
    int16_t         truncated_z;                /* 0x001A */ /*used for hazard respawns*/
    char            unk_0x1C[0x0C];             /* 0x001C */
    xyz_t           position;                   /* 0x0028 */
    char            unk_0x34[0x08];             /* 0x0034 */
    float           jumped_from_x;              /* 0x003C */
    float           jumped_from_z;              /* 0x0040 */
    float           landed_at_x;                /* 0x0044 */
    float           landed_at_z;                /* 0x0048 */
    float           jumped_from_y;              /* 0x004C */
    float           last_jump_height;           /* 0x0050 */
    float           speed;                      /* 0x0054 */
    float           walk_speed;                 /* 0x0058 */ /*constant: 0x40000000 = 2.0*/
    float           run_speed;                  /* 0x005C */ /*constant: 0x40800000 = 4.0*/
    char            unk_0x60[0x0C];             /* 0x0060 */
    float           jump_const;                 /* 0x006C */ /*used by jumping func to compare if jump_var_1 less than const*/
    float           y_speed;                    /* 0x0070 */ /*related to rise/fall speed*/
    float           y_acceleration;             /* 0x0074 */ /*related to height cap*/
    float           y_jerk;                     /* 0x0078 */ /*related to height cap*/
    float           y_snap;                     /* 0x007C */ /*related to height cap*/
    float           movement_angle;             /* 0x0080 */ /*locking this makes you move in only that direction regardless of control stick angle*/
    float           facing_angle;               /* 0x0084 */
    char            unk_0x88[0x08];             /* 0x0088 */
    float           body_rotation;              /* 0x0090 */ /*used for turning effect*/
    char            unk_0x94[0x10];             /* 0x0094 */
    int32_t         sprite_animation;           /* 0x00A4 */ /* 1st byte: back turned=01 | 4th byte: animations 00-32*/
    float           left_right;                 /* 0x00A8 */ /*0.0=left, 180.0=right*/
    char            unk_0xAC[0x04];             /* 0x00AC */
    int16_t         collider_height;            /* 0x00B0 */
    int16_t         collider_diameter;          /* 0x00B2 */
    uint8_t         action_state;               /* 0x00B4 */
    uint8_t         prev_action_state;          /* 0x00B5 */
    int8_t          fall_state;                 /* 0x00B6 */ /*also used as sleep state in Peach idle action*/
    char            unk_B7[0x09];               /* 0x00B7 */
    uint16_t        frames_in_air;              /* 0x00C0 */
    char            unk_0xC2[0x02];             /* 0x00C2 */
    char            unk_0xC4[0x02];             /* 0x00C4 */
    uint16_t        interactable_id;            /* 0x00C6 */ /*only for doors?*/
    uint32_t        *talkable_npc;              /* 0x00C8 */
    char            unk_0xCC[0x08];             /* 0x00CC */
    float           spin_variable;              /* 0x00D4 */
    char            unk_0xD8[0x04];             /* 0x00D8 */
    controller_t    raw;                        /* 0x00DC */
    controller_t    previous;                   /* 0x00E0 */
    controller_t    pad_held;                   /* 0x00E4 */
    int32_t         pad_x;                      /* 0x00E8 */
    int32_t         pad_y;                      /* 0x00EC */
    controller_t    pad_held_list[10];          /* 0x00F0 */
    controller_t    pad_pressed_list[10];       /* 0x0118 */
    controller_t    held_timer_list[10];        /* 0x0140 */
    int32_t         pad_x_list[10];             /* 0x0168 */
    int32_t         pad_y_list[10];             /* 0x0190 */
    uint32_t        timer;                      /* 0x01B8 */
    char            unk_0x1BC[0xCC];            /* 0x01BC */
    uint8_t         spin_cooldown_timer;        /* 0x0288 */ /*4 frames at the end of spin*/
    char            unk_0x289[0x02];            /* 0x0289 */
    uint8_t         spin_timer;                 /* 0x028B */
    char            unk_0x28C[0x20];            /* 0x028C */
    float           spin_speed;                 /* 0x02AC */
    char            unk_0x2B0[0x04];            /* 0x02B0 */
    char            unk_0x2B4[0x01];            /* 0x02B4 */
    uint8_t         spin_duration;              /* 0x02B5 */
    char            unk_0x228[0x02];            /* 0x02B6 */
    char            unk_0x2B8[0x10];            /* 0x02B8 */
    stats_t         stats;                      /* 0x02C8 */
    char            unk_0x14[0x08];             /* 0x02DC */
    party_t         party;                      /* 0x02E4 */
    uint16_t        key_items[32];              /* 0x033C */
    uint16_t        badges[128];                /* 0x037C */
    uint16_t        items[10];                  /* 0x047C */
    uint16_t        storage[32];                /* 0x0490 */
    uint16_t        equipped_badges[64];        /* 0x04D0 */
    merlee_t        merlee;                     /* 0x0550 */
    star_power_t    star_power;                 /* 0x0556 */
    int16_t         other_hits_taken;           /* 0x055C */
    int16_t         unk_55E;                    /* 0x055E */
    int16_t         hits_taken;                 /* 0x0560 */
    int16_t         hits_blocked;               /* 0x0562 */
    int16_t         player_first_strikes;       /* 0x0564 */
    int16_t         enemy_first_strikes;        /* 0x0566 */
    int16_t         power_bounces;              /* 0x0568 */
    int16_t         battle_count;               /* 0x056A */
    int16_t         unk_56C[4];                 /* 0x056C */
    int32_t         unk_574[2];                 /* 0x0574 */
    uint32_t        total_coins_earned;         /* 0x057C */
    int16_t         idle_frame_counter;         /* 0x0580 */ /* frames with no inputs, overflows every ~36 minutes of idling */
    char            unk_582[2];                 /* 0x0582 */
    uint32_t        frame_counter;              /* 0x0584 */ /* increases by 2 per frame */
    int16_t         quizzes_answered;           /* 0x0588 */
    int16_t         quizzes_correct;            /* 0x058A */
    int32_t         unk_590[24];                /* 0x058C */
    int32_t         trade_event_start_time;     /* 0x05EC */
    int32_t         unk_5EC;                    /* 0x05F0 */
    int16_t         star_pieces_collected;      /* 0x05F4 */
    int16_t         jump_game_plays;            /* 0x05F6 */
    int32_t         jump_game_total;            /* 0x05F8 */ /* all-time winnings, max = 99999 */
    int16_t         jump_game_record;           /* 0x05FC */
    int16_t         smash_game_plays;           /* 0x05FE */
    int32_t         smash_game_total;           /* 0x0600 */ /* all-time winnings, max = 99999 */
    int16_t         smash_game_record;          /* 0x0604 */

}player_ctxt_t;

typedef struct{
    char            unk_0x00[0x08];             /* 0x0000 */
    uint32_t        room_change_ptr;            /* 0x0008 */
    char            unk_0x0C[0x0C];             /* 0x000C */
                                                /* size: 0x0018 */
}warp_ctxt_t;

typedef struct{
    uint32_t        effects[96];
}effects_ctxt_t;

typedef struct{
    uint32_t        instructions[32];
}ace_ctxt_t;

typedef struct{
    uint16_t        last_timer;
}ace_store_ctxt_t;

typedef struct{
    uint32_t        rng;
}unk4_ctxt_t;

typedef struct{
    uint32_t vi_frames;
}unk5_ctxt_t;

typedef struct {
    char            unk_0x00[0x03];         /* 0x0000 */
    uint8_t         menu_open;              /* 0x0003 */ /* seems to always be non-zero when a menu is open in battle */
}unk6_ctxt_t;

typedef struct {
    uint32_t        turn;                   /* 0x0000 */
    char            unk_0x04[4];            /* 0x0004 */
    uint32_t        turns_since_wave;       /* 0x0008 */
    uint32_t        turns_since_shield;     /* 0x000C */
    uint32_t        turns_since_claw;       /* 0x0010 */
    uint32_t        turns_since_stomp;      /* 0x0014 */
    uint32_t        turns_since_heal;       /* 0x0018 */
}bowser_ctxt_t;

typedef struct {
    uint16_t        mash_bar;               /* 0x0000 */ /* ranges from 0 to 10,000 */
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
