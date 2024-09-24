#include "ed64_io.h"
#include "hb_io.h"
#include "iodev.h"
#include "sc64_io.h"
#include <errno.h>
#include <n64.h>

static u32 clockTicksDflt(void) {
    u32 count;
    __asm__("mfc0    %0, $9;" : "=r"(count));
    return count;
}

static u32 clockFreqDflt(void) {
    return OS_CPU_COUNTER;
}

static struct Iodev *currentDev;

s32 ioInit(void) {
    struct Iodev *devs[] = {
        &homeboyIodev, &everdrive64X, &everdrive64V2, &everdrive64V1, &sc64,
    };

    s32 nDevs = sizeof(devs) / sizeof(devs[0]);
    for (s32 i = 0; i < nDevs; i++) {
        currentDev = devs[i];
        if (currentDev->probe() == 0) {
            return 0;
        }
    }

    currentDev = NULL;
    errno = ENODEV;
    return -1;
}

s32 diskInit(void) {
    if (currentDev && currentDev->diskInit) {
        if (currentDev->diskInit()) {
            errno = ENODEV;
            return -1;
        } else {
            return 0;
        }
    } else {
        errno = ENODEV;
        return -1;
    }
}

s32 diskRead(size_t lba, size_t nBlocks, void *dst) {
    if (currentDev && currentDev->diskRead) {
        if (currentDev->diskRead(lba, nBlocks, dst)) {
            errno = EIO;
            return -1;
        } else {
            return 0;
        }
    } else {
        errno = ENODEV;
        return -1;
    }
}

s32 diskWrite(size_t lba, size_t nBlocks, const void *src) {
    if (currentDev && currentDev->diskWrite) {
        if (currentDev->diskWrite(lba, nBlocks, src)) {
            errno = EIO;
            return -1;
        } else {
            return 0;
        }
    } else {
        errno = ENODEV;
        return -1;
    }
}

s32 fifoPoll(void) {
    if (currentDev && currentDev->fifoPoll) {
        return currentDev->fifoPoll();
    } else {
        errno = ENODEV;
        return 0;
    }
}

s32 fifoRead(void *dst, size_t nBlocks) {
    if (currentDev && currentDev->fifoRead) {
        if (currentDev->fifoRead(dst, nBlocks)) {
            errno = EIO;
            return -1;
        } else {
            return 0;
        }
    } else {
        errno = ENODEV;
        return -1;
    }
}

s32 fifoWrite(const void *src, size_t nBlocks) {
    if (currentDev && currentDev->fifoWrite) {
        if (currentDev->fifoWrite(src, nBlocks)) {
            errno = EIO;
            return -1;
        } else {
            return 0;
        }
    } else {
        errno = ENODEV;
        return -1;
    }
}

u32 clockTicks(void) {
    if (currentDev && currentDev->clockTicks) {
        return currentDev->clockTicks();
    } else {
        return clockTicksDflt();
    }
}

u32 clockFreq(void) {
    if (currentDev && currentDev->clockFreq) {
        return currentDev->clockFreq();
    } else {
        return clockFreqDflt();
    }
}

// s32 cpu_reset(void)
//{
//   if (currentDev && currentDev->cpu_reset)
//     currentDev->cpu_reset();
//   else
//     cpu_reset_dflt();
//
//   return 0;
// }
