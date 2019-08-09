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
    xyz_t           mario_pos_copy;             /* 0x0098 */
    float           mario_angle_copy;           /* 0x00A4 */

}status_t;


/* Addresses */
#define pm_Status_addr         0x80074004
#define pm_DlistBuffer_addr    0x8009A64C
#define pm_GameUpdate_addr     0x801181D4


/* Data */
#define disp_buf              (*(gfx_t*)      pm_DlistBuffer_addr)
#define pm_status             (*(status_t*)   pm_Status_addr)

/*Function Prototypes*/
typedef void (*pm_GameUpdate_t) ();

/*Functions*/
#define pm_GameUpdate         ((pm_GameUpdate_t)  pm_GameUpdate_addr)

#endif