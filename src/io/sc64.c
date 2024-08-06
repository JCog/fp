#include "sc64.h"
#include "io/iodev.h"
#include "pi.h"
#include "util/util.h"
#include <stddef.h>

static s32 cartIrqf;
static u32 cartLat;
static u32 cartPwd;

_Alignas(16) static u64 cartBuf[512 / 8];

static void cartBufRd(const void *addr) {
    const uu64 *ptr = addr;
    for (s32 i = 0; i < 512 / 8; i += 2) {
        u64 a = ptr[i + 0];
        u64 b = ptr[i + 1];
        cartBuf[i + 0] = a;
        cartBuf[i + 1] = b;
    }
}

static void cartBufWr(void *addr) {
    uu64 *ptr = addr;
    for (s32 i = 0; i < 512 / 8; i += 2) {
        u64 a = cartBuf[i + 0];
        u64 b = cartBuf[i + 1];
        ptr[i + 0] = a;
        ptr[i + 1] = b;
    }
}

static void cartLock(void) {
    __osPiGetAccess();

    cartIrqf = setIrqf(0);

    cartLat = pi_regs.dom1_lat;
    cartPwd = pi_regs.dom1_pwd;
}

static void cartUnlock(void) {
    pi_regs.dom1_lat = cartLat;
    pi_regs.dom1_pwd = cartPwd;

    __osPiRelAccess();

    setIrqf(cartIrqf);
}

static inline u32 regRd(s32 reg) {
    return __piReadRaw((u32)&REGS_PTR[reg]);
}

static inline void regWr(s32 reg, u32 dat) {
    return __piWriteRaw((u32)&REGS_PTR[reg], dat);
}

static s32 scSync(void) {
    while (regRd(SC_STATUS_REG) & SC_CMD_BUSY) {
        ;
    }
    if (regRd(SC_STATUS_REG) & SC_CMD_ERROR) {
        return -1;
    }
    return 0;
}

static s32 probe(void) {
    cartLock();

    /* open registers */
    regWr(SC_KEY_REG, SC_KEY_RESET);
    regWr(SC_KEY_REG, SC_KEY_UNL);
    regWr(SC_KEY_REG, SC_KEY_OCK);

    /* check magic number */
    if ((regRd(SC_IDENTIFIER_REG)) != SC_IDENTIFIER) {
        regWr(SC_KEY_REG, 0);
        CART_ABORT();
    }

    scSync();
    cartUnlock();
    return 0;
}

static s32 diskInit(void) {
    cartLock();
    scSync();

    regWr(SC_DATA1_REG, SC_SD_INIT);
    regWr(SC_COMMAND_REG, SC_SD_OP);

    if (scSync()) {
        regWr(SC_KEY_REG, 0);
        CART_ABORT();
    }

    cartUnlock();
    return 0;
}

static s32 diskRead(size_t lba, size_t nBlocks, void *dst) {
    char *addr = dst;
    s32 n;
    cartLock();
    scSync();

    while (nBlocks > 0) {
        n = nBlocks < 16 ? nBlocks : 16;

        regWr(SC_DATA0_REG, lba);
        regWr(SC_COMMAND_REG, SC_SD_SECTOR_SET);

        if (scSync()) {
            CART_ABORT();
        }

        regWr(SC_DATA0_REG, SC_BUFFER_REG);
        regWr(SC_DATA1_REG, n);
        regWr(SC_COMMAND_REG, SC_SD_READ);

        if (scSync()) {
            CART_ABORT();
        }

        if ((u32)addr & 7) {
            for (s32 i = 0; i < n; i++) {
                piReadLocked(SC_BUFFER_REG + 512 * i, cartBuf, 512);
                cartBufWr(addr);
                addr += 512;
            }
        } else {
            piReadLocked(SC_BUFFER_REG, addr, 512 * n);
            addr += 512 * n;
        }

        lba += n;
        nBlocks -= n;
    }

    cartUnlock();
    return 0;
}

static s32 diskWrite(size_t lba, size_t nBlocks, const void *src) {
    const char *addr = src;
    s32 n;
    cartLock();
    scSync();

    while (nBlocks > 0) {
        n = nBlocks < 16 ? nBlocks : 16;

        if ((u32)addr & 7) {
            for (s32 i = 0; i < n; i++) {
                cartBufRd(addr);
                piWriteLocked(SC_BUFFER_REG + 512 * i, cartBuf, 512);
                addr += 512;
            }
        } else {
            piWriteLocked(SC_BUFFER_REG, addr, 512 * n);
            addr += 512 * n;
        }

        scSync();
        regWr(SC_DATA0_REG, lba);
        regWr(SC_COMMAND_REG, SC_SD_SECTOR_SET);

        if (scSync()) {
            CART_ABORT();
        }

        regWr(SC_DATA0_REG, SC_BUFFER_REG);
        regWr(SC_DATA1_REG, n);
        regWr(SC_COMMAND_REG, SC_SD_WRITE);

        if (scSync()) {
            CART_ABORT();
        }

        lba += n;
        nBlocks -= n;
    }

    cartUnlock();
    return 0;
}

struct Iodev sc64 = {.probe = probe, .diskInit = diskInit, .diskRead = diskRead, .diskWrite = diskWrite};
