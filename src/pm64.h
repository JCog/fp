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
  union
  {
    struct
    {
      uint16_t      a                   : 1;
      uint16_t      b                   : 1;
      uint16_t      z                   : 1;
      uint16_t      s                   : 1;
      uint16_t      du                  : 1;
      uint16_t      dd                  : 1;
      uint16_t      dl                  : 1;
      uint16_t      dr                  : 1;
      uint16_t                          : 2;
      uint16_t      l                   : 1;
      uint16_t      r                   : 1;
      uint16_t      cu                  : 1;
      uint16_t      cd                  : 1;
      uint16_t      cl                  : 1;
      uint16_t      cr                  : 1;
    };
    uint16_t        pad;                      /* 0x0000 */
  };
  char				unk_0x04[0x3C];           /* 0x0004 */
  int8_t            x;                        /* 0x0040 */
  char              unk_0x41[0x03];           /* 0x0041 */
  int8_t            y;                        /* 0x0044 */
                                              

}input_t;

typedef struct{
	input_t         input;                       /* 0x0000 */


}status_t;

/* Addresses */
#define pm_Status_addr         0x80074004
#define pm_dlistBuffer_addr    0x8009A64C
#define pm_GameUpdate_addr     0x801181D4


/* Data */
#define disp_buf              (*(gfx_t*)      pm_dlistBuffer_addr)
#define pm_status             ((status_t)   pm_Status_addr)

/*function prototypes*/
typedef void (*pm_GameUpdate_t) ();

/*functions*/
#define pm_GameUpdate         ((pm_GameUpdate_t)  pm_GameUpdate_addr)

#endif