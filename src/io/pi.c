#include "pi.h"
#include "util/util.h"

typedef void IoFunc(u32 devAddr, u32 ramAddr, size_t size);

static void pioWrite(u32 devAddr, u32 ramAddr, size_t size) {
    if (size == 0) {
        return;
    }

    u32 devS = devAddr & ~0x3;
    u32 devE = (devAddr + size + 0x3) & ~0x3;
    u32 devP = devS;

    u32 ramS = ramAddr;
    u32 ramE = ramS + size;
    u32 ramP = ramAddr - (devAddr - devS);

    while (devP < devE) {
        u32 w = __piReadRaw(devP);
        for (s32 i = 0; i < 4; i++) {
            u8 b;
            if (ramP >= ramS && ramP < ramE) {
                b = *(u8 *)ramP;
            } else {
                b = w >> 24;
            }
            w = (w << 8) | b;
            ramP++;
        }
        __piWriteRaw(devP, w);
        devP += 4;
    }
}

static void pioRead(u32 devAddr, u32 ramAddr, size_t size) {
    if (size == 0) {
        return;
    }

    u32 devS = devAddr & ~0x3;
    u32 devE = (devAddr + size + 0x3) & ~0x3;
    u32 devP = devS;

    u32 ramS = ramAddr;
    u32 ramE = ramS + size;
    u32 ramP = ramAddr - (devAddr - devS);

    while (devP < devE) {
        u32 w = __piReadRaw(devP);
        for (s32 i = 0; i < 4; i++) {
            if (ramP >= ramS && ramP < ramE) {
                *(u8 *)ramP = w >> 24;
            }
            w <<= 8;
            ramP++;
        }
        devP += 4;
    }
}

static void dmaWrite(u32 devAddr, u32 ramAddr, size_t size) {
    if (size == 0) {
        return;
    }

    OSMesgQueue mq;
    OSMesg m;
    __OSEventState pi_event;

    s32 irqf = getIrqf();
    if (irqf) {
        osCreateMesgQueue(&mq, &m, 1);

        pi_event = __osEventStateTab[OS_EVENT_PI];
        __osEventStateTab[OS_EVENT_PI].messageQueue = &mq;
    }

    dcacheWb((void *)ramAddr, size);
    pi_regs.dram_addr = ramAddr & 0x1FFFFFFF;
    pi_regs.cart_addr = devAddr & 0x1FFFFFFF;
    pi_regs.rd_len = size - 1;

    if (irqf) {
        osRecvMesg(&mq, NULL, OS_MESG_BLOCK);

        __osEventStateTab[OS_EVENT_PI] = pi_event;
    } else {
        __piWait();
        pi_regs.status = PI_STATUS_CLR_INTR;
    }
}

static void dmaRead(u32 devAddr, u32 ramAddr, size_t size) {
    if (size == 0) {
        return;
    }

    OSMesgQueue mq;
    OSMesg m;
    __OSEventState pi_event;

    s32 irqf = getIrqf();
    if (irqf) {
        osCreateMesgQueue(&mq, &m, 1);

        pi_event = __osEventStateTab[OS_EVENT_PI];
        __osEventStateTab[OS_EVENT_PI].messageQueue = &mq;
    }

    dcacheWbinv((void *)ramAddr, size);
    pi_regs.dram_addr = ramAddr & 0x1FFFFFFF;
    pi_regs.cart_addr = devAddr & 0x1FFFFFFF;
    pi_regs.wr_len = size - 1;

    if (irqf) {
        osRecvMesg(&mq, NULL, OS_MESG_BLOCK);

        __osEventStateTab[OS_EVENT_PI] = pi_event;
    } else {
        __piWait();
        pi_regs.status = PI_STATUS_CLR_INTR;
    }
}

static void doTransfer(u32 devAddr, u32 ramAddr, size_t size, IoFunc *pioFunc, IoFunc *dmaFunc) {
    if ((devAddr ^ ramAddr) & 1) {
        /* Impossible alignment for DMA transfer,
         * we have to PIO the whole thing.
         */
        pioFunc(devAddr, ramAddr, size);
    } else {
        u32 ramS = ramAddr;
        u32 ramE = ramAddr + size;
        u32 ramAlignS = (ramS + 0x7) & ~0x7;
        u32 devS = devAddr;

        if (ramE > ramAlignS) {
            u32 ramAlignE = ramE & ~0x1;
            size_t pioS = ramAlignS - ramS;
            size_t pioE = ramE - ramAlignE;
            size_t dma = size - pioS - pioE;
            u32 devE = devAddr + size;
            u32 devAlignS = devS + pioS;
            u32 devAlignE = devE - pioE;

            pioFunc(devS, ramS, pioS);
            pioFunc(devAlignE, ramAlignE, pioE);
            dmaFunc(devAlignS, ramAlignS, dma);
        } else {
            pioFunc(devS, ramS, size);
        }
    }
}

void piWriteLocked(u32 devAddr, const void *src, size_t size) {
    doTransfer(devAddr, (u32)src, size, pioWrite, dmaWrite);
}

void piReadLocked(u32 devAddr, void *dst, size_t size) {
    doTransfer(devAddr, (u32)dst, size, pioRead, dmaRead);
}

void piWrite(u32 devAddr, const void *src, size_t size) {
    __osPiGetAccess();
    piWriteLocked(devAddr, src, size);
    __osPiRelAccess();
}

void piRead(u32 devAddr, void *dst, size_t size) {
    __osPiGetAccess();
    piReadLocked(devAddr, dst, size);
    __osPiRelAccess();
}
