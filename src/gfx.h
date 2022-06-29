#ifndef GFX_H
#define GFX_H
#include <stddef.h>
#include <n64.h>
#include "gu.h"
#include "pm64.h"

#define GFX_FILE_DRAM (-1)
#define gfx_disp(...)                                    \
    {                                                    \
        Gfx gfx_disp__[] = {__VA_ARGS__};                \
        gfx_disp_append(gfx_disp__, sizeof(gfx_disp__)); \
    }
#define GFX_TEXT_NORMAL 0
#define GFX_TEXT_FAST   1

/// X.10 fixed-point literal
#define X10(f)          (s32)(f * 1024.0f)

enum gfx_mode {
    GFX_MODE_FILTER,
    GFX_MODE_COMBINE,
    GFX_MODE_COLOR,
    GFX_MODE_DROPSHADOW,
    GFX_MODE_TEXT,
    GFX_MODE_ALL,
};

struct gfx_texdesc {
    g_ifmt_t im_fmt;
    g_isiz_t im_siz;
    u32 address;
    s16 tile_width;
    s16 tile_height;
    s16 tiles_x;
    s16 tiles_y;
    u32 file_vaddr;
    size_t file_vsize;
};

struct gfx_texldr {
    u32 file_vaddr;
    void *file_data;
};

struct gfx_texture {
    g_ifmt_t im_fmt;
    g_isiz_t im_siz;
    void *data;
    s16 tile_width;
    s16 tile_height;
    s16 tiles_x;
    s16 tiles_y;
    size_t tile_size;
};

struct gfx_sprite {
    struct gfx_texture *texture;
    s16 texture_tile;
    f32 x;
    f32 y;
    f32 xscale;
    f32 yscale;
};

struct gfx_font {
    struct gfx_texture *texture;
    s16 char_width;
    s16 char_height;
    s16 chars_xtile;
    s16 chars_ytile;
    u8 code_start;
    s16 letter_spacing;
    s16 line_spacing;
    s16 baseline;
    s16 median;
    s16 x;
};

void gfx_start(void);
void gfx_mode_init(void);
void gfx_mode_configure(enum gfx_mode mode, u64 value);
void gfx_mode_apply(enum gfx_mode mode);
void gfx_mode_set(enum gfx_mode mode, u64 value);
void gfx_mode_push(enum gfx_mode mode);
void gfx_mode_pop(enum gfx_mode mode);
void gfx_mode_replace(enum gfx_mode mode, u64 value);
/* all sizes are specified in number of bytes */
Gfx *gfx_disp_append(Gfx *disp, size_t size);
void *gfx_data_append(void *data, size_t size);
void gfx_flush(void);

void gfx_texldr_init(struct gfx_texldr *texldr);
struct gfx_texture *gfx_texldr_load(struct gfx_texldr *texldr, const struct gfx_texdesc *texdesc,
                                    struct gfx_texture *texture);
void gfx_texldr_destroy(struct gfx_texldr *texldr);

struct gfx_texture *gfx_texture_create(g_ifmt_t im_fmt, g_isiz_t im_siz, s32 tile_width, s32 tile_height, s32 tiles_x,
                                       s32 tiles_y);
struct gfx_texture *gfx_texture_load(const struct gfx_texdesc *texdesc, struct gfx_texture *texture);
void gfx_texture_destroy(struct gfx_texture *texture);
void gfx_texture_free(struct gfx_texture *texture);
void *gfx_texture_data(const struct gfx_texture *texture, s16 image);
struct gfx_texture *gfx_texture_copy(const struct gfx_texture *src, struct gfx_texture *dest);
void gfx_texture_copy_tile(struct gfx_texture *dest, s32 dest_tile, const struct gfx_texture *src, s32 src_tile,
                           _Bool blend);
void gfx_texture_colortransform(struct gfx_texture *texture, const MtxF *matrix);

void gfx_disp_rdp_load_tile(Gfx **disp, const struct gfx_texture *texture, s16 texture_tile);
void gfx_rdp_load_tile(const struct gfx_texture *texture, s16 texture_tile);

void gfx_sprite_draw(const struct gfx_sprite *sprite);

s32 gfx_font_xheight(const struct gfx_font *font);
void gfx_printf(const struct gfx_font *font, s32 x, s32 y, const char *format, ...);
void gfx_printf_n(const struct gfx_font *font, s32 x, s32 y, const char *format, ...);
void gfx_printf_f(const struct gfx_font *font, s32 x, s32 y, const char *format, ...);

extern const MtxF gfx_cm_desaturate;

void draw_hud_element(HudElement *elem);

#endif
