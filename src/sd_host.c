#include "sd_host.h"
#include "io.h"
#include "sd.h"
#include <stddef.h>

static u8 crc7(void *data, s32 size) {
    u8 *p = data;
    u8 crc = 0;

    while (size != 0) {
        crc = crc ^ *p++;
        for (s32 i = 0; i < 8; i++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x12;
            } else {
                crc = (crc << 1);
            }
        }
        size--;
    }

    return crc | 1;
}

static void crc16Wide(const void *data, s32 size, u16 (*crcBuf)[4]) {
#if defined(__mips) && __mips >= 3
    s32 crc;
    s32 t0;
    s32 t1;

    __asm__(
        "  dli     %[crc], 0;"
        "  beq     %[size], $zero, 1f;"

        "  addu    %[size], %[data], %[size];"
        "0:"
        "  lbu     %[t1], 0(%[data]);"
        "  lbu     %[t0], 1(%[data]);"
        "  sll     %[t1], %[t1], 8;"
        "  or      %[t0], %[t0], %[t1];"

        "  dsrl32  %[t1], %[crc], 16;"
        "  xor     %[t1], %[t1], %[t0];"

        "  dsll    %[t0], %[t1], 28;"
        "  daddu   %[t0], %[t0], %[t1];"
        "  dsll    %[t0], %[t0], 20;"
        "  daddu   %[t0], %[t0], %[t1];"
        "  dsll    %[crc], %[crc], 16;"

        "  addiu   %[data], %[data], 2;"
        "  xor     %[crc], %[crc], %[t0];"

        "  bne     %[data], %[size], 0b;"
        "1:"

        /* big endian byte order is assumed */
        "  sh      %[crc], 6 + %[crc_buf];"
        "  dsrl    %[crc], %[crc], 16;"
        "  sh      %[crc], 4 + %[crc_buf];"
        "  dsrl    %[crc], %[crc], 16;"
        "  sh      %[crc], 2 + %[crc_buf];"
        "  dsrl    %[crc], %[crc], 16;"
        "  sh      %[crc], 0 + %[crc_buf];"

        : [data] "+r"(data), [size] "+r"(size), [crc_buf] "=m"(*crcBuf), [crc] "=r"(crc), [t0] "=r"(t0), [t1] "=r"(t1));
#else
    /* CRC-16 CCITT/XMODEM (0x1021) stretched to 64 bits */
    const u64 poly = 0x0001000000100001;
    u64 crc = 0;
    u64 x;
    const u8 *p = data;

    for (s32 i = 0; i < size; i += 2) {
        x = (p[i] << 8) | p[i + 1];
        x = (crc >> 48) ^ x;
        x = x * poly;
        crc = (crc << 16) ^ x;
    }

    /* store result in big endian byte order */
    u8 *crcP = (void *)*crcBuf;
    for (s32 i = 0; i < 8; i++) {
        *crcP++ = crc >> 56;
        crc = crc << 8;
    }
#endif
}

static s32 csErr(u32 cs) {
    if ((cs & CS_ERR_BITS) == 0) {
        return 0;
    } else if (cs & CS_CC_ERROR) {
        return -SD_ERR_CC;
    } else if (cs & CS_CARD_ECC_FAILED) {
        return -SD_ERR_ECC;
    } else if (cs & CS_ILLEGAL_COMMAND) {
        return -SD_ERR_ILCMD;
    } else if (cs & CS_COM_CRC_ERROR) {
        return -SD_ERR_CRC;
    } else if (cs & (CS_WP_ERASE_SKIP | CS_CSD_OVERWRITE | CS_LOCK_UNLOCK_FAILED | CS_WP_VIOLATION)) {
        return -SD_ERR_WPVIOL;
    } else if (cs & CS_ERASE_SEQ_ERROR) {
        return -SD_ERR_ERSEQ;
    } else if (cs & (CS_BLOCK_LEN_ERROR | CS_ERASE_PARAM)) {
        return -SD_ERR_PARAM;
    } else if (cs & CS_ADDRESS_ERROR) {
        return -SD_ERR_ADDR;
    } else if (cs & CS_OUT_OF_RANGE) {
        return -SD_ERR_RANGE;
    } else {
        return -SD_ERR_GEN;
    }
}

static s32 wrTokErr(s32 tok) {
    if (tok == DAT_RESP_OK) {
        return 0;
    } else if (tok == DAT_RESP_CRC_ERR) {
        return -SD_ERR_CRC;
    } else if (tok == DAT_RESP_WR_ERR) {
        return -SD_ERR_CC;
    } else {
        return -SD_ERR_GEN;
    }
}

static s32 spiR1Err(s32 r1) {
    if ((r1 & SPI_R1_ERR_BITS) == 0) {
        return 0;
    } else if (r1 & SPI_R1_ILLEGAL_CMD) {
        return -SD_ERR_ILCMD;
    } else if (r1 & SPI_R1_CRC_ERR) {
        return -SD_ERR_CRC;
    } else if (r1 & SPI_R1_ERASE_SEQ_ERR) {
        return -SD_ERR_ERSEQ;
    } else if (r1 & SPI_R1_ADDR_ERR) {
        return -SD_ERR_ADDR;
    } else if (r1 & SPI_R1_PARAM_ERR) {
        return -SD_ERR_PARAM;
    } else {
        return -SD_ERR_GEN;
    }
}

static s32 spiRdTokErr(s32 tok) {
    if (tok & SPI_BLK_RANGE_ERR) {
        return -SD_ERR_RANGE;
    } else if (tok & SPI_BLK_ECC_ERR) {
        return -SD_ERR_ECC;
    } else if (tok & SPI_BLK_CC_ERR) {
        return -SD_ERR_CC;
    } else {
        return -SD_ERR_GEN;
    }
}

static void setSpd(struct SdHost *host, s32 spd) {
    /* provide an 8 clock period before switching */
    if (host->proto == SD_PROTO_SDBUS) {
        host->datTxClk(0xF, 8);
    } else { /* host->proto == SD_PROTO_SPIBUS */
        host->spiIo(0xFF);
    }

    host->setSpd(spd);
}

static s32 cardCmdSd(struct SdHost *host, s32 cmd, const u8 *txBuf, u8 *rxBuf) {
    s32 respType = sdRespType(cmd);
    s32 respSize = sdRespSize(respType);

    /* send command */
    host->cmdTxBuf(txBuf, 7);

    if (respSize != 0) {
        /* wait for response */
        rxBuf[0] = 0xFF;
        for (s32 i = 0;; i++) {
            if (i > 64) {
                return -SD_ERR_TIMEOUT;
            }

            rxBuf[0] = (rxBuf[0] << 1) | host->cmdRx();

            /* start bit, direction bit card -> host */
            if ((rxBuf[0] & 0xC0) == 0x00) {
                break;
            }
        }

        /* receive response */
        host->cmdRxBuf(&rxBuf[1], respSize - 1);
    }

    if (respType != 0 && respType != R3) {
        /* verify response crc */
        u8 crc;
        if (respType == R2) {
            crc = crc7(&rxBuf[1], respSize - 2);
        } else {
            crc = crc7(&rxBuf[0], respSize - 1);
        }

        if (rxBuf[respSize - 1] != crc) {
            return -SD_ERR_CRC;
        }
    }

    /* check respone status */
    if (respType == R1) {
        return csErr(sdR1Cs(rxBuf));
    } else {
        return 0;
    }
}

static s32 cardCmdSpi(struct SdHost *host, s32 cmd, const u8 *txBuf, u8 *rxBuf) {
    s32 respType = spiRespType(cmd);
    s32 respSize = spiRespSize(respType);

    /* send command */
    host->spiTxBuf(txBuf, 7);

    if (cmd == STOP_TRANSMISSION) {
        /* send 8 clocks to allow the transmission to end */
        host->spiIo(0xFF);
    }

    if (respSize != 0) {
        /* wait for response */
        for (s32 i = 0;; i++) {
            if (i > 8) {
                return -SD_ERR_TIMEOUT;
            }

            rxBuf[0] = host->spiIo(0xFF);
            if ((rxBuf[0] & SPI_R1_ZERO) == 0) {
                break;
            }
        }

        /* receive response */
        host->spiRxBuf(&rxBuf[1], respSize - 1);
    }

    /* check respone status */
    if (respType != 0) {
        return spiR1Err(rxBuf[0]);
    } else {
        return 0;
    }
}

static s32 cardCmd(struct SdHost *host, s32 cmd, u32 arg, void *resp) {
    u8 txBuf[17];
    u8 *rxBuf;

    if (resp != NULL) {
        rxBuf = resp;
    } else {
        rxBuf = txBuf;
    }

    /* 8 clocks to allow the previous operation to complete */
    txBuf[0] = 0xFF;
    /* start bit, direction bit host -> card, cmd */
    txBuf[1] = 0x40 | cmd;
    /* argument */
    txBuf[2] = arg >> 24;
    txBuf[3] = arg >> 16;
    txBuf[4] = arg >> 8;
    txBuf[5] = arg >> 0;
    /* crc, end bit (tacked on by crc7) */
    txBuf[6] = crc7(&txBuf[1], 5);

    if (host->proto == SD_PROTO_SDBUS) {
        return cardCmdSd(host, cmd, txBuf, rxBuf);
    } else { /* host->proto == SD_PROTO_SPIBUS */
        return cardCmdSpi(host, cmd, txBuf, rxBuf);
    }
}

static s32 rxBlkSd(struct SdHost *host, void *buf, size_t blkSize) {
    /* wait for start bit on DAT0-DAT3 */
    unsigned timeout = msecFromNow(100);
    while (host->datRx() != 0x0) {
        if (clockAfter(timeout)) {
            return -SD_ERR_TIMEOUT;
        }
    }

    /* receive data block */
    host->datRxBuf(buf, blkSize);

    /* receive crc */
    u16 rxCrc[4];
    host->datRxBuf(rxCrc, sizeof(rxCrc));

    /* compute crc */
    u16 crc[4];
    crc16Wide(buf, blkSize, &crc);

    /* verify crc */
    for (s32 i = 0; i < 4; i++) {
        if (rxCrc[i] != crc[i]) {
            return -SD_ERR_CRC;
        }
    }

    return 0;
}

static s32 rxBlkSpi(struct SdHost *host, void *buf, size_t blkSize) {
    /* wait for data block token */
    unsigned timeout = msecFromNow(100);
    for (;;) {
        s32 tok = host->spiIo(0xFF);

        if (tok == SPI_BLK_START) {
            break;
        } else if ((tok & 0xF0) == 0x00) {
            return spiRdTokErr(tok);
        }

        if (clockAfter(timeout)) {
            return -SD_ERR_TIMEOUT;
        }
    }

    /* receive data block */
    host->spiRxBuf(buf, blkSize);

    /* receive crc */
    u16 rxCrc;
    host->spiRxBuf(&rxCrc, sizeof(rxCrc));

    /* skip crc verification for SPI Bus */
    return 0;
}

static s32 rxBlk(struct SdHost *host, void *buf, size_t blkSize) {
    if (host->proto == SD_PROTO_SDBUS) {
        return rxBlkSd(host, buf, blkSize);
    } else { /* host->proto == SD_PROTO_SPIBUS */
        return rxBlkSpi(host, buf, blkSize);
    }
}

static s32 txBlkSd(struct SdHost *host, const void *buf, size_t blkSize) {
    /* compute crc */
    u16 crc[4];
    if (buf != NULL) {
        crc16Wide(buf, blkSize, &crc);
    } else {
        crc[0] = crc[1] = crc[2] = crc[3] = 0;
    }

    /* wait for busy signal on DAT0-DAT3 to be released */
    unsigned timeout = msecFromNow(250);
    while (host->datRx() != 0xF) {
        if (clockAfter(timeout)) {
            return -SD_ERR_TIMEOUT;
        }
    }

    /* transmit start bit on DAT0-DAT3 */
    host->datTx(0xF);
    host->datTx(0x0);
    /* at least one card seems to need the start and end tokens to be on
     * byte boundaries, hence the two clocks */

    /* transmit data */
    if (buf != NULL) {
        host->datTxBuf(buf, blkSize);
    } else {
        host->datTxClk(0x0, blkSize * 2);
    }

    /* transmit crc */
    host->datTxBuf(crc, sizeof(crc));

    /* transmit end bit on DAT0-DAT3 */
    host->datTx(0xF);
    host->datTx(0xF);

    /* wait for start bit on DAT0 */
    for (s32 i = 0;; i++) {
        if (i > 64) {
            return -SD_ERR_TIMEOUT;
        }

        if ((host->datRx() & 1) == 0) {
            break;
        }
    }

    /* receive response token on DAT0 */
    s32 tok = 0;
    for (s32 i = 0; i < 4; i++) {
        tok = (tok << 1) | (host->datRx() & 1);
    }

    /* check response token */
    return wrTokErr(tok);
}

static s32 txBlkSpi(struct SdHost *host, const void *buf, size_t blkSize) {
    s32 tok;

    /* wait for free receive buffer */
    unsigned timeout = msecFromNow(250);
    while (host->spiIo(0xFF) != 0xFF) {
        if (clockAfter(timeout)) {
            return -SD_ERR_TIMEOUT;
        }
    }

    /* transmit start block token */
    host->spiIo(SPI_MBW_START);

    /* transmit data */
    if (buf != NULL) {
        host->spiTxBuf(buf, blkSize);
    } else {
        host->spiTxClk(0, blkSize * 8);
    }

    /* transmit bogus crc (ignored by card) */
    host->spiIo(0xFF);
    host->spiIo(0xFF);

    /* receive and check data response token */
    for (s32 i = 0;; i++) {
        if (i > 8) {
            return -SD_ERR_TIMEOUT;
        }

        tok = host->spiIo(0xFF);

        if ((tok & 0x11) == 0x01) {
            return wrTokErr(tok & 0x1F);
        }
    }
}

static s32 txBlk(struct SdHost *host, const void *buf, size_t blkSize) {
    if (host->proto == SD_PROTO_SDBUS) {
        return txBlkSd(host, buf, blkSize);
    } else { /* host->proto == SD_PROTO_SPIBUS */
        return txBlkSpi(host, buf, blkSize);
    }
}

static s32 stopRd(struct SdHost *host) {
    return cardCmd(host, STOP_TRANSMISSION, 0, NULL);
}

static s32 stopWrSd(struct SdHost *host) {
    s32 ret = cardCmd(host, STOP_TRANSMISSION, 0, NULL);
    if (ret != 0) {
        return ret;
    }

    /* wait for busy signal on DAT0-DAT3 to be released */
    unsigned timeout = msecFromNow(500);
    while (host->datRx() != 0xF) {
        if (clockAfter(timeout)) {
            return -SD_ERR_TIMEOUT;
        }
    }

    return 0;
}

static s32 stopWrSpi(struct SdHost *host) {
    unsigned timeout;

    /* wait for busy signal to be released */
    timeout = msecFromNow(250);
    while (host->spiIo(0xFF) != 0xFF) {
        if (clockAfter(timeout)) {
            return -SD_ERR_TIMEOUT;
        }
    }

    /* send stop transmission token */
    host->spiIo(SPI_MBW_STOP);

    /* send 8 clocks to allow the transmission to end */
    host->spiIo(0xFF);

    /* and wait again */
    timeout = msecFromNow(500);
    while (host->spiIo(0xFF) != 0xFF) {
        if (clockAfter(timeout)) {
            return -SD_ERR_TIMEOUT;
        }
    }

    return 0;
}

static s32 stopWr(struct SdHost *host) {
    if (host->proto == SD_PROTO_SDBUS) {
        return stopWrSd(host);
    } else { /* host->proto == SD_PROTO_SPIBUS */
        return stopWrSpi(host);
    }
}

static s32 rxMblk(struct SdHost *host, void *buf, size_t blkSize, size_t nBlk) {
    s32 ret;

    if (host->rxMblk) {
        ret = host->rxMblk(buf, blkSize, nBlk);
    } else {
        char *p = buf;

        for (size_t i = 0; i < nBlk; i++) {
            ret = rxBlk(host, p, blkSize);
            if (ret != 0) {
                break;
            }

            if (p != NULL) {
                p += blkSize;
            }
        }
    }

    return ret;
}

static s32 txMblk(struct SdHost *host, const void *buf, size_t blkSize, size_t nBlk) {
    s32 ret;

    const char *p = buf;

    for (size_t i = 0; i < nBlk; i++) {
        ret = txBlk(host, p, blkSize);
        if (ret != 0) {
            break;
        }

        if (p != NULL) {
            p += blkSize;
        }
    }

    return ret;
}

s32 sdInit(struct SdHost *host) {
    s32 ret;
    u8 dat[64];

    /* acquire the host device */
    host->lock();

    /* reset card */
    host->cardType = 0;
    setSpd(host, 0);
    /* provide 80 clocks for card initialization */
    if (host->proto == SD_PROTO_SDBUS) {
        host->datTxClk(0xF, 80);
    } else { /* host->proto == SD_PROTO_SPIBUS */
        host->spiSs(0);
        host->spiTxClk(0x1, 80);
        host->spiSs(1);
    }
    ret = cardCmd(host, GO_IDLE_STATE, 0, NULL);
    if (ret != 0) {
        goto exit;
    }

    /* 2.7V - 3.6V, check pattern 0b10101010 */
    ret = cardCmd(host, SEND_IF_COND, IF_COND_VHS_VDD1 | 0xAA, NULL);
    if (ret == 0) {
        host->cardType |= SD_CARD_V2;
    } else if (ret != -SD_ERR_ILCMD) {
        goto exit;
    }

    unsigned timeout = msecFromNow(1000);
    for (;;) {
        ret = cardCmd(host, APP_CMD, 0, NULL);
        if (ret != 0) {
            goto exit;
        }

        /* 3.2V - 3.4V */
        u32 opCond = OCR_3V2_3V3 | OCR_3V3_3V4;
        if (host->cardType & SD_CARD_V2) {
            /* Host Capacity Support */
            opCond = opCond | OCR_CCS;
        }
        ret = cardCmd(host, SD_SEND_OP_COND, opCond, dat);
        if (ret != 0) {
            goto exit;
        }

        if (host->proto == SD_PROTO_SDBUS) {
            u32 ocr = r3Ocr(dat);
            /* check card power up status bit */
            if (ocr & OCR_BUSY) {
                if (host->cardType & SD_CARD_V2) {
                    /* check Card Capacity Status */
                    if (ocr & OCR_CCS) {
                        host->cardType |= SD_CARD_HC;
                    }
                }
                break;
            }
        } else { /* host->proto == SD_PROTO_SPIBUS */
            /* check in_idle_state */
            if ((dat[0] & SPI_R1_IN_IDLE_STATE) == 0) {
                break;
            }
        }

        if (clockAfter(timeout)) {
            ret = -SD_ERR_TIMEOUT;
            goto exit;
        }
    }

    /* switch to Default Speed (25MHz) */
    setSpd(host, 25);

    if (host->proto == SD_PROTO_SDBUS) {
        /* get rca and select card */
        ret = cardCmd(host, ALL_SEND_CID, 0, NULL);
        if (ret != 0) {
            goto exit;
        }
        ret = cardCmd(host, SEND_RELATIVE_ADDR, 0, dat);
        if (ret != 0) {
            goto exit;
        }
        u32 rca = r6Rca(dat);
        ret = cardCmd(host, SELECT_CARD, rca << 16, NULL);
        if (ret != 0) {
            goto exit;
        }

        /* select wide (4 bit) data bus */
        ret = cardCmd(host, APP_CMD, rca << 16, NULL);
        if (ret != 0) {
            goto exit;
        }
        ret = cardCmd(host, SET_BUS_WIDTH, 2, NULL);
        if (ret != 0) {
            goto exit;
        }
    } else { /* host->proto == SD_PROTO_SPIBUS */
        if (host->cardType & SD_CARD_V2) {
            /* check Card Capacity Status */
            ret = cardCmd(host, READ_OCR, 0, dat);
            if (ret != 0) {
                goto exit;
            }

            if (r3Ocr(dat) & OCR_CCS) {
                host->cardType |= SD_CARD_HC;
            }
        }
    }

    /* try to switch to High Speed (50MHz) */
    ret = cardCmd(host, SWITCH_FUNC, SWFN_SET | 0xFFFFF1, NULL);
    if (ret == 0) {
        ret = rxBlk(host, dat, SWFN_DAT_SIZE);
        if (ret != 0) {
            goto exit;
        }

        if (swfnSel(dat, 1) == 1) {
            setSpd(host, 50);
        }
    } else if (ret != -SD_ERR_ILCMD) {
        goto exit;
    }

    ret = 0;

exit:
    /* release the host device */
    host->unlock();

    return ret;
}

s32 sdRead(struct SdHost *host, size_t lba, void *dst, size_t nBlk) {
    s32 ret;
    const size_t blkSize = 512;

    /* acquire the host device */
    host->lock();

    /* send read command */
    if (host->cardType & SD_CARD_HC) {
        ret = cardCmd(host, READ_MULTIPLE_BLOCK, lba, NULL);
    } else {
        ret = cardCmd(host, READ_MULTIPLE_BLOCK, lba * blkSize, NULL);
    }
    if (ret != 0) {
        goto exit;
    }

    /* receive blocks */
    ret = rxMblk(host, dst, blkSize, nBlk);

    /* stop transmission */
    if (ret == 0) {
        ret = stopRd(host);
    } else if (ret != -SD_ERR_TIMEOUT) {
        stopRd(host);
    }

exit:
    /* release the host device */
    host->unlock();

    return ret;
}

s32 sdWrite(struct SdHost *host, size_t lba, const void *src, size_t nBlk) {
    s32 ret;
    const size_t blkSize = 512;

    /* acquire the host device */
    host->lock();

    /* send write command */
    if (host->cardType & SD_CARD_HC) {
        ret = cardCmd(host, WRITE_MULTIPLE_BLOCK, lba, NULL);
    } else {
        ret = cardCmd(host, WRITE_MULTIPLE_BLOCK, lba * blkSize, NULL);
    }
    if (ret != 0) {
        goto exit;
    }

    /* transmit blocks */
    ret = txMblk(host, src, blkSize, nBlk);

    /* stop transmission */
    if (ret == 0) {
        ret = stopWr(host);
    } else if (ret != -SD_ERR_TIMEOUT) {
        stopWr(host);
    }

exit:
    /* release the host device */
    host->unlock();

    return ret;
}
