#ifndef SC64_H
#define SC64_H

#include "types.h"

#define SC_BASE_REG        0xBFFF0000
#define SC_BUFFER_REG      0xBFFE0000
#define REGS_PTR           ((volatile u32 *)SC_BASE_REG)

#define SC_STATUS_REG      0
#define SC_COMMAND_REG     0
#define SC_DATA0_REG       1
#define SC_DATA1_REG       2
#define SC_IDENTIFIER_REG  3
#define SC_KEY_REG         4

#define SC_CMD_BUSY        0x80000000
#define SC_CMD_ERROR       0x40000000
#define SC_IRQ_PENDING     0x20000000

#define SC_CONFIG_GET      'c'
#define SC_CONFIG_SET      'C'
#define SC_SD_OP           'i'
#define SC_SD_SECTOR_SET   'I'
#define SC_SD_READ         's'
#define SC_SD_WRITE        'S'

#define SC_CFG_ROM_WRITE   1
#define SC_CFG_DD_MODE     3
#define SC_CFG_SAVE_TYPE   6

#define SC_SD_DEINIT       0
#define SC_SD_INIT         1
#define SC_SD_GET_STATUS   2
#define SC_SD_GET_INFO     3
#define SC_SD_BYTESWAP_ON  4
#define SC_SD_BYTESWAP_OFF 5

#define SC_DD_MODE_REGS    1
#define SC_DD_MODE_IPL     2

#define SC_IDENTIFIER      0x53437632 /* SCv2 */

#define SC_KEY_RESET       0x00000000
#define SC_KEY_LOCK        0xFFFFFFFF
#define SC_KEY_UNL         0x5F554E4C /* _UNL */
#define SC_KEY_OCK         0x4F434B5F /* OCK_ */

typedef u64 uu64 __attribute__((aligned(1))); // NOLINT
#define CART_ABORT()  \
    {                 \
        cartUnlock(); \
        return -1;    \
    }

#endif
