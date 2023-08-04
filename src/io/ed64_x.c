#include "ed64_x.h"
#include "common.h"
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

    cartLat = pi_regs.dom1_lat;
    cartPwd = pi_regs.dom1_pwd;
}

static void cartLock(void) {
    cartLockSafe();

    pi_regs.dom1_lat = 4;
    pi_regs.dom1_pwd = 12;
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

static inline void spiNclk(s32 bitlen) {
    spiCfg &= ~SD_CFG_BITLEN;
    spiCfg |= bitlen;

    regWr(REG_SD_STATUS, spiCfg);
}

static inline void cmdTx(u8 dat) {
    regWr(REG_SD_CMD_WR, dat);

    while (regRd(REG_SD_STATUS) & SD_STA_BUSY) {
        ;
    }
}

static inline u8 cmdRx(void) {
    regWr(REG_SD_CMD_RD, 0xFF);

    while (regRd(REG_SD_STATUS) & SD_STA_BUSY) {
        ;
    }

    return regRd(REG_SD_CMD_RD);
}

static inline void datTx(u16 dat) {
    regWr(REG_SD_DAT_WR, dat);

    while (regRd(REG_SD_STATUS) & SD_STA_BUSY) {
        ;
    }
}

static inline u16 datRx(void) {
    regWr(REG_SD_DAT_RD, 0xFFFF);

    while (regRd(REG_SD_STATUS) & SD_STA_BUSY) {
        ;
    }

    return regRd(REG_SD_DAT_RD);
}

static void sdSetSpd(s32 spd) {
    /* The ED64-X IO don't seem to support Default Speed (25MHz), so I guess
     * we'd better hope that the card supports High Speed (50MHz).
     */
    if (spd >= 25) {
        spiCfg |= SD_CFG_SPD;
    } else {
        spiCfg &= ~SD_CFG_SPD;
    }

    regWr(REG_SD_STATUS, spiCfg);
}

static s32 sdCmdRx(void) {
    spiNclk(1);

    return cmdRx() & 0x1;
}

static s32 sdDatRx(void) {
    spiNclk(1);

    return datRx() & 0xF;
}

static void sdDatTx(s32 dat) {
    spiNclk(1);

    datTx((dat << 12) | 0x0FFF);
}

static void sdCmdRxBuf(void *buf, size_t size) {
    u8 *p = buf;

    spiNclk(8);

    for (size_t i = 0; i < size; i++) {
        *p++ = cmdRx();
    }
}

static void sdCmdTxBuf(const void *buf, size_t size) {
    const u8 *p = buf;

    spiNclk(8);

    for (size_t i = 0; i < size; i++) {
        cmdTx(*p++);
    }
}

static void sdDatRxBuf(void *buf, size_t size) {
    u8 *p = buf;

    spiNclk(4);

    for (size_t i = 0; i < size / 2; i++) {
        u16 dat = datRx();
        *p++ = dat >> 8;
        *p++ = dat >> 0;
    }
}

static void sdDatTxBuf(const void *buf, size_t size) {
    const u8 *p = buf;

    spiNclk(4);

    for (size_t i = 0; i < size / 2; i++) {
        u16 dat = 0;
        dat = (dat << 8) | *p++;
        dat = (dat << 8) | *p++;
        datTx(dat);
    }
}

static void sdDatTxClk(s32 dat, size_t nClk) {
    dat = dat & 0xF;
    dat = (dat << 4) | dat;
    dat = (dat << 8) | dat;

    spiNclk(4);

    for (size_t i = 0; i < nClk / 4; i++) {
        datTx(dat);
    }
}

static s32 sdRxMblk(void *buf, size_t blkSize, size_t nBlk) {
    const u32 cartAddr = 0xB2000000;

    /* dma to cart */
    regWr(REG_DMA_ADDR, cartAddr);
    regWr(REG_DMA_LEN, nBlk);
    while (regRd(REG_DMA_STA) & DMA_STA_BUSY) {
        ;
    }

    /* check for dma timeout */
    if (regRd(REG_DMA_STA) & DMA_STA_ERROR) {
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
    regWr(REG_KEY, 0xAA55);

    /* check magic number */
    if ((regRd(REG_EDID) >> 16) != 0xED64) {
        goto nodev;
    }

    cartUnlock();
    return 0;

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
    if ((regRd(REG_USB_CFG) & (USB_STA_PWR | USB_STA_RXF)) == USB_STA_PWR) {
        ret = 1;
    } else {
        ret = 0;
    }
    cartUnlock();

    return ret;
}

static s32 fifoRead(void *dst, size_t nBlocks) {
    const size_t blkSize = 512;

    cartLock();

    char *p = dst;
    while (nBlocks != 0) {
        /* wait for power on and rx buffer full (PWR high, RXF low) */
        while ((regRd(REG_USB_CFG) & (USB_STA_PWR | USB_STA_RXF)) != USB_STA_PWR) {
            ;
        }

        /* receive */
        regWr(REG_USB_CFG, USB_LE_CFG | USB_LE_CTR | USB_CFG_RD | USB_CFG_ACT);
        while (regRd(REG_USB_CFG) & USB_STA_ACT) {
            ;
        }

        /* copy from rx buffer */
        regWr(REG_USB_CFG, USB_LE_CFG | USB_LE_CTR | USB_CFG_RD);
        piReadLocked((u32)&REGS_PTR[REG_USB_DAT], p, blkSize);

        p += blkSize;
        nBlocks--;
    }

    cartUnlock();
    return 0;
}

static s32 fifoWrite(const void *src, size_t nBlocks) {
    const size_t blkSize = 512;

    cartLock();

    const char *p = src;
    while (nBlocks != 0) {
        /* wait for power on and tx buffer empty (PWR high, TXE low) */
        while ((regRd(REG_USB_CFG) & (USB_STA_PWR | USB_STA_TXE)) != USB_STA_PWR) {
            ;
        }

        /* copy to tx buffer */
        regWr(REG_USB_CFG, USB_LE_CFG | USB_LE_CTR | USB_CFG_WR);
        piWriteLocked((u32)&REGS_PTR[REG_USB_DAT], p, blkSize);

        /* transmit */
        regWr(REG_USB_CFG, USB_LE_CFG | USB_LE_CTR | USB_CFG_WR | USB_CFG_ACT);
        while (regRd(REG_USB_CFG) & USB_STA_ACT) {
            ;
        }

        p += blkSize;
        nBlocks--;
    }

    cartUnlock();
    return 0;
}

struct Iodev everdrive64X = {
    .probe = probe,

    .diskInit = diskInit,
    .diskRead = diskRead,
    .diskWrite = diskWrite,

    .fifoPoll = fifoPoll,
    .fifoRead = fifoRead,
    .fifoWrite = fifoWrite,
};
