#include "hb.h"
#include "iodev.h"
#include <mips.h>
#include <stddef.h>

/*  Homeboy devices are currently only implemented on systems that don't
 *  emulate the CPU cache. No flushing/invalidating is done.
 */

static s32 hbCheck(void) {
    if (hb_regs.key == 0x1234) {
        return 0;
    } else {
        return -1;
    }
}

static s32 hbSdInit(void) {
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

static s32 hbSdRead(size_t lba, size_t nBlocks, void *dst) {
    hb_regs.sdDramAddr = MIPS_KSEG0_TO_PHYS(dst);
    hb_regs.sdNBlocks = nBlocks;
    hb_regs.sdReadLba = lba;
    while (hb_regs.status & HB_STATUS_SD_BUSY) {
        ;
    }

    if (hb_regs.status & HB_STATUS_ERROR) {
        return -1;
    } else {
        return 0;
    }
}

static s32 hbSdWrite(size_t lba, size_t nBlocks, const void *src) {
    if (src != NULL) {
        hb_regs.sdDramAddr = MIPS_KSEG0_TO_PHYS(src);
        hb_regs.sdNBlocks = nBlocks;
        hb_regs.sdWriteLba = lba;
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

        while (nBlocks != 0) {
            if (hbSdWrite(lba, 1, data)) {
                return -1;
            }

            nBlocks--;
            lba++;
        }

        return 0;
    }
}

static s32 hbReset(u32 dramSaveAddr, u32 dramSaveLen) {
    hb_regs.dramSaveAddr = dramSaveAddr;
    hb_regs.dramSaveLen = dramSaveLen;
    hb_regs.status = HB_STATUS_RESET;

    return 0;
}

static u64 hbGetTimebase64(void) {
    return ((u64)hb_regs.timebaseHi << 32) | hb_regs.timebaseLo;
}

static u32 clockTicks(void) {
    return hb_regs.timebaseLo;
}

static u32 clockFreq(void) {
    return HB_TIMEBASE_FREQ;
}

static void cpuReset(void) {
    /* simulate 0.5s nmi delay */
    u64 tbWait = hbGetTimebase64() + HB_TIMEBASE_FREQ / 2;
    while (hbGetTimebase64() < tbWait) {
        ;
    }

    hbReset(0x00400000, 0x00400000);
}

struct Iodev homeboyIodev = {
    .probe = hbCheck,

    .diskInit = hbSdInit,
    .diskRead = hbSdRead,
    .diskWrite = hbSdWrite,

    .clockTicks = clockTicks,
    .clockFreq = clockFreq,

    .cpuReset = cpuReset,
};
