#include "gfx.h"
#include "common.h"
#include <malloc.h>
#include <mips.h>
#include <n64.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector/vector.h>

#define GFX_DISP_SIZE 0x10000
static Gfx *gfx_disp;
static Gfx *gfx_disp_w;
static Gfx *gfx_disp_p;
static Gfx *gfx_disp_d;

#define GFX_STACK_LENGTH 8
static u64 gfx_modes[GFX_MODE_ALL];
static u64 gfx_mode_stack[GFX_MODE_ALL][GFX_STACK_LENGTH];
static s32 gfx_mode_stack_pos[GFX_MODE_ALL];
static bool gfx_synced;

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
        gfx_synced = TRUE;
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
    gfx_synced = FALSE;
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
    gfx_synced = FALSE;
}

void gfx_texldr_init(struct gfx_texldr *texldr) {
    texldr->file_vaddr = GFX_FILE_DRAM;
    texldr->file_data = NULL;
}

static s32 get_texture_tile_raster_size(const struct gfx_texture *texture) {
    return texture->tile_width * texture->tile_height * G_SIZ_BITS(texture->im_siz) / 8;
}

static bool texture_data_is_on_heap(struct gfx_texture *texture) {
    return (uintptr_t)texture->data >= 0x80400000;
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
    texture->tile_size = (get_texture_tile_raster_size(texture) + ICON_PALETTE_SIZE * texdesc->pal_count + 7) / 8 * 8;
    texture->pal_count = texdesc->pal_count;
    size_t texture_size = texture->tile_size * texture->tiles_x * texture->tiles_y;
    void *texture_data = NULL;
    void *file_start = NULL;
    if (texdesc->file_vaddr == GFX_FILE_DRAM_PM) {
        // these textures are always loaded in memory by the base game, so no need to waste memory copying them
        texture_data = (void *)texdesc->address;
        texture->tile_size = get_texture_tile_raster_size(texture) + ICON_PALETTE_SIZE * texdesc->pal_count;
    } else if (texdesc->file_vaddr != GFX_FILE_DRAM) {
        if (texldr->file_vaddr != texdesc->file_vaddr) {
            if (texldr->file_data) {
                free(texldr->file_data);
            }
            texldr->file_data = malloc(texdesc->file_vsize);
            if (!texldr->file_data) {
                texldr->file_vaddr = GFX_FILE_DRAM;
                if (new_texture) {
                    free(new_texture);
                }
                return NULL;
            }
            texldr->file_vaddr = texdesc->file_vaddr;
            nuPiReadRom(texldr->file_vaddr, texldr->file_data, texdesc->file_vsize);
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
        texture_data = malloc(texture_size);
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
    if (texture->data && texture_data_is_on_heap(texture)) {
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
                           bool blend) {
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

void gfx_texture_mirror_horizontal(struct gfx_texture *texture, s16 tile) {
    s32 adj_width = texture->tile_width;
    s32 bytes_per_pixel = G_SIZ_BITS(texture->im_siz) / 8;
    if (bytes_per_pixel == 0) {
        bytes_per_pixel = 1; // 0.5, but we'll handle it manually
        adj_width /= 2;
    }

    if (!texture_data_is_on_heap(texture)) {
        void *new_data = malloc(texture->tiles_x * texture->tiles_y * texture->tile_size);
        memcpy(new_data, texture->data, sizeof(*new_data));
        texture->data = new_data;
    }

    unsigned char *new_raster = malloc(get_texture_tile_raster_size(texture));
    unsigned char *new_pos = new_raster;
    unsigned char *old_raster = texture->data + tile * texture->tile_size;
    unsigned char *old_pos;
    for (s32 y = 0; y < texture->tile_height; y++) {
        s32 y_offset = bytes_per_pixel * (texture->tile_height * y + texture->tile_width - 1);
        if (texture->im_siz == G_IM_SIZ_4b) {
            y_offset /= 2;
        }
        old_pos = old_raster + y_offset;
        for (s32 x = 0; x < adj_width; x++) {
            memcpy(new_pos, old_pos, bytes_per_pixel);
            if (texture->im_siz == G_IM_SIZ_4b) {
                *new_pos = (*new_pos & 0xF0) >> 4 | (*new_pos & 0x0F) << 4;
            }
            new_pos += bytes_per_pixel;
            old_pos -= bytes_per_pixel;
        }
    }
    memcpy(old_raster, new_raster, get_texture_tile_raster_size(texture));
    free(new_raster);
}

void gfx_texture_translate(struct gfx_texture *texture, s16 tile, s32 x_offset, s32 y_offset) {
    s32 adj_width = texture->tile_width;
    s32 adj_height = texture->tile_height;
    s32 bytes_per_pixel = G_SIZ_BITS(texture->im_siz) / 8;
    s32 adj_x_offset = x_offset;
    if (bytes_per_pixel == 0) {
        bytes_per_pixel = 1; // 0.5, but we'll handle it manually
        adj_width /= 2;
        adj_height /= 2;
        adj_x_offset /= 2;
    }

    if (!texture_data_is_on_heap(texture)) {
        void *new_data = malloc(texture->tiles_x * texture->tiles_y * texture->tile_size);
        memcpy(new_data, texture->data, sizeof(*new_data));
        texture->data = new_data;
    }

    if (x_offset != 0) {
        unsigned char *new_raster = calloc(get_texture_tile_raster_size(texture), 1);
        unsigned char *new_pos;
        unsigned char *old_raster = texture->data + tile * texture->tile_size;
        unsigned char *old_pos;
        for (s32 y = 0; y < texture->tile_height; y++) {
            new_pos = new_raster + bytes_per_pixel * (adj_height * y);
            old_pos = old_raster + bytes_per_pixel * (adj_height * y - adj_x_offset);
            for (s32 x = 0; x < adj_width; x++) {
                if (x >= adj_x_offset && x < adj_width + adj_x_offset) {
                    if (texture->im_siz == G_IM_SIZ_4b && (x_offset % 2 == 1 || x_offset % 2 == -1)) {
                        if (x_offset > 0) {
                            *new_pos = (*old_pos & 0xF0) >> 4;
                            if (x > adj_x_offset) {
                                *new_pos |= (*(old_pos - 1) & 0x0F) << 4;
                            }
                        } else {
                            *new_pos = (*old_pos & 0x0F) << 4;
                            if (x < adj_width + adj_x_offset - 1) {
                                *new_pos |= (*(old_pos + 1) & 0xF0) >> 4;
                            }
                        }
                    } else {
                        memcpy(new_pos, old_pos, bytes_per_pixel);
                    }
                }
                new_pos += bytes_per_pixel;
                old_pos += bytes_per_pixel;
            }
        }
        memcpy(old_raster, new_raster, get_texture_tile_raster_size(texture));
        free(new_raster);
    }
    if (y_offset != 0) {
        unsigned char *new_raster = calloc(get_texture_tile_raster_size(texture), 1);
        unsigned char *new_pos;
        unsigned char *old_raster = texture->data + tile * texture->tile_size;
        unsigned char *old_pos;
        for (s32 y = 0; y < texture->tile_height; y++) {
            if (y >= y_offset && y < texture->tile_height + y_offset) {
                new_pos = new_raster + bytes_per_pixel * (adj_height * y);
                old_pos = old_raster + bytes_per_pixel * (adj_height * y - y_offset * adj_width);
                for (s32 x = 0; x < adj_width; x++) {
                    memcpy(new_pos, old_pos, bytes_per_pixel);
                    new_pos += bytes_per_pixel;
                    old_pos += bytes_per_pixel;
                }
            }
        }
        memcpy(old_raster, new_raster, get_texture_tile_raster_size(texture));
        free(new_raster);
    }
}

void gfx_add_grayscale_palette(struct gfx_texture *texture, s8 base_palette_index) {
    if (texture->im_fmt != G_IM_FMT_CI || texture->im_siz != G_IM_SIZ_4b) {
        return;
    }
    typedef struct {
        u16 r : 5;
        u16 g : 5;
        u16 b : 5;
        u16 a : 1;
    } rgba;

    s32 tile_count = texture->tiles_x * texture->tiles_y;
    char *new_texture_data = malloc((texture->tile_size + ICON_PALETTE_SIZE) * tile_count);
    size_t new_tile_size = texture->tile_size + ICON_PALETTE_SIZE;

    u32 raster_size = get_texture_tile_raster_size(texture);

    for (s32 i_tile = 0; i_tile < tile_count; i_tile++) {
        char *base_tile = texture->data + texture->tile_size * i_tile;
        char *new_tile = new_texture_data + new_tile_size * i_tile;
        memcpy(new_tile, base_tile, texture->tile_size);

        rgba *base_palette = (rgba *)(base_tile + raster_size + ICON_PALETTE_SIZE * base_palette_index);
        rgba *new_palette = (rgba *)(new_texture_data + raster_size + ICON_PALETTE_SIZE * texture->pal_count);
        u8 pixel_count = ICON_PALETTE_SIZE / 2;
        for (u32 i_pixel = 0; i_pixel < pixel_count; i_pixel++) {
            rgba *old_pixel = &base_palette[i_pixel];
            rgba *new_pixel = &((rgba *)new_palette)[i_pixel];

            f32 lum = 0.2782f * old_pixel->r + 0.6562f * old_pixel->g + 0.0656f * old_pixel->b;
            u16 gray = (lum * 14 / 31) + 12;
            new_pixel->r = gray;
            new_pixel->g = gray;
            new_pixel->b = gray;
            new_pixel->a = old_pixel->a;
        }
    }

    if (texture_data_is_on_heap(texture)) {
        free(texture->data);
    }
    texture->data = new_texture_data;
    texture->tile_size = new_tile_size;
    texture->pal_count++;
}

void gfx_disp_rdp_load_tile(Gfx **disp, const struct gfx_texture *texture, s16 texture_tile, s8 palette_index) {
    if (texture->im_fmt == G_IM_FMT_CI) {
        gDPSetTextureLUT((*disp)++, G_TT_RGBA16);
        gDPLoadTLUT_pal16((*disp)++, 0,
                          gfx_texture_data(texture, texture_tile) + get_texture_tile_raster_size(texture) +
                              ICON_PALETTE_SIZE * palette_index);
    } else {
        gDPSetTextureLUT((*disp)++, G_TT_NONE);
    }
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

void gfx_rdp_load_tile(const struct gfx_texture *texture, s16 texture_tile, s8 palette_index) {
    gfx_disp_rdp_load_tile(&gfx_disp_p, texture, texture_tile, palette_index);
    gfx_synced = TRUE;
}

void gfx_sprite_draw(const struct gfx_sprite *sprite) {
    struct gfx_texture *texture = sprite->texture;
    gfx_rdp_load_tile(texture, sprite->texture_tile, sprite->palette_index);
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
    gfx_synced = FALSE;
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
        bool tile_loaded = FALSE;
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
                tile_loaded = TRUE;
                gfx_rdp_load_tile(texture, i, 0);
            }
            gSPScisTextureRectangle(gfx_disp_p++, qs102(x + cx), qs102(y + cy), qs102(x + cx + font->char_width),
                                    qs102(y + cy + font->char_height), G_TX_RENDERTILE,
                                    qu105(c % font->chars_xtile * font->char_width),
                                    qu105(c / font->chars_xtile * font->char_height), qu510(1), qu510(1));
        }
    }
    gfx_synced = FALSE;
}

static void flush_chars(void) {
    const struct gfx_font *font = gfx_char_font;
    u32 color = 0;
    bool first = TRUE;
    for (s32 i = 0; i < CHAR_TILE_MAX; ++i) {
        struct vector *tile_vect = &gfx_chars[i];
        for (s32 j = 0; j < tile_vect->size; ++j) {
            struct gfx_char *gc = vector_at(tile_vect, j);
            if (j == 0) {
                gfx_rdp_load_tile(font->texture, i, 0);
            }
            if (first || color != gc->color) {
                color = gc->color;
                gfx_sync();
                gDPSetPrimColor(gfx_disp_p++, 0, 0, (color >> 24) & 0xFF, (color >> 16) & 0xFF, (color >> 8) & 0xFF,
                                (color >> 0) & 0xFF);
            }
            first = FALSE;
            gSPScisTextureRectangle(gfx_disp_p++, qs102(gc->x), qs102(gc->y), qs102(gc->x + font->char_width),
                                    qs102(gc->y + font->char_height), G_TX_RENDERTILE,
                                    qu105(gc->tile_char % font->chars_xtile * font->char_width),
                                    qu105(gc->tile_char / font->chars_xtile * font->char_height), qu510(1), qu510(1));
            gfx_synced = FALSE;
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
