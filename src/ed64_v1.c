#include "common.h"
#include "ed64_l.h"
#include "iodev.h"
#include "pi.h"
#include "sd_host.h"
#include "util.h"
#include <n64.h>
#include <stddef.h>

static s32 cartIrqf;
static u32 cartLat;
static u32 cartPwd;
static u16 spiCfg;

static void cartLockSafe(void) {
    __osPiGetAccess();

    cartIrqf = setIrqf(0);

    cartLat = pi_regs.dom2_lat;
    cartPwd = pi_regs.dom2_pwd;
}

static void cartLock(void) {
    cartLockSafe();

    pi_regs.dom2_lat = 4;
    pi_regs.dom2_pwd = 12;
}

static void cartUnlock(void) {
    pi_regs.dom2_lat = cartLat;
    pi_regs.dom2_pwd = cartPwd;

    __osPiRelAccess();

    setIrqf(cartIrqf);
}

static inline u32 regRd(s32 reg) {
    return __piReadRaw((u32)&REGS_PTR[reg]);
}

static inline void regWr(s32 reg, u32 dat) {
    return __piWriteRaw((u32)&REGS_PTR[reg], dat);
}

static inline void spiTx(u8 dat) {
    regWr(REG_SPI, dat);

    while (regRd(REG_STATUS) & STATUS_SPI) {
        ;
    }
}

static inline u8 spiIo(u8 dat) {
    spiTx(dat);

    return regRd(REG_SPI);
}

static inline u8 spiRx(void) {
    return spiIo(0xFF);
}

static void sdSetSpd(s32 spd) {
    spiCfg &= ~SPI_SPEED;

    if (spd >= 50) {
        spiCfg |= SPI_SPEED_50;
    } else if (spd >= 25) {
        spiCfg |= SPI_SPEED_25;
    } else {
        spiCfg |= SPI_SPEED_LO;
    }

    regWr(REG_SPI_CFG, spiCfg);
}

static void sdSpiSs(s32 ss) {
    if (ss) {
        spiCfg &= ~SPI_SS;
    } else {
        spiCfg |= SPI_SS;
    }

    regWr(REG_SPI_CFG, spiCfg);
}

static s32 sdSpiIo(s32 dat) {
    return spiIo(dat);
}

static void sdSpiRxBuf(void *buf, size_t size) {
    u8 *p = buf;

    for (size_t i = 0; i < size; i++) {
        *p++ = spiRx();
    }
}

static void sdSpiTxBuf(const void *buf, size_t size) {
    const u8 *p = buf;

    for (size_t i = 0; i < size; i++) {
        spiTx(*p++);
    }
}

static void sdSpiTxClk(s32 dat, size_t nClk) {
    if (dat & 0x1) {
        dat = 0xFF;
    } else {
        dat = 0x00;
    }

    for (size_t i = 0; i < nClk / 8; i++) {
        spiTx(dat);
    }
}

static s32 sdRxMblk(void *buf, size_t blkSize, size_t nBlk) {
    const u32 cartAddr = 0xB2000000;

    /* dma to cart */
    regWr(REG_DMA_LEN, nBlk - 1);
    regWr(REG_DMA_ADDR, cartAddr >> 11);
    regWr(REG_DMA_CFG, DMA_SD_TO_RAM);
    while (regRd(REG_STATUS) & STATUS_DMA_BUSY) {
        ;
    }

    /* check for dma timeout */
    if (regRd(REG_STATUS) & STATUS_DMA_TOUT) {
        return -SD_ERR_TIMEOUT;
    }

    /* copy to ram */
    piReadLocked(cartAddr, buf, blkSize * nBlk);

    return 0;
}

static struct SdHost sdHost = {
    .proto = SD_PROTO_SPIBUS,

    .lock = cartLock,
    .unlock = cartUnlock,
    .setSpd = sdSetSpd,

    .spiSs = sdSpiSs,
    .spiIo = sdSpiIo,
    .spiRxBuf = sdSpiRxBuf,
    .spiTxBuf = sdSpiTxBuf,
    .spiTxClk = sdSpiTxClk,

    .rxMblk = sdRxMblk,
};

static s32 probe(void) {
    cartLockSafe();

    /* open registers */
    regWr(REG_KEY, 0x1234);

    /* check firmware version */
    u16 fwVer = regRd(REG_VER);
    if (fwVer < 0x0100 || fwVer >= 0x0116) {
        goto nodev;
    }

    /* check spi device */
    /* for a v1 device we expect a write to trigger eight clocks with the MISO
     * line high */
    regWr(REG_SPI_CFG, SPI_SPEED_LO | SPI_SS);
    regWr(REG_SPI, 0x00);
    for (s32 i = 0;; i++) {
        if (i > 32) {
            goto nodev;
        }

        if ((regRd(REG_STATUS) & STATUS_SPI) == 0) {
            break;
        }
    }
    u16 dat = regRd(REG_SPI);
    if (dat == 0xFF) {
        /* spi seems to work as expected */
        cartUnlock();
        return 0;
    }

nodev:
    regWr(REG_KEY, 0);
    cartUnlock();
    return -1;
}

static s32 diskInit(void) {
    return sdInit(&sdHost);
}

static s32 diskRead(size_t lba, size_t nBlocks, void *dst) {
    return sdRead(&sdHost, lba, dst, nBlocks);
}

static s32 diskWrite(size_t lba, size_t nBlocks, const void *src) {
    return sdWrite(&sdHost, lba, src, nBlocks);
}

struct Iodev everdrive64V1 = {
    .probe = probe,

    .diskInit = diskInit,
    .diskRead = diskRead,
    .diskWrite = diskWrite,
};
