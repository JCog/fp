#include "common.h"
#include "ed64_l.h"
#include "iodev.h"
#include "pi.h"
#include "sd_host.h"
#include "util/util.h"
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

static inline void spiMode(s32 cfg) {
    spiCfg &= ~(SPI_RD | SPI_DAT | SPI_1CLK);
    spiCfg |= cfg;

    regWr(REG_SPI_CFG, spiCfg);
}

static inline void spiTx(u8 dat) {
    regWr(REG_SPI, dat);

    while (regRd(REG_STATUS) & STATUS_SPI) {
        ;
    }
}

static inline u8 spiRx(void) {
    spiTx(0xFF);

    return regRd(REG_SPI);
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

static s32 sdCmdRx(void) {
    spiMode(SPI_CMD | SPI_RD | SPI_1CLK);

    return spiRx() & 0x1;
}

static s32 sdDatRx(void) {
    spiMode(SPI_DAT | SPI_RD | SPI_1CLK);

    return spiRx() & 0xF;
}

static void sdDatTx(s32 dat) {
    spiMode(SPI_DAT | SPI_WR | SPI_1CLK);

    spiTx((dat << 4) | 0x0F);
}

static void sdCmdRxBuf(void *buf, size_t size) {
    u8 *p = buf;

    spiMode(SPI_CMD | SPI_RD | SPI_BYTE);

    for (size_t i = 0; i < size; i++) {
        *p++ = spiRx();
    }
}

static void sdCmdTxBuf(const void *buf, size_t size) {
    const u8 *p = buf;

    spiMode(SPI_CMD | SPI_WR | SPI_BYTE);

    for (size_t i = 0; i < size; i++) {
        spiTx(*p++);
    }
}

static void sdDatRxBuf(void *buf, size_t size) {
    u8 *p = buf;

    spiMode(SPI_DAT | SPI_RD | SPI_BYTE);

    for (size_t i = 0; i < size; i++) {
        *p++ = spiRx();
    }
}

static void sdDatTxBuf(const void *buf, size_t size) {
    const u8 *p = buf;

    spiMode(SPI_DAT | SPI_WR | SPI_BYTE);

    for (size_t i = 0; i < size; i++) {
        spiTx(*p++);
    }
}

static void sdDatTxClk(s32 dat, size_t nClk) {
    dat = dat & 0xF;
    dat = (dat << 4) | dat;

    spiMode(SPI_DAT | SPI_WR | SPI_BYTE);

    for (size_t i = 0; i < nClk / 2; i++) {
        spiTx(dat);
    }
}

static s32 sdRxMblk(void *buf, size_t blkSize, size_t nBlk) {
    const u32 cartAddr = 0xB2000000;

    /* dma to cart */
    spiMode(SPI_DAT | SPI_RD | SPI_1CLK);
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
    .proto = SD_PROTO_SDBUS,

    .lock = cartLock,
    .unlock = cartUnlock,
    .setSpd = sdSetSpd,

    .cmdRx = sdCmdRx,
    .datRx = sdDatRx,
    .datTx = sdDatTx,
    .cmdRxBuf = sdCmdRxBuf,
    .cmdTxBuf = sdCmdTxBuf,
    .datRxBuf = sdDatRxBuf,
    .datTxBuf = sdDatTxBuf,
    .datTxClk = sdDatTxClk,

    .rxMblk = sdRxMblk,
};

static s32 probe(void) {
    cartLockSafe();

    /* open registers */
    regWr(REG_KEY, 0x1234);

    /* check firmware version */
    u16 fwVer = regRd(REG_VER);
    if (fwVer < 0x0116) {
        goto nodev;
    }

    /* check spi device */
    /* for a v2 device we expect a write with this config to trigger one
     * clock with DAT0-DAT3 high */
    regWr(REG_SPI_CFG, SPI_SPEED_LO | SPI_SS | SPI_RD | SPI_DAT | SPI_1CLK);
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
    if (dat == 0x0F) {
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

static s32 fifoPoll(void) {
    s32 ret;

    cartLock();
    if (regRd(REG_STATUS) & STATUS_RXF) {
        ret = 0;
    } else {
        ret = 1;
    }
    cartUnlock();

    return ret;
}

static s32 fifoRead(void *dst, size_t nBlocks) {
    const u32 cartAddr = 0xB2000000;
    const size_t blkSize = 512;

    cartLock();

    /* wait for rx buffer full (RXF low) */
    while (regRd(REG_STATUS) & STATUS_RXF) {
        ;
    }

    /* dma fifo to cart */
    regWr(REG_DMA_LEN, nBlocks - 1);
    regWr(REG_DMA_ADDR, cartAddr >> 11);
    regWr(REG_DMA_CFG, DMA_FIFO_TO_RAM);
    while (regRd(REG_STATUS) & STATUS_DMA_BUSY) {
        ;
    }

    /* check for dma timeout */
    if (regRd(REG_STATUS) & STATUS_DMA_TOUT) {
        cartUnlock();
        return -1;
    }

    /* copy to ram */
    piReadLocked(cartAddr, dst, nBlocks * blkSize);

    cartUnlock();
    return 0;
}

static s32 fifoWrite(const void *src, size_t nBlocks) {
    const u32 cartAddr = 0xB2000000;
    const size_t blkSize = 512;

    cartLock();

    /* wait for tx buffer empty (TXE low) */
    while (regRd(REG_STATUS) & STATUS_TXE) {
        ;
    }

    /* copy to cart */
    piWriteLocked(cartAddr, src, nBlocks * blkSize);

    /* dma cart to fifo */
    regWr(REG_DMA_LEN, nBlocks - 1);
    regWr(REG_DMA_ADDR, cartAddr >> 11);
    regWr(REG_DMA_CFG, DMA_RAM_TO_FIFO);
    while (regRd(REG_STATUS) & STATUS_DMA_BUSY) {
        ;
    }

    /* check for dma timeout */
    if (regRd(REG_STATUS) & STATUS_DMA_TOUT) {
        cartUnlock();
        return -1;
    }

    cartUnlock();
    return 0;
}

struct Iodev everdrive64V2 = {
    .probe = probe,

    .diskInit = diskInit,
    .diskRead = diskRead,
    .diskWrite = diskWrite,

    .fifoPoll = fifoPoll,
    .fifoRead = fifoRead,
    .fifoWrite = fifoWrite,
};
