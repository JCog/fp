#ifndef SD_HOST_H
#define SD_HOST_H
#include "common.h"

#define SD_PROTO_SDBUS  0
#define SD_PROTO_SPIBUS 1

#define SD_CARD_V2      1
#define SD_CARD_HC      2

#define SD_ERR_TIMEOUT  1
#define SD_ERR_CRC      2
#define SD_ERR_ILCMD    3
#define SD_ERR_ADDR     4
#define SD_ERR_PARAM    5
#define SD_ERR_ERSEQ    6
#define SD_ERR_WPVIOL   7
#define SD_ERR_ECC      8
#define SD_ERR_CC       9
#define SD_ERR_RANGE    10
#define SD_ERR_GEN      11

struct SdHost {
    s32 proto;

    void (*lock)(void);
    void (*unlock)(void);
    void (*setSpd)(s32 spd);
    union {
        struct {
            void (*spiSs)(s32 ss);
            s32 (*spiIo)(s32 dat);
            void (*spiRxBuf)(void *buf, size_t size);
            void (*spiTxBuf)(const void *buf, size_t size);
            void (*spiTxClk)(s32 dat, size_t nClk);
        };
        struct {
            s32 (*cmdRx)(void);
            s32 (*datRx)(void);
            void (*datTx)(s32 dat);
            void (*cmdRxBuf)(void *buf, size_t size);
            void (*cmdTxBuf)(const void *buf, size_t size);
            void (*datRxBuf)(void *buf, size_t size);
            void (*datTxBuf)(const void *buf, size_t size);
            void (*datTxClk)(s32 dat, size_t nClk);
        };
    };
    s32 (*rxMblk)(void *buf, size_t blkSize, size_t nBlk);
    s32 (*txMblk)(const void *buf, size_t blkSize, size_t nBlk);

    s32 cardType;
};

s32 sdInit(struct SdHost *host);
s32 sdRead(struct SdHost *host, size_t lba, void *dst, size_t nBlk);
s32 sdWrite(struct SdHost *host, size_t lba, const void *src, size_t nBlk);

#endif
