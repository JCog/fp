#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>
#include <mips.h>
#include <n64.h>
#include "gfx.h"
#include "pm64.h"
#include "fp.h"

#define GFX_DISP_SIZE 0x10000
static Gfx *gfx_disp;
static Gfx *gfx_disp_w;
static Gfx *gfx_disp_p;
static Gfx *gfx_disp_d;

#define GFX_STACK_LENGTH 8
static u64 gfx_modes[GFX_MODE_ALL];
static u64 gfx_mode_stack[GFX_MODE_ALL][GFX_STACK_LENGTH];
static s32 gfx_mode_stack_pos[GFX_MODE_ALL];
static _Bool gfx_synced;

#define CHAR_TILE_MAX 8
struct gfx_char {
    s32 tile_char;
    u32 color;
    s32 x;
    s32 y;
};
static const struct gfx_font *gfx_char_font;
static struct vector gfx_chars[CHAR_TILE_MAX];

static void draw_chars(const struct gfx_font *font, s32 x, s32 y, const char *buf, size_t l);
static void flush_chars(void);
static void gfx_printf_n_va(const struct gfx_font *font, s32 x, s32 y, const char *format, va_list args);
static void gfx_printf_f_va(const struct gfx_font *font, s32 x, s32 y, const char *format, va_list args);

static inline void gfx_sync(void) {
    if (!gfx_synced) {
        gDPPipeSync(gfx_disp_p++);
        gfx_synced = 1;
    }
}

const MtxF gfx_cm_desaturate = guDefMtxF(0.3086f, 0.6094f, 0.0820f, 0.f, 0.3086f, 0.6094f, 0.0820f, 0.f, 0.3086f,
                                         0.6094f, 0.0820f, 0.f, 0.f, 0.f, 0.f, 1.f);

void gfx_start(void) {
    for (s32 i = 0; i < CHAR_TILE_MAX; ++i)
        vector_init(&gfx_chars[i], sizeof(struct gfx_char));
    gfx_disp = malloc(GFX_DISP_SIZE);
    gfx_disp_w = malloc(GFX_DISP_SIZE);
    gfx_disp_p = gfx_disp;
    gfx_disp_d = gfx_disp + (GFX_DISP_SIZE + sizeof(*gfx_disp) - 1) / sizeof(*gfx_disp);
}

void gfx_mode_init(void) {
    gfx_sync();
    gSPLoadGeometryMode(gfx_disp_p++, 0);
    gDPSetCycleType(gfx_disp_p++, G_CYC_1CYCLE);
    gDPSetRenderMode(gfx_disp_p++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gDPSetScissor(gfx_disp_p++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    gDPSetAlphaDither(gfx_disp_p++, G_AD_DISABLE);
    gDPSetColorDither(gfx_disp_p++, G_CD_DISABLE);
    gDPSetAlphaCompare(gfx_disp_p++, G_AC_NONE);
    gDPSetDepthSource(gfx_disp_p++, G_ZS_PRIM);
    gDPSetCombineKey(gfx_disp_p++, G_CK_NONE);
    gDPSetTextureConvert(gfx_disp_p++, G_TC_FILT);
    gDPSetTextureDetail(gfx_disp_p++, G_TD_CLAMP);
    gDPSetTexturePersp(gfx_disp_p++, G_TP_NONE);
    gDPSetTextureLOD(gfx_disp_p++, G_TL_TILE);
    gDPSetTextureLUT(gfx_disp_p++, G_TT_NONE);
    gDPPipelineMode(gfx_disp_p++, G_PM_NPRIMITIVE);
    gfx_mode_apply(GFX_MODE_ALL);
}

void gfx_mode_configure(enum gfx_mode mode, u64 value) {
    gfx_modes[mode] = value;
}

void gfx_mode_apply(enum gfx_mode mode) {
    Gfx dl[GFX_MODE_ALL];
    Gfx *pdl = dl;
    switch (mode) {
        case GFX_MODE_ALL:
        case GFX_MODE_FILTER: {
            gDPSetTextureFilter(pdl++, gfx_modes[GFX_MODE_FILTER]);
            if (mode != GFX_MODE_ALL) {
                break;
            }
        }
        case GFX_MODE_COMBINE: {
            gDPSetCombine(pdl++, gfx_modes[GFX_MODE_COMBINE]);
            if (mode != GFX_MODE_ALL) {
                break;
            }
        }
        case GFX_MODE_COLOR: {
            u32 c = gfx_modes[GFX_MODE_COLOR];
            gDPSetPrimColor(pdl++, 0, 0, (c >> 24) & 0xFF, (c >> 16) & 0xFF, (c >> 8) & 0xFF, (c >> 0) & 0xFF);
            if (mode != GFX_MODE_ALL) {
                break;
            }
        }
        default: break;
    }
    size_t s = pdl - dl;
    if (s > 0) {
        gfx_sync();
        memcpy(gfx_disp_p, dl, s * sizeof(*dl));
        gfx_disp_p += s;
    }
}

void gfx_mode_set(enum gfx_mode mode, u64 value) {
    gfx_mode_configure(mode, value);
    gfx_mode_apply(mode);
}

void gfx_mode_push(enum gfx_mode mode) {
    if (mode == GFX_MODE_ALL) {
        for (s32 i = 0; i < GFX_MODE_ALL; ++i) {
            s32 *p = &gfx_mode_stack_pos[i];
            gfx_mode_stack[i][*p] = gfx_modes[i];
            *p = (*p + 1) % GFX_STACK_LENGTH;
        }
    } else {
        s32 *p = &gfx_mode_stack_pos[mode];
        gfx_mode_stack[mode][*p] = gfx_modes[mode];
        *p = (*p + 1) % GFX_STACK_LENGTH;
    }
}

void gfx_mode_pop(enum gfx_mode mode) {
    if (mode == GFX_MODE_ALL) {
        for (s32 i = 0; i < GFX_MODE_ALL; ++i) {
            s32 *p = &gfx_mode_stack_pos[i];
            *p = (*p + GFX_STACK_LENGTH - 1) % GFX_STACK_LENGTH;
            gfx_mode_set(i, gfx_mode_stack[i][*p]);
        }
    } else {
        s32 *p = &gfx_mode_stack_pos[mode];
        *p = (*p + GFX_STACK_LENGTH - 1) % GFX_STACK_LENGTH;
        gfx_mode_set(mode, gfx_mode_stack[mode][*p]);
    }
}

void gfx_mode_replace(enum gfx_mode mode, u64 value) {
    gfx_mode_push(mode);
    gfx_mode_configure(mode, value);
    gfx_mode_apply(mode);
}

Gfx *gfx_disp_append(Gfx *disp, size_t size) {
    Gfx *p = gfx_disp_p;
    memcpy(gfx_disp_p, disp, size);
    gfx_disp_p += (size + sizeof(*gfx_disp_p) - 1) / sizeof(*gfx_disp_p);
    gfx_synced = 0;
    return p;
}

void *gfx_data_append(void *data, size_t size) {
    gfx_disp_d -= (size + sizeof(*gfx_disp_d) - 1) / sizeof(*gfx_disp_d);
    memcpy(gfx_disp_d, data, size);
    return gfx_disp_d;
}

void gfx_flush(void) {
    flush_chars();
    gSPEndDisplayList(gfx_disp_p++);
    gSPDisplayList(pm_MasterGfxPos++, gfx_disp);
    Gfx *disp_w = gfx_disp_w;
    gfx_disp_w = gfx_disp;
    gfx_disp = disp_w;
    gfx_disp_p = gfx_disp;
    gfx_disp_d = gfx_disp + (GFX_DISP_SIZE + sizeof(*gfx_disp) - 1) / sizeof(*gfx_disp);
    gfx_synced = 0;
}

void gfx_texldr_init(struct gfx_texldr *texldr) {
    texldr->file_vaddr = GFX_FILE_DRAM;
    texldr->file_data = NULL;
}

struct gfx_texture *gfx_texldr_load(struct gfx_texldr *texldr, const struct gfx_texdesc *texdesc,
                                    struct gfx_texture *texture) {
    struct gfx_texture *new_texture = NULL;
    if (!texture) {
        new_texture = malloc(sizeof(*new_texture));
        if (!new_texture) {
            return new_texture;
        }
        texture = new_texture;
    }
    texture->im_fmt = texdesc->im_fmt;
    texture->im_siz = texdesc->im_siz;
    texture->tile_width = texdesc->tile_width;
    texture->tile_height = texdesc->tile_height;
    texture->tiles_x = texdesc->tiles_x;
    texture->tiles_y = texdesc->tiles_y;
    texture->tile_size = (texture->tile_width * texture->tile_height * G_SIZ_BITS(texture->im_siz) + 63) / 64 * 8;
    size_t texture_size = texture->tile_size * texture->tiles_x * texture->tiles_y;
    void *texture_data = NULL;
    void *file_start = NULL;
    if (texdesc->file_vaddr != GFX_FILE_DRAM) {
        if (texldr->file_vaddr != texdesc->file_vaddr) {
            if (texldr->file_data) {
                free(texldr->file_data);
            }
            texldr->file_data = memalign(64, texdesc->file_vsize);
            if (!texldr->file_data) {
                texldr->file_vaddr = GFX_FILE_DRAM;
                if (new_texture) {
                    free(new_texture);
                }
                return NULL;
            }
            texldr->file_vaddr = texdesc->file_vaddr;
            /*zu_getfile(texldr->file_vaddr, texldr->file_data, texdesc->file_vsize);*/
        }
        if (texdesc->file_vsize == texture_size) {
            texture_data = texldr->file_data;
            texldr->file_vaddr = GFX_FILE_DRAM;
            texldr->file_data = NULL;
        } else {
            file_start = texldr->file_data;
        }
    }
    if (!texture_data) {
        texture_data = memalign(64, texture_size);
        if (!texture_data) {
            if (new_texture) {
                free(new_texture);
            }
            return NULL;
        }
        memcpy(texture_data, (char *)file_start + texdesc->address, texture_size);
    }
    texture->data = texture_data;
    return texture;
}

void gfx_texldr_destroy(struct gfx_texldr *texldr) {
    if (texldr->file_data) {
        free(texldr->file_data);
    }
}

struct gfx_texture *gfx_texture_create(g_ifmt_t im_fmt, g_isiz_t im_siz, s32 tile_width, s32 tile_height, s32 tiles_x,
                                       s32 tiles_y) {
    struct gfx_texture *texture = malloc(sizeof(*texture));
    if (!texture) {
        return texture;
    }
    texture->tile_size = (tile_width * tile_height * G_SIZ_BITS(im_siz) + 63) / 64 * 8;
    texture->data = memalign(64, tiles_x * tiles_y * texture->tile_size);
    if (!texture->data) {
        free(texture);
        return NULL;
    }
    texture->im_fmt = im_fmt;
    texture->im_siz = im_siz;
    texture->tile_width = tile_width;
    texture->tile_height = tile_height;
    texture->tiles_x = tiles_x;
    texture->tiles_y = tiles_y;
    return texture;
}

struct gfx_texture *gfx_texture_load(const struct gfx_texdesc *texdesc, struct gfx_texture *texture) {
    struct gfx_texldr texldr;
    gfx_texldr_init(&texldr);
    texture = gfx_texldr_load(&texldr, texdesc, texture);
    gfx_texldr_destroy(&texldr);
    return texture;
}

void gfx_texture_destroy(struct gfx_texture *texture) {
    if (texture->data) {
        free(texture->data);
    }
}

void gfx_texture_free(struct gfx_texture *texture) {
    gfx_texture_destroy(texture);
    free(texture);
}

void *gfx_texture_data(const struct gfx_texture *texture, s16 tile) {
    return (char *)texture->data + texture->tile_size * tile;
}

struct gfx_texture *gfx_texture_copy(const struct gfx_texture *src, struct gfx_texture *dest) {
    struct gfx_texture *new_texture = NULL;
    if (!dest) {
        new_texture = malloc(sizeof(*new_texture));
        if (!new_texture) {
            return new_texture;
        }
        dest = new_texture;
    }
    size_t texture_size = src->tile_size * src->tiles_x * src->tiles_y;
    void *texture_data = memalign(64, texture_size);
    if (!texture_data) {
        if (new_texture) {
            free(new_texture);
        }
        return NULL;
    }
    *dest = *src;
    dest->data = texture_data;
    memcpy(dest->data, src->data, texture_size);
    return dest;
}

void gfx_texture_copy_tile(struct gfx_texture *dest, s32 dest_tile, const struct gfx_texture *src, s32 src_tile,
                           _Bool blend) {
    if (src->im_fmt != G_IM_FMT_RGBA || src->im_siz != G_IM_SIZ_32b || dest->im_fmt != src->im_fmt ||
        dest->im_siz != src->im_siz || dest->tile_width != src->tile_width || dest->tile_height != src->tile_height) {
        return;
    }
    struct rgba32 {
        u8 r;
        u8 g;
        u8 b;
        u8 a;
    };
    size_t tile_pixels = src->tile_width * src->tile_height;
    struct rgba32 *p_dest = gfx_texture_data(dest, dest_tile);
    struct rgba32 *p_src = gfx_texture_data(src, src_tile);
    for (size_t i = 0; i < tile_pixels; ++i) {
        if (blend) {
            p_dest->r = p_dest->r + (p_src->r - p_dest->r) * p_src->a / 0xFF;
            p_dest->g = p_dest->g + (p_src->g - p_dest->g) * p_src->a / 0xFF;
            p_dest->b = p_dest->b + (p_src->b - p_dest->b) * p_src->a / 0xFF;
            p_dest->a = p_src->a + (0xFF - p_src->a) * p_dest->a / 0xFF;
        } else {
            *p_dest = *p_src;
        }
        ++p_dest;
        ++p_src;
    }
}

void gfx_texture_colortransform(struct gfx_texture *texture, const MtxF *matrix) {
    if (texture->im_fmt != G_IM_FMT_RGBA || texture->im_siz != G_IM_SIZ_32b) {
        return;
    }
    struct rgba32 {
        u8 r;
        u8 g;
        u8 b;
        u8 a;
    };
    size_t texture_pixels = texture->tile_width * texture->tile_height * texture->tiles_x * texture->tiles_y;
    struct rgba32 *pixel_data = texture->data;
    MtxF m = *matrix;
    for (size_t i = 0; i < texture_pixels; ++i) {
        struct rgba32 p = pixel_data[i];
        f32 r = p.r * m.xx + p.g * m.xy + p.b * m.xz + p.a * m.xw;
        f32 g = p.r * m.yx + p.g * m.yy + p.b * m.yz + p.a * m.yw;
        f32 b = p.r * m.zx + p.g * m.zy + p.b * m.zz + p.a * m.zw;
        f32 a = p.r * m.wx + p.g * m.wy + p.b * m.wz + p.a * m.ww;
        struct rgba32 n = {
            r < 0x00   ? 0x00
            : r > 0xFF ? 0xFF
                       : r,
            g < 0x00   ? 0x00
            : g > 0xFF ? 0xFF
                       : g,
            b < 0x00   ? 0x00
            : b > 0xFF ? 0xFF
                       : b,
            a < 0x00   ? 0x00
            : a > 0xFF ? 0xFF
                       : a,
        };
        pixel_data[i] = n;
    }
}

void gfx_disp_rdp_load_tile(Gfx **disp, const struct gfx_texture *texture, s16 texture_tile) {
    if (texture->im_siz == G_IM_SIZ_4b) {
        gDPLoadTextureTile_4b((*disp)++, gfx_texture_data(texture, texture_tile), texture->im_fmt, texture->tile_width,
                              texture->tile_height, 0, 0, texture->tile_width - 1, texture->tile_height - 1, 0,
                              G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                              G_TX_NOLOD, G_TX_NOLOD);
    } else {
        gDPLoadTextureTile((*disp)++, gfx_texture_data(texture, texture_tile), texture->im_fmt, texture->im_siz,
                           texture->tile_width, texture->tile_height, 0, 0, texture->tile_width - 1,
                           texture->tile_height - 1, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP,
                           G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    }
}

void gfx_rdp_load_tile(const struct gfx_texture *texture, s16 texture_tile) {
    gfx_disp_rdp_load_tile(&gfx_disp_p, texture, texture_tile);
    gfx_synced = 1;
}

void gfx_sprite_draw(const struct gfx_sprite *sprite) {
    struct gfx_texture *texture = sprite->texture;
    gfx_rdp_load_tile(texture, sprite->texture_tile);
    if (gfx_modes[GFX_MODE_DROPSHADOW]) {
        u8 a = gfx_modes[GFX_MODE_COLOR] & 0xFF;
        a = a * a / 0xFF;
        gfx_mode_replace(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0x00, 0x00, a));
        gSPScisTextureRectangle(gfx_disp_p++, qs102(sprite->x + 1) & ~3, qs102(sprite->y + 1) & ~3,
                                qs102(sprite->x + texture->tile_width * sprite->xscale + 1) & ~3,
                                qs102(sprite->y + texture->tile_height * sprite->yscale + 1) & ~3, G_TX_RENDERTILE,
                                qu105(0), qu105(0), qu510(1.f / sprite->xscale), qu510(1.f / sprite->yscale));
        gfx_mode_pop(GFX_MODE_COLOR);
    }
    gfx_sync();
    gSPScisTextureRectangle(gfx_disp_p++, qs102(sprite->x) & ~3, qs102(sprite->y) & ~3,
                            qs102(sprite->x + texture->tile_width * sprite->xscale) & ~3,
                            qs102(sprite->y + texture->tile_height * sprite->yscale) & ~3, G_TX_RENDERTILE, qu105(0),
                            qu105(0), qu510(1.f / sprite->xscale), qu510(1.f / sprite->yscale));
    gfx_synced = 0;
}

s32 gfx_font_xheight(const struct gfx_font *font) {
    return font->baseline - font->median;
}

void gfx_printf(const struct gfx_font *font, s32 x, s32 y, const char *format, ...) {
    if (gfx_modes[GFX_MODE_TEXT] == GFX_TEXT_NORMAL) {
        va_list args;
        va_start(args, format);
        gfx_printf_n_va(font, x, y, format, args);
        va_end(args);
    } else if (gfx_modes[GFX_MODE_TEXT] == GFX_TEXT_FAST) {
        va_list args;
        va_start(args, format);
        gfx_printf_f_va(font, x, y, format, args);
        va_end(args);
    }
}

void gfx_printf_n(const struct gfx_font *font, s32 x, s32 y, const char *format, ...) {
    va_list args;
    va_start(args, format);
    gfx_printf_n_va(font, x, y, format, args);
    va_end(args);
}

void gfx_printf_f(const struct gfx_font *font, s32 x, s32 y, const char *format, ...) {
    va_list args;
    va_start(args, format);
    gfx_printf_f_va(font, x, y, format, args);
    va_end(args);
}

static void draw_chars(const struct gfx_font *font, s32 x, s32 y, const char *buf, size_t l) {
    x -= font->x;
    y -= font->baseline;
    struct gfx_texture *texture = font->texture;
    s32 chars_per_tile = font->chars_xtile * font->chars_ytile;
    s32 n_tiles = texture->tiles_x * texture->tiles_y;
    s32 n_chars = chars_per_tile * n_tiles;
    for (s32 i = 0; i < n_tiles; ++i) {
        s32 tile_begin = chars_per_tile * i;
        s32 tile_end = tile_begin + chars_per_tile;
        _Bool tile_loaded = 0;
        s32 cx = 0;
        s32 cy = 0;
        for (s32 j = 0; j < l; ++j, cx += font->char_width + font->letter_spacing) {
            u8 c = buf[j];
            if (c < font->code_start || c >= font->code_start + n_chars) {
                continue;
            }
            c -= font->code_start;
            if (c < tile_begin || c >= tile_end) {
                continue;
            }
            c -= tile_begin;
            if (!tile_loaded) {
                tile_loaded = 1;
                gfx_rdp_load_tile(texture, i);
            }
            gSPScisTextureRectangle(gfx_disp_p++, qs102(x + cx), qs102(y + cy), qs102(x + cx + font->char_width),
                                    qs102(y + cy + font->char_height), G_TX_RENDERTILE,
                                    qu105(c % font->chars_xtile * font->char_width),
                                    qu105(c / font->chars_xtile * font->char_height), qu510(1), qu510(1));
        }
    }
    gfx_synced = 0;
}

static void flush_chars(void) {
    const struct gfx_font *font = gfx_char_font;
    u32 color = 0;
    _Bool first = 1;
    for (s32 i = 0; i < CHAR_TILE_MAX; ++i) {
        struct vector *tile_vect = &gfx_chars[i];
        for (s32 j = 0; j < tile_vect->size; ++j) {
            struct gfx_char *gc = vector_at(tile_vect, j);
            if (j == 0) {
                gfx_rdp_load_tile(font->texture, i);
            }
            if (first || color != gc->color) {
                color = gc->color;
                gfx_sync();
                gDPSetPrimColor(gfx_disp_p++, 0, 0, (color >> 24) & 0xFF, (color >> 16) & 0xFF, (color >> 8) & 0xFF,
                                (color >> 0) & 0xFF);
            }
            first = 0;
            gSPScisTextureRectangle(gfx_disp_p++, qs102(gc->x), qs102(gc->y), qs102(gc->x + font->char_width),
                                    qs102(gc->y + font->char_height), G_TX_RENDERTILE,
                                    qu105(gc->tile_char % font->chars_xtile * font->char_width),
                                    qu105(gc->tile_char / font->chars_xtile * font->char_height), qu510(1), qu510(1));
            gfx_synced = 0;
        }
        vector_clear(tile_vect);
    }
}

static void gfx_printf_n_va(const struct gfx_font *font, s32 x, s32 y, const char *format, va_list args) {
    const size_t bufsize = 1024;
    char buf[bufsize];
    s32 l = vsnprintf(buf, bufsize, format, args);
    if (l > bufsize - 1) {
        l = bufsize - 1;
    }
    if (gfx_modes[GFX_MODE_DROPSHADOW]) {
        u8 a = gfx_modes[GFX_MODE_COLOR] & 0xFF;
        a = a * a / 0xFF;
        gfx_mode_replace(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0x00, 0x00, a));
        draw_chars(font, x + 1, y + 1, buf, l);
        gfx_mode_pop(GFX_MODE_COLOR);
    }
    draw_chars(font, x, y, buf, l);
}

static void gfx_printf_f_va(const struct gfx_font *font, s32 x, s32 y, const char *format, va_list args) {
    const size_t bufsize = 1024;
    char buf[bufsize];
    s32 l = vsnprintf(buf, bufsize, format, args);
    if (l > bufsize - 1) {
        l = bufsize - 1;
    }
    x -= font->x;
    y -= font->baseline;
    struct gfx_texture *texture = font->texture;
    s32 chars_per_tile = font->chars_xtile * font->chars_ytile;
    s32 n_tiles = texture->tiles_x * texture->tiles_y;
    s32 n_chars = chars_per_tile * n_tiles;
    if (gfx_modes[GFX_MODE_DROPSHADOW]) {
        u8 a = gfx_modes[GFX_MODE_COLOR] & 0xFF;
        a = a * a / 0xFF;
        u32 color = GPACK_RGBA8888(0x00, 0x00, 0x00, a);
        s32 cx = x + 1;
        s32 cy = y + 1;
        for (s32 i = 0; i < l; ++i, cx += font->char_width + font->letter_spacing) {
            u8 c = buf[i];
            if (c < font->code_start || c >= font->code_start + n_chars) {
                continue;
            }
            c -= font->code_start;
            s32 tile_idx = c / chars_per_tile;
            s32 tile_char = c % chars_per_tile;
            struct gfx_char gc = {
                tile_char,
                color,
                cx,
                cy,
            };
            vector_push_back(&gfx_chars[tile_idx], 1, &gc);
        }
    }
    s32 cx = x;
    s32 cy = y;
    for (s32 i = 0; i < l; ++i, cx += font->char_width + font->letter_spacing) {
        u8 c = buf[i];
        if (c < font->code_start || c >= font->code_start + n_chars) {
            continue;
        }
        c -= font->code_start;
        s32 tile_idx = c / chars_per_tile;
        s32 tile_char = c % chars_per_tile;
        struct gfx_char gc = {
            tile_char,
            gfx_modes[GFX_MODE_COLOR],
            cx,
            cy,
        };
        vector_push_back(&gfx_chars[tile_idx], 1, &gc);
    }

    gfx_char_font = font;
}

void hud_element_draw_rect(HudElement *hudElement, s16 texSizeX, s16 texSizeY, s16 drawSizeX, s16 drawSizeY,
                           s16 offsetX, s16 offsetY, s32 clamp, s32 dropShadow) {
    u32 isFmtCI4;
    u32 isFmtIA8;
    s32 flipX, flipY;
    s32 fmt;
    s32 widthScale, heightScale;
    s32 texStartX, texStartY;
    u32 isLastTileX, isLastTileY;
    s32 uls, ult, lrs, lrt;
    s32 uly, lry, ulx, lrx;
    s32 masks, maskt;
    s32 screenPosOffsetScaledX, screenPosOffsetScaledY;
    u8 *imageAddr;
    u8 *paletteAddr;
    s16 baseX, baseY;
    s32 tileMode;
    u32 flags1, flags2;
    u16 renderPosX, renderPosY;
    s16 tempX, tempY;

    imageAddr = hudElement->rasterAddr;
    paletteAddr = hudElement->paletteAddr;

    screenPosOffsetScaledX = hudElement->screenPosOffset.x * 1024;
    screenPosOffsetScaledY = hudElement->screenPosOffset.y * 1024;
    widthScale = hudElement->widthScale;
    screenPosOffsetScaledX /= widthScale;
    heightScale = hudElement->heightScale;
    screenPosOffsetScaledY /= heightScale;

    renderPosX = hudElement->worldPosOffset.x;
    renderPosY = hudElement->worldPosOffset.y;
    renderPosX += hudElement->renderPosX + screenPosOffsetScaledX;
    renderPosY += hudElement->renderPosY + screenPosOffsetScaledY;
    tempX = offsetX;
    tempY = offsetY;
    tempX += renderPosX;
    tempY += renderPosY;

    baseX = tempX;
    baseY = tempY;

    if (dropShadow) {
        baseX = tempX + 2;
        baseY = tempY + 2;
    }

    flags1 = (hudElement->flags & HUD_ELEMENT_FLAGS_FMT_CI4);
    isFmtCI4 = flags1 != 0;
    flags1 = (hudElement->flags & HUD_ELEMENT_FLAGS_FMT_IA8);
    isFmtIA8 = flags1 != 0;
    flags1 = (hudElement->flags & HUD_ELEMENT_FLAGS_FLIPX);
    flipX = flags1 != 0;
    flags2 = (hudElement->flags & HUD_ELEMENT_FLAGS_FLIPY);
    flipY = flags2 != 0;

    fmt = 0; // stays the same if (isFmtCI4 == 0 && isFmtIA8 == 0)
    if (isFmtCI4 == 1 && isFmtIA8 == 1) {
        fmt = 0; // RGBA
    }
    if (isFmtCI4 == 1 && isFmtIA8 == 0) {
        fmt = 1; // CI
    }
    if (isFmtCI4 == 0 && isFmtIA8 == 1) {
        fmt = 2; // IA
    }
    if (isFmtCI4 == 1 && isFmtIA8 == 1) {
        fmt = 2; // IA
    }

    masks = 6;
    maskt = 5;
    if (!(hudElement->flags & HUD_ELEMENT_FLAGS_SCALED)) {
        switch (drawSizeX) {
            case 8: masks = 3; break;
            case 16: masks = 4; break;
            case 32: masks = 5; break;
        }

        switch (drawSizeY) {
            case 8: maskt = 3; break;
            case 16: maskt = 4; break;
            case 32: maskt = 5; break;
        }
    }

    switch (fmt) {
        case 0:
            gDPSetRenderMode(gfx_disp_p++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
            gDPSetTextureLUT(gfx_disp_p++, G_TT_NONE);
            break;
        case 1:
            if (!(hudElement->flags & HUD_ELEMENT_FLAGS_TRANSPARENT)) {
                if (!(hudElement->flags & HUD_ELEMENT_FLAGS_ANTIALIASING)) {
                    gDPSetRenderMode(gfx_disp_p++, G_RM_TEX_EDGE, G_RM_TEX_EDGE2);
                } else {
                    gDPSetRenderMode(gfx_disp_p++, G_RM_AA_TEX_EDGE, G_RM_AA_TEX_EDGE2);
                }
            } else {
                if (!(hudElement->flags & HUD_ELEMENT_FLAGS_ANTIALIASING)) {
                    gDPSetRenderMode(gfx_disp_p++, G_RM_CLD_SURF, G_RM_CLD_SURF2);
                } else {
                    gDPSetRenderMode(gfx_disp_p++, G_RM_CLD_SURF | AA_EN, G_RM_CLD_SURF2 | AA_EN);
                }
            }
            gDPSetTextureLUT(gfx_disp_p++, G_TT_RGBA16);
            gDPLoadTLUT_pal16(gfx_disp_p++, 0, paletteAddr);
            break;
        case 2:
            gDPSetRenderMode(gfx_disp_p++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
            gDPSetTextureLUT(gfx_disp_p++, G_TT_NONE);
            break;
    }

    if (hudElement->flags & HUD_ELEMENT_FLAGS_FILTER_TEX) {
        gDPSetTextureFilter(gfx_disp_p++, G_TF_AVERAGE);
    } else {
        gDPSetTextureFilter(gfx_disp_p++, G_TF_POINT);
    }

    ult = 0;
    isLastTileY = 0;
    uly = baseY;
    while (1) {
        lry = uly + 1024.0 / heightScale * 32.0;
        lrt = ult + 31;
        if (flipY) {
            texStartY = texSizeY;
        } else {
            texStartY = 0;
        }

        if (lry < 0 || uly > SCREEN_HEIGHT) {
            break;
        }

        if (lry >= SCREEN_HEIGHT) {
            s32 temp = ult + SCREEN_HEIGHT + 31;
            temp -= baseY + lrt;
            lrt = temp - 1;

            lry = SCREEN_HEIGHT;
            isLastTileY = 1;
        }

        if (lrt + 1 >= texSizeY) {
            lrt = texSizeY - 1;
            if (texSizeY > 16) {
                lry = baseY + drawSizeY - 1;
            } else {
                lry = baseY + drawSizeY;
            }
            isLastTileY = 1;
        }

        isLastTileX = 0;
        uls = 0;
        ulx = baseX;
        while (1) {
            lrx = ulx + 1024.0 / widthScale * 64.0;
            lrs = uls + 63;
            if (flipX) {
                texStartX = texSizeX;
            } else {
                texStartX = 0;
            }

            if (lrx < 0 || ulx > SCREEN_WIDTH) {
                break;
            }

            if (lrx >= SCREEN_WIDTH) {
                s32 temp = uls + SCREEN_WIDTH + 63;
                temp -= baseX + lrs;
                lrs = temp - 1;

                lrx = SCREEN_WIDTH;
                isLastTileX = 1;
            }

            if (lrs + 1 >= texSizeX) {
                lrs = texSizeX - 1;
                if (texSizeX > 16) {
                    lrx = baseX + drawSizeX - 1;
                } else {
                    lrx = baseX + drawSizeX;
                }
                isLastTileX = 1;
            }

            gDPPipeSync(gfx_disp_p++);

            if (isLastTileX) {
                tileMode = !isLastTileY;
            }
            if (!isLastTileX && !isLastTileY) {
                tileMode = 0;
            }
            if (!isLastTileX && isLastTileY) {
                tileMode = 2;
            }
            if (isLastTileX && isLastTileY) {
                tileMode = 3;
            }

            switch (fmt) {
                case 0:
                    gDPSetCombineMode(gfx_disp_p++, G_CC_DECALRGBA, G_CC_DECALRGBA);
                    if (!(hudElement->flags & HUD_ELEMENT_FLAGS_TRANSPARENT)) {
                        gDPSetCombineMode(gfx_disp_p++, G_CC_DECALRGBA, G_CC_DECALRGBA);
                    } else {
                        gDPSetCombineLERP(gfx_disp_p++, 0, 0, 0, TEXEL0, PRIMITIVE, 0, TEXEL0, 0, 0, 0, 0, TEXEL0,
                                          TEXEL0, 0, PRIMITIVE, 0);
                    }

                    if (hudElement->flags & HUD_ELEMENT_FLAGS_TRANSPARENT) {
                        gDPSetPrimColor(gfx_disp_p++, 0, 0, 0, 0, 0, hudElement->opacity);
                    }

                    if (!flipX && !flipY) {
                        gDPLoadTextureTile(gfx_disp_p++, imageAddr, G_IM_FMT_RGBA, G_IM_SIZ_32b, texSizeX, texSizeY,
                                           uls, ult, lrs, lrt, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP,
                                           masks, maskt, G_TX_NOLOD, G_TX_NOLOD);
                    } else {
                        gDPLoadTextureTile(gfx_disp_p++, imageAddr, G_IM_FMT_RGBA, G_IM_SIZ_32b, texSizeX, texSizeY,
                                           uls, ult, lrs, lrt, 0, G_TX_MIRROR | G_TX_WRAP, G_TX_MIRROR | G_TX_WRAP,
                                           masks, maskt, G_TX_NOLOD, G_TX_NOLOD);
                    }
                    break;
                case 1:
                    if (!dropShadow) {
                        if (hudElement->flags & HUD_ELEMENT_FLAGS_TRANSPARENT) {
                            gDPSetCombineLERP(gfx_disp_p++, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE,
                                              0, TEXEL0, 0, TEXEL0, 0, PRIMITIVE, 0);
                        } else {
                            gDPSetCombineLERP(gfx_disp_p++, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE, 0, TEXEL0, 0, PRIMITIVE,
                                              0, TEXEL0, 0, TEXEL0, 0, PRIMITIVE, 0);
                        }

                        if (hudElement->flags & HUD_ELEMENT_FLAGS_TRANSPARENT) {
                            gDPSetPrimColor(gfx_disp_p++, 0, 0, hudElement->tint.r, hudElement->tint.g,
                                            hudElement->tint.b, hudElement->opacity);
                        } else {
                            gDPSetPrimColor(gfx_disp_p++, 0, 0, hudElement->tint.r, hudElement->tint.g,
                                            hudElement->tint.b, 255);
                        }
                    } else {
                        gDPSetRenderMode(gfx_disp_p++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
                        gDPSetCombineLERP(gfx_disp_p++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0, 0, 0, 0, PRIMITIVE,
                                          TEXEL0, 0, PRIMITIVE, 0);
                        gDPSetPrimColor(gfx_disp_p++, 0, 0, 40, 40, 40, 72);
                    }

                    if (!flipX && !flipY) {
                        if (!clamp) {
                            gDPLoadTextureTile_4b(gfx_disp_p++, imageAddr, G_IM_FMT_CI, texSizeX, texSizeY, uls, ult,
                                                  lrs, lrt, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP,
                                                  masks, maskt, G_TX_NOLOD, G_TX_NOLOD);
                        } else {
                            switch (tileMode) {
                                case 0:
                                    gDPLoadTextureTile_4b(gfx_disp_p++, imageAddr, G_IM_FMT_CI, texSizeX, texSizeY, uls,
                                                          ult, lrs, lrt, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                                          G_TX_NOMIRROR | G_TX_WRAP, masks, maskt, G_TX_NOLOD,
                                                          G_TX_NOLOD);
                                    break;
                                case 1:
                                    gDPLoadTextureTile_4b(gfx_disp_p++, imageAddr, G_IM_FMT_CI, texSizeX, texSizeY, uls,
                                                          ult, lrs, lrt, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                                          G_TX_NOMIRROR | G_TX_WRAP, masks, maskt, G_TX_NOLOD,
                                                          G_TX_NOLOD);
                                    break;
                                case 2:
                                    gDPLoadTextureTile_4b(gfx_disp_p++, imageAddr, G_IM_FMT_CI, texSizeX, texSizeY, uls,
                                                          ult, lrs, lrt, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                                          G_TX_NOMIRROR | G_TX_WRAP, masks, maskt, G_TX_NOLOD,
                                                          G_TX_NOLOD);
                                    break;
                                case 3:
                                    gDPLoadTextureTile_4b(gfx_disp_p++, imageAddr, G_IM_FMT_CI, texSizeX, texSizeY, uls,
                                                          ult, lrs, lrt, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                                          G_TX_NOMIRROR | G_TX_WRAP, masks, maskt, G_TX_NOLOD,
                                                          G_TX_NOLOD);
                                    break;
                            }
                        }
                    } else {
                        gDPLoadTextureTile_4b(gfx_disp_p++, imageAddr, G_IM_FMT_CI, texSizeX, texSizeY, uls, ult, lrs,
                                              lrt, 0, G_TX_MIRROR | G_TX_WRAP, G_TX_MIRROR | G_TX_WRAP, masks, maskt,
                                              G_TX_NOLOD, G_TX_NOLOD);
                    }
                    break;
                case 2:
                    gDPSetCombineLERP(gfx_disp_p++, TEXEL0, 0, PRIMITIVE, 0, PRIMITIVE, 0, TEXEL0, 0, TEXEL0, 0,
                                      PRIMITIVE, 0, PRIMITIVE, 0, TEXEL0, 0);
                    gDPSetPrimColor(gfx_disp_p++, 0, 0, hudElement->tint.r, hudElement->tint.g, hudElement->tint.b,
                                    hudElement->opacity);

                    if (!flipX && !flipY) {
                        if (!clamp) {
                            switch (tileMode) {
                                case 0:
                                    gDPLoadTextureTile(gfx_disp_p++, imageAddr, G_IM_FMT_IA, G_IM_SIZ_8b, texSizeX,
                                                       texSizeY, uls, ult, lrs, lrt, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                                       G_TX_NOMIRROR | G_TX_WRAP, masks, maskt, G_TX_NOLOD, G_TX_NOLOD);
                                    break;
                                case 1:
                                    gDPLoadTextureTile(gfx_disp_p++, imageAddr, G_IM_FMT_IA, G_IM_SIZ_8b, texSizeX,
                                                       texSizeY, uls, ult, lrs, lrt, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                                       G_TX_NOMIRROR | G_TX_WRAP, masks, maskt, G_TX_NOLOD, G_TX_NOLOD);
                                    break;
                                case 2:
                                    gDPLoadTextureTile(gfx_disp_p++, imageAddr, G_IM_FMT_IA, G_IM_SIZ_8b, texSizeX,
                                                       texSizeY, uls, ult, lrs, lrt, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                                       G_TX_NOMIRROR | G_TX_WRAP, masks, maskt, G_TX_NOLOD, G_TX_NOLOD);
                                    break;
                                case 3:
                                    gDPLoadTextureTile(gfx_disp_p++, imageAddr, G_IM_FMT_IA, G_IM_SIZ_8b, texSizeX,
                                                       texSizeY, uls, ult, lrs, lrt, 0, G_TX_NOMIRROR | G_TX_WRAP,
                                                       G_TX_NOMIRROR | G_TX_WRAP, masks, maskt, G_TX_NOLOD, G_TX_NOLOD);
                                    break;
                            }
                        } else {
                            gDPLoadTextureTile(gfx_disp_p++, imageAddr, G_IM_FMT_IA, G_IM_SIZ_8b, texSizeX, texSizeY,
                                               uls, ult, lrs, lrt, 0, G_TX_NOMIRROR | G_TX_CLAMP,
                                               G_TX_NOMIRROR | G_TX_CLAMP, masks, maskt, G_TX_NOLOD, G_TX_NOLOD);
                        }
                    } else {
                        gDPLoadTextureTile(gfx_disp_p++, imageAddr, G_IM_FMT_IA, G_IM_SIZ_8b, texSizeX, texSizeY, uls,
                                           ult, lrs, lrt, 0, G_TX_MIRROR | G_TX_WRAP, G_TX_MIRROR | G_TX_WRAP, masks,
                                           maskt, G_TX_NOLOD, G_TX_NOLOD);
                    }
                    break;
            }

            if (hudElement->flags & HUD_ELEMENT_FLAGS_FILTER_TEX) {
                gSPScisTextureRectangle(gfx_disp_p++, ulx * 4, uly * 4, lrx * 4, lry * 4, 0, texStartX * 32 + 16,
                                        texStartY * 32 + 16, widthScale, heightScale);
            } else {
                gSPScisTextureRectangle(gfx_disp_p++, ulx * 4, uly * 4, lrx * 4, lry * 4, 0, texStartX * 32,
                                        texStartY * 32, widthScale, heightScale);
            }
            if (isLastTileX) {
                break;
            }
            ulx += 1024.0 / widthScale * 64.0;
            uls += 64;
        }

        if (isLastTileY) {
            break;
        }

        ult += 32;
        uly += 1024.0 / heightScale * 32.0;
    }

    gDPPipeSync(gfx_disp_p++);
}

void draw_hud_element(HudElement *elem) {
    s32 texSizeX, texSizeY;
    s32 drawSizeX, drawSizeY;
    s32 offsetX, offsetY;
    s32 preset;

    if (elem->flags && !(elem->flags & HUD_ELEMENT_FLAGS_DISABLED)) {
        if (!(elem->flags & (HUD_ELEMENT_FLAGS_200000 | HUD_ELEMENT_FLAGS_10000000)) && (elem->drawSizePreset >= 0)) {
            gDPPipeSync(gfx_disp_p++);
            gDPSetCycleType(gfx_disp_p++, G_CYC_1CYCLE);
            gDPSetTexturePersp(gfx_disp_p++, G_TP_NONE);
            gDPSetTextureLOD(gfx_disp_p++, G_TL_TILE);
            gDPSetTextureDetail(gfx_disp_p++, G_TD_CLAMP);
            gDPSetTextureConvert(gfx_disp_p++, G_TC_FILT);
            gDPSetCombineKey(gfx_disp_p++, G_CK_NONE);
            gDPSetAlphaCompare(gfx_disp_p++, G_AC_NONE);
            gDPNoOp(gfx_disp_p++);
            gDPSetColorDither(gfx_disp_p++, G_CD_DISABLE);
            gDPSetAlphaDither(gfx_disp_p++, G_AD_DISABLE);
            gSPTexture(gfx_disp_p++, -1, -1, 0, G_TX_RENDERTILE, G_ON);

            if (!(elem->flags & HUD_ELEMENT_FLAGS_FIXEDSCALE)) {
                if (!(elem->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
                    preset = elem->tileSizePreset;
                    texSizeX = gHudElementSizes[preset].width;
                    texSizeY = gHudElementSizes[preset].height;
                } else {
                    texSizeX = elem->customImageSize.x;
                    texSizeY = elem->customImageSize.y;
                }

                if (!(elem->flags & HUD_ELEMENT_FLAGS_SCALED)) {
                    if (!(elem->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
                        drawSizeX = gHudElementSizes[elem->drawSizePreset].width;
                        drawSizeY = gHudElementSizes[elem->drawSizePreset].height;
                    } else {
                        drawSizeX = elem->customDrawSize.x;
                        drawSizeY = elem->customDrawSize.y;
                    }
                } else {
                    drawSizeX = elem->sizeX;
                    drawSizeY = elem->sizeY;
                }

                do {
                    offsetX = -drawSizeX / 2;
                    offsetY = -drawSizeY / 2;
                } while (0); // required to match

                if (!(elem->flags & HUD_ELEMENT_FLAGS_REPEATED)) {
                    if (elem->flags & HUD_ELEMENT_FLAGS_DROP_SHADOW) {
                        hud_element_draw_rect(elem, texSizeX, texSizeY, drawSizeX, drawSizeY, offsetX, offsetY, 1, 1);
                    }
                    hud_element_draw_rect(elem, texSizeX, texSizeY, drawSizeX, drawSizeY, offsetX, offsetY, 1, 0);
                } else {
                    if (elem->flags & HUD_ELEMENT_FLAGS_DROP_SHADOW) {
                        hud_element_draw_rect(elem, texSizeX, texSizeY, drawSizeX, drawSizeY, offsetX, offsetY, 0, 1);
                    }
                    hud_element_draw_rect(elem, texSizeX, texSizeY, drawSizeX, drawSizeY, offsetX, offsetY, 0, 0);
                }
            } else {
                f32 xScaled, yScaled;

                if (!(elem->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
                    preset = elem->tileSizePreset;
                    texSizeX = gHudElementSizes[preset].width;
                    texSizeY = gHudElementSizes[preset].height;
                } else {
                    texSizeX = elem->customImageSize.x;
                    texSizeY = elem->customImageSize.y;
                }

                drawSizeX = elem->unkImgScale[0];
                drawSizeY = elem->unkImgScale[1];

                offsetX = -elem->unkImgScale[0] / 2;
                offsetY = -elem->unkImgScale[1] / 2;

                xScaled = (f32)drawSizeX / (f32)texSizeX;
                yScaled = (f32)drawSizeY / (f32)texSizeY;

                xScaled = 1.0f / xScaled;
                yScaled = 1.0f / yScaled;

                elem->widthScale = X10(xScaled);
                elem->heightScale = X10(yScaled);

                if (elem->flags & HUD_ELEMENT_FLAGS_DROP_SHADOW) {
                    hud_element_draw_rect(elem, texSizeX, texSizeY, drawSizeX, drawSizeY, offsetX, offsetY, 0, 1);
                }
                hud_element_draw_rect(elem, texSizeX, texSizeY, drawSizeX, drawSizeY, offsetX, offsetY, 0, 1);
            }
        }
    }
}
