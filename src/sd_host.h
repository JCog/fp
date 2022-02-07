#ifndef SD_HOST_H
#define SD_HOST_H
#include <stddef.h>
#include <stdint.h>

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

struct sd_host {
    s32 proto;

    void (*lock)(void);
    void (*unlock)(void);
    void (*set_spd)(s32 spd);
    union {
        struct {
            void (*spi_ss)(s32 ss);
            s32 (*spi_io)(s32 dat);
            void (*spi_rx_buf)(void *buf, size_t size);
            void (*spi_tx_buf)(const void *buf, size_t size);
            void (*spi_tx_clk)(s32 dat, size_t n_clk);
        };
        struct {
            s32 (*cmd_rx)(void);
            s32 (*dat_rx)(void);
            void (*dat_tx)(s32 dat);
            void (*cmd_rx_buf)(void *buf, size_t size);
            void (*cmd_tx_buf)(const void *buf, size_t size);
            void (*dat_rx_buf)(void *buf, size_t size);
            void (*dat_tx_buf)(const void *buf, size_t size);
            void (*dat_tx_clk)(s32 dat, size_t n_clk);
        };
    };
    s32 (*rx_mblk)(void *buf, size_t blk_size, size_t n_blk);
    s32 (*tx_mblk)(const void *buf, size_t blk_size, size_t n_blk);

    s32 card_type;
};

s32 sd_init(struct sd_host *host);
s32 sd_read(struct sd_host *host, size_t lba, void *dst, size_t n_blk);
s32 sd_write(struct sd_host *host, size_t lba, const void *src, size_t n_blk);

#endif
