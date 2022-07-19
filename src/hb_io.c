#include "hb.h"
#include "iodev.h"
#include <mips.h>
#include <stddef.h>

/*  Homeboy devices are currently only implemented on systems that don't
 *  emulate the CPU cache. No flushing/invalidating is done.
 */

static s32 hb_check(void) {
    if (hb_regs.key == 0x1234) {
        return 0;
    } else {
        return -1;
    }
}

static s32 hb_sd_init(void) {
    hb_regs.status = HB_STATUS_SD_INIT;
    while (hb_regs.status & HB_STATUS_SD_BUSY) {
        ;
    }

    u32 status = hb_regs.status;
    if ((status & HB_STATUS_SD_READY) && (status & HB_STATUS_SD_INSERTED)) {
        return 0;
    } else {
        return -1;
    }
}

static s32 hb_sd_read(size_t lba, size_t n_blocks, void *dst) {
    hb_regs.sd_dram_addr = MIPS_KSEG0_TO_PHYS(dst);
    hb_regs.sd_n_blocks = n_blocks;
    hb_regs.sd_read_lba = lba;
    while (hb_regs.status & HB_STATUS_SD_BUSY) {
        ;
    }

    if (hb_regs.status & HB_STATUS_ERROR) {
        return -1;
    } else {
        return 0;
    }
}

static s32 hb_sd_write(size_t lba, size_t n_blocks, const void *src) {
    if (src != NULL) {
        hb_regs.sd_dram_addr = MIPS_KSEG0_TO_PHYS(src);
        hb_regs.sd_n_blocks = n_blocks;
        hb_regs.sd_write_lba = lba;
        while (hb_regs.status & HB_STATUS_SD_BUSY) {
            ;
        }

        if (hb_regs.status & HB_STATUS_ERROR) {
            return -1;
        } else {
            return 0;
        }
    } else {
        char data[512] = {0};

        while (n_blocks != 0) {
            if (hb_sd_write(lba, 1, data)) {
                return -1;
            }

            n_blocks--;
            lba++;
        }

        return 0;
    }
}

static s32 hb_reset(u32 dram_save_addr, u32 dram_save_len) {
    hb_regs.dram_save_addr = dram_save_addr;
    hb_regs.dram_save_len = dram_save_len;
    hb_regs.status = HB_STATUS_RESET;

    return 0;
}

static u64 hb_get_timebase64(void) {
    return ((u64)hb_regs.timebase_hi << 32) | hb_regs.timebase_lo;
}

static u32 clock_ticks(void) {
    return hb_regs.timebase_lo;
}

static u32 clock_freq(void) {
    return HB_TIMEBASE_FREQ;
}

static void cpu_reset(void) {
    /* simulate 0.5s nmi delay */
    u64 tb_wait = hb_get_timebase64() + HB_TIMEBASE_FREQ / 2;
    while (hb_get_timebase64() < tb_wait) {
        ;
    }

    hb_reset(0x00400000, 0x00400000);
}

struct iodev homeboy_iodev = {
    .probe = hb_check,

    .disk_init = hb_sd_init,
    .disk_read = hb_sd_read,
    .disk_write = hb_sd_write,

    .clock_ticks = clock_ticks,
    .clock_freq = clock_freq,

    .cpu_reset = cpu_reset,
};
