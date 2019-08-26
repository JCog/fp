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
    int8_t          actor_related;              /* 0x0070 */ /*some actors dissapear when value is changed*/
    int8_t          demo_flag;                  /* 0x0071 */ /*1 for demo. 0 in normal gameplay freezes mario*/
    int8_t          demo_scene;                 /* 0x0072 */ /*0-0x12 for each demo scene*/
    int8_t          controller_plugged;         /* 0x0073 */ /*needs to be 1 otherwise "no controller" */
    char            unk_0x74[0x08];             /* 0x0074 */
    char            unk_0x7C[0x02];             /* 0x007C */
    int8_t          mario_peach;                /* 0x007E */ /*0= mario, 1=peach*/
    char            unk_0x7F[0x01];             /* 0x007F */
    char            unk_0x80[0x04];             /* 0x0080 */
    char            unk_0x84[0x02];             /* 0x0084 */
    uint16_t        group_id;                   /* 0x0086 */
    uint16_t        group_id_copy;              /* 0x0088 */
    char            unk_0x8A[0x02];             /* 0x008A */
    uint16_t        room_id;                    /* 0x008C */
    uint16_t        entrance_id;                /* 0x008E */
    char            unk_0x90[0x08];             /* 0x0090 */
    xyz_t           player_position_copy;       /* 0x0098 */
    float           player_angle_copy;          /* 0x00A4 */

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
    int32_t         animation;                  /* 0x0000 */ /*third byte related to cutscenes - write 0 to break free*/
    char            unk_0x04[0x04];             /* 0x0004 */
    uint16_t        idle_timer;                 /* 0x0008 */
    char            unk_0x0A[0x02];             /* 0x000A */
    char            unk_0x0C[0x02];             /* 0x000C */
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
    float           y_speed;                    /* 0x0070 */ /*related to rise/fall speeed*/ 
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
    char            unk_0xAC[0x14];             /* 0x00AC */
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

}player_ctxt_t;

typedef struct{
    char            unk_0x00[0x08];             /* 0x0000 */
    uint32_t        room_change_ptr;            /* 0x0008 */
    char            unk_0x0C[0x0C];             /* 0x000C */
                                                /* size: 0x0018 */
}warp_ctxt_t;

/* Addresses */
#define pm_status_addr         0x80074004
#define pm_unk1_addr           0x8009A5A8
#define pm_gfx_addr            0x8009A64C
#define pm_unk2_addr           0x8009E6D0
#define pm_overworld_addr      0x8010ED70
#define pm_hud_addr            0x8010F118
#define pm_player_addr         0x8010F188
#define pm_GameUpdate_addr     0x801181D4
#define pm_warp_addr           0x80156740

/* Data */
#define pm_status             (*(status_ctxt_t*)      pm_status_addr)
#define pm_unk1               (*(unk1_ctxt_t*)        pm_unk1_addr)
#define pm_gfx                (*(gfx_ctxt_t*)         pm_gfx_addr)
#define pm_unk2               (*(unk2_ctxt_t*)        pm_unk2_addr)
#define pm_overworld          (*(overworld_ctxt_t*)   pm_overworld_addr)
#define pm_hud                (*(hud_ctxt_t*)         pm_hud_addr)
#define pm_player             (*(player_ctxt_t*)      pm_player_addr)
#define pm_warp               (*(warp_ctxt_t*)        pm_warp_addr)

/*Function Prototypes*/
typedef void (*pm_GameUpdate_t) ();

/*Functions*/
#define pm_GameUpdate         ((pm_GameUpdate_t)  pm_GameUpdate_addr)

#endif