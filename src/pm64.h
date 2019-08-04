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
    int8_t              x_cardinal;      /* 0x0000 */
    int8_t              y_cardinal;      /* 0x0002 */
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
      uint16_t          buttons;                /* 0x0004 */               
    };
}input_t;

typedef struct{
    input_t         raw;                       /* 0x0000 */ /* raw input */
    char            unk_0x04[0x0C];            /* 0x0004 */
    input_t         pad_press_release;         /* 0x0010 */ /* one frame when pressed or released */
    char            unk_0x14[0x0C];            /* 0x0014 */
    input_t         pad_held;                  /* 0x0020 */ /* once every 4 frames */
    char            unk_0x24[0x0C];            /* 0x0024 */
    input_t         previous;                  /* 0x0030 */
    char            unk_0x34[0x0C];            /* 0x0034 */
    int8_t          control_x;                 /* 0x0040 */ 
    char            unk_0x41[0x03];            /* 0x0041 */
    int8_t          control_y;                 /* 0x0044 */
    char            unk_0x45[0x03];            /* 0x0045 */

}status_t;

/* Addresses */
#define pm_Status_addr         0x80074004
#define pm_DlistBuffer_addr    0x8009A64C
#define pm_GameUpdate_addr     0x801181D4


/* Data */
#define disp_buf              (*(gfx_t*)      pm_DlistBuffer_addr)
#define pm_status             (*(status_t*)   pm_Status_addr)

/*function prototypes*/
typedef void (*pm_GameUpdate_t) ();

/*functions*/
#define pm_GameUpdate         ((pm_GameUpdate_t)  pm_GameUpdate_addr)

#endif