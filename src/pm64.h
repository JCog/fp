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


#define disp_buf (*(gfx_t*)    0x8009A64C)
#define pm_GameUpdate_addr     0x801181D4

/*function prototypes*/
typedef void (*pm_GameUpdate_t) ();

/*functions*/
#define pm_GameUpdate         ((pm_GameUpdate_t)  pm_GameUpdate_addr)

#endif