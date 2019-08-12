#ifndef _PM64_H
#define _PM64_H
#include <n64.h>
#include <stdint.h>

#define PM64_SCREEN_WIDTH    320
#define PM64_SCREEN_HEIGHT   240

typedef struct{
    Gfx *p;
    uint32_t unk;
    Gfx *buf;

}gfx_t;

typedef struct{
    float       x;                            /* 0x0000 */
    float       y;                            /* 0x0004 */
    float       z;                            /* 0x0008 */
                                              /* size: 0x000C */
}xyz_t;

typedef struct{
    int8_t              x_cardinal;           /* 0x0000 */
    int8_t              y_cardinal;           /* 0x0001 */
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
                                                /*size: 0x0004*/             
    };
}controller_t;

typedef struct{
    controller_t    raw;                        /* 0x0000 */ /* raw input */
    char            unk_0x04[0x0C];             /* 0x0004 */            
    controller_t    press_release;              /* 0x0010 */ /* one frame when pressed or released */
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
    int16_t         hold_timer;                 /* 0x0058*/  /*counts from 0F to 00 to start held variable*/
    char            unk_0x5A[0x02];             /* 0x005A */            
    char            unk_0x5C[0x04];             /* 0x005C */            
    int16_t         held_timer;                 /* 0x0060 */ /*FFFF when nothing held, when hold_timer hits 0 it will cycle between 0 and 3*/
    char            unk_0x62[0x02];             /* 0x0062 */       
                                                /* size: 0x0064 */    

}input_t;

typedef struct{
	controller_t	raw;						/* 0x0000 */
	
												/* size: 0x0050 */   
}player_input_t;

typedef struct{
    input_t         input;                      /* 0x0000 */
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
    int16_t         group_id;                   /* 0x0086 */
    int16_t         group_id_copy;              /* 0x0088 */
    char            unk_0x8A[0x02];             /* 0x008A */
    int16_t         room_id;                    /* 0x008C */
    int16_t         entrance_id;                /* 0x008E */
    char            unk_0x90[0x08];             /* 0x0090 */
    xyz_t           player_position_copy;       /* 0x0098 */
    float           player_angle_copy;          /* 0x00A4 */

}status_t;

typedef struct{
    int32_t         animation_related;          /*0x0000*/ /*third byte related to cutscenes - write 0 to break free*/
    char            unk_0x04[0x04];             /*0x0004*/
    uint16_t        idle_timer;                 /*0x0008*/
    char            unk_0x0A[0x02];             /*0x000A*/
    char            unk_0x0C[0x02];             /*0x000C*/
    uint16_t        transparency;               /*0x000E*/ /*0xFF00 when spinning*/
    uint16_t        flash_timer;                /*0x0010*/ /*used when running away*/
    char            unk_0x12[0x02];             /*0x0012*/
    uint16_t        busy;             			/*0x0014*/ /*changed when talking/opening doors/loading zones*/
    int16_t			truncated_x;				/*0x0016*/ /*used for hazard respawns*/
    char            unk_0x18[0x02];             /*0x0018*/
    int16_t			truncated_z;				/*0x001A*/ /*used for hazard respawns*/
    char            unk_0x1C[0x0C];             /*0x001C*/
    xyz_t			position;					/*0x0028*/
    char            unk_0x34[0x08];             /*0x0034*/
    float			jumped_from_x;				/*0x003C*/
    float			jumped_from_z;				/*0x0040*/
    float			landed_at_x;				/*0x0044*/
    float			landed_at_z;				/*0x0048*/
    float			jumped_from_y;				/*0x004C*/
    float			last_jump_height;			/*0x0050*/
    float			speed;						/*0x0054*/
    float			walk_speed;					/*0x0058*/ /*constant: 0x40000000 = 2.0*/
    float			run_speed;					/*0x005C*/ /*constant: 0x40800000 = 4.0*/
    char            unk_0x60[0x0C];             /*0x0060*/
    float			jump_const;					/*0x006C*/ /*used by jumping func to compare if jump_var_1 less than const*/
	float			jump_var_1;					/*0x0070*/ /*related to rise/fall speeed*/ 
	float			jump_var_2;					/*0x0074*/ /*related to height cap*/
	float			jump_var_3;					/*0x0078*/ /*related to height cap*/
	float			jump_var_4;					/*0x007C*/ /*related to height cap*/
	float			movement_angle;				/*0x0080*/ /*locking this makes you move in only that direction regardless of control stick angle*/
	float			facing_angle;				/*0x0084*/
	char			unk_0x88[0x08];				/*0x0088*/
	float			body_rotation;				/*0x0090*/ /*used for turning effect*/
	char			unk_0x94[0x10];				/*0x0094*/
	int32_t			sprite_animation;			/*0x00A4*/ /* 1st byte: back turned=01 | 4th byte: animations 00-32*/
	float			left_right;					/*0x00A8*/ /*0.0=left, 180.0=right*/
	char			unk_0xAC[0x14];				/*0x00AC*/
	uint16_t		frames_in_air;				/*0x00C0*/
	char			unk_0xC2[0x02];				/*0x00C2*/
	char			unk_0xC4[0x02];				/*0x00C4*/
	uint16_t		interactable_id;			/*0x00C8*/ /*only for doors?*/
	int32_t			*talkable_npc;				/*0x00CA*/
	char			unk_0xCE[0x08];				/*0x00CE*/
	float			spin_variable;				/*0x00D6*/
	char			unk_0xDA[0x04];				/*0x00DA*/
	player_input_t  player_input;				/*0x00DE*/


}player_t;

/* Addresses */
#define pm_Status_addr         0x80074004
#define pm_DlistBuffer_addr    0x8009A64C
#define pm_player_addr         0x8010F188
#define pm_GameUpdate_addr     0x801181D4


/* Data */
#define pm_disp_buf           (*(gfx_t*)      pm_DlistBuffer_addr)
#define pm_status             (*(status_t*)   pm_Status_addr)
#define pm_player             (*(player_t*)   pm_player_addr)

/*Function Prototypes*/
typedef void (*pm_GameUpdate_t) ();

/*Functions*/
#define pm_GameUpdate         ((pm_GameUpdate_t)  pm_GameUpdate_addr)

#endif