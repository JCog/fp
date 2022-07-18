#include <stdlib.h>
#include <grc.h>
#include "gfx.h"
#include "gu.h"
#include "resource.h"
#include "pm64.h"
#include "util.h"

#define VSIZE(x, y, im_size, palette_count, tile_count) \
    (x * y * G_SIZ_BITS(im_size) / 8 + ICON_PALETTE_SIZE * palette_count) * tile_count

struct item_icon_cache {
    u32 address;
    struct gfx_texture *texture;
};
static struct item_icon_cache item_textures[0x16D];

/* resource data table */
static void *res_data[RES_MAX] = {NULL};

/* resource constructors */
static void *rc_font_generic(struct gfx_texdesc *texdesc, s32 char_width, s32 char_height, s32 code_start,
                             s32 letter_spacing, s32 line_spacing, s32 baseline, s32 median, s32 x) {
    struct gfx_font *font = malloc(sizeof(*font));
    if (!font) {
        return font;
    }
    struct gfx_texture *texture = gfx_texture_load(texdesc, NULL);
    if (!texture) {
        free(font);
        return NULL;
    }
    font->texture = texture;
    font->char_width = char_width;
    font->char_height = char_height;
    font->chars_xtile = texture->tile_width / font->char_width;
    font->chars_ytile = texture->tile_height / font->char_height;
    font->code_start = code_start;
    font->letter_spacing = letter_spacing;
    font->line_spacing = line_spacing;
    font->baseline = baseline;
    font->median = median;
    font->x = x;
    return font;
}

static void *rc_grc_font_generic(const char *grc_resource_name, s32 char_width, s32 char_height, s32 code_start,
                                 s32 letter_spacing, s32 line_spacing, s32 baseline, s32 median, s32 x) {
    void *p_t;
    grc_resource_get(grc_resource_name, &p_t, NULL);
    if (!p_t) {
        return p_t;
    }
    struct grc_texture *t = p_t;
    struct gfx_texdesc td = {
        t->im_fmt,
        t->im_siz,
        (u32)&t->texture_data,
        t->tile_width,
        t->tile_height,
        t->tiles_x,
        t->tiles_y,
        GFX_FILE_DRAM,
        0,
        0,
    };
    return rc_font_generic(&td, char_width, char_height, code_start, letter_spacing, line_spacing, baseline, median, x);
}

static void *rc_font_fipps(void) {
    return rc_grc_font_generic("fipps", 10, 14, 33, -2, -5, 10, 3, 2);
}

static void *rc_font_notalot35(void) {
    return rc_grc_font_generic("notalot35", 8, 9, 33, -1, -1, 7, 2, 2);
}

static void *rc_font_origamimommy(void) {
    return rc_grc_font_generic("origamimommy", 8, 10, 33, -2, -2, 8, 1, 0);
}

static void *rc_font_pcsenior(void) {
    return rc_grc_font_generic("pcsenior", 8, 8, 33, 0, 0, 7, 2, 0);
}

static void *rc_font_pixelintv(void) {
    return rc_grc_font_generic("pixelintv", 8, 12, 33, 0, -4, 10, 5, 1);
}

static void *rc_font_pressstart2p(void) {
    return rc_grc_font_generic("pressstart2p", 8, 8, 33, 0, 0, 7, 2, 0);
}

static void *rc_font_smwtextnc(void) {
    return rc_grc_font_generic("smwtextnc", 12, 8, 33, -4, 0, 7, 2, 3);
}

static void *rc_font_werdnasreturn(void) {
    return rc_grc_font_generic("werdnasreturn", 8, 12, 33, 0, -4, 11, 6, 1);
}

static void *rc_font_pixelzim(void) {
    return rc_grc_font_generic("pixelzim", 3, 6, 33, 1, 0, 5, 3, 0);
}

static s32 hud_script_to_texdesc(struct gfx_texdesc *td_out, const u32 *hud_script, u32 vaddr_offset, s8 pal_count) {
    g_ifmt_t im_fmt = G_IM_FMT_RGBA;
    g_isiz_t im_siz = G_IM_SIZ_32b;
    u32 address = UINT32_MAX;
    s16 tile_width = 0;
    s16 tile_height = 0;

    const u32 *script_pos = hud_script;
    s32 tile_size_preset;
    _Bool script_done = 0;
    while (!script_done) {
        switch (*script_pos++) {
            case HUD_ELEMENT_OP_End: script_done = 1; break;
            case HUD_ELEMENT_OP_SetCI:
                im_fmt = G_IM_FMT_CI;
                im_siz = G_IM_SIZ_4b;
                script_pos++;
                address = *script_pos++;
                script_pos++;
                break;
            case HUD_ELEMENT_OP_SetImage:
                im_fmt = G_IM_FMT_CI;
                im_siz = G_IM_SIZ_4b;
                script_pos++;
                address = *script_pos++;
                script_pos += 3;
                break;
            case HUD_ELEMENT_OP_UseIA8:
                im_fmt = G_IM_FMT_IA;
                im_siz = G_IM_SIZ_8b;
                break;
            case HUD_ELEMENT_OP_SetRGBA:
                script_pos++;
                address = *script_pos++;
                break;
            case HUD_ELEMENT_OP_SetTileSize:
                tile_size_preset = *script_pos++;
                tile_width = pm_gHudElementSizes[tile_size_preset].width;
                tile_height = pm_gHudElementSizes[tile_size_preset].height;
                break;
            case HUD_ELEMENT_OP_SetCustomSize:
                tile_width = *script_pos++;
                tile_height = *script_pos++;
                break;
            case HUD_ELEMENT_OP_AddTexelOffsetX:
            case HUD_ELEMENT_OP_AddTexelOffsetY:
            case HUD_ELEMENT_OP_SetScale:
            case HUD_ELEMENT_OP_SetAlpha:
            case HUD_ELEMENT_OP_op_15:
            case HUD_ELEMENT_OP_RandomBranch:
            case HUD_ELEMENT_OP_SetFlags:
            case HUD_ELEMENT_OP_ClearFlags:
            case HUD_ELEMENT_OP_PlaySound: script_pos++; break;
            case HUD_ELEMENT_OP_SetTexelOffset:
            case HUD_ELEMENT_OP_RandomDelay:
            case HUD_ELEMENT_OP_RandomRestart:
            case HUD_ELEMENT_OP_SetPivot: script_pos += 2; break;
            case HUD_ELEMENT_OP_SetSizesAutoScale:
            case HUD_ELEMENT_OP_SetSizesFixedScale: script_pos += 3; break;
        }
    }
    if (address == UINT32_MAX || tile_width == 0 || tile_height == 0) {
        PRINTF("error loading hud_script 0x%X\n addr: 0x%X, w: %d, h: %d\n\n", hud_script, address, tile_width,
               tile_height);
        return 0;
    }

    u32 file_vaddr;
    size_t file_vsize;
    if (address >> 24 == 0x80) {
        file_vaddr = GFX_FILE_DRAM_PM;
        file_vsize = 0;
    } else {
        file_vaddr = address + vaddr_offset;
        file_vsize = VSIZE(tile_width, tile_height, im_siz, pal_count, 1);
    }

    *td_out = (struct gfx_texdesc){
        im_fmt, im_siz, address, tile_width, tile_height, 1, 1, file_vaddr, file_vsize, pal_count,
    };
    return 1;
}

static void *rc_pmicon_partner(void) {
    struct gfx_texdesc td = {
        G_IM_FMT_CI, G_IM_SIZ_4b, 0, 32, 32, 1, 12, ICONS_PARTNERS_ROM_START, VSIZE(32, 32, G_IM_SIZ_4b, 2, 12), 2,
    };
    return gfx_texture_load(&td, NULL);
}

static void *rc_pmicon_star_spirits(void) {
    struct gfx_texdesc td = {
        G_IM_FMT_CI, G_IM_SIZ_4b, 0, 32, 32, 1, 9, ICONS_STAR_SPIRITS_ROM_START, VSIZE(32, 32, G_IM_SIZ_4b, 2, 9), 2,
    };
    return gfx_texture_load(&td, NULL);
}

static void *rc_pmicon_bp(void) {
    struct gfx_texdesc td = {
        G_IM_FMT_CI, G_IM_SIZ_4b, 0, 16, 16, 1, 1, ICONS_BP_ROM_START, VSIZE(16, 16, G_IM_SIZ_4b, 1, 1), 1,
    };
    return gfx_texture_load(&td, NULL);
}

static void *rc_icon_check(void) {
    return resource_load_grc_texture("check_icons");
}

static void *rc_icon_buttons(void) {
    return resource_load_grc_texture("button_icons");
}

static void *rc_icon_pause(void) {
    return resource_load_grc_texture("pause_icons");
}

static void *rc_icon_macro(void) {
    return resource_load_grc_texture("macro_icons");
}

static void *rc_icon_movie(void) {
    return resource_load_grc_texture("movie_icons");
}

static void *rc_icon_arrow(void) {
    return resource_load_grc_texture("arrow_icons");
}

static void *rc_icon_file(void) {
    return resource_load_grc_texture("file_icons");
}

static void *rc_icon_save(void) {
    return resource_load_grc_texture("save_icons");
}

static void *rc_icon_osk(void) {
    return resource_load_grc_texture("osk_icons");
}

static void *rc_texture_crosshair(void) {
    return resource_load_grc_texture("crosshair");
}

static void *rc_texture_control_stick(void) {
    return resource_load_grc_texture("control_stick");
}

/* resource destructors */
static void rd_font_generic(void *data) {
    struct gfx_font *font = data;
    gfx_texture_free(font->texture);
    free(font);
}

/* resource management tables */
static void *(*res_ctor[RES_MAX])(void) = {
    rc_font_fipps,
    rc_font_notalot35,
    rc_font_origamimommy,
    rc_font_pcsenior,
    rc_font_pixelintv,
    rc_font_pressstart2p,
    rc_font_smwtextnc,
    rc_font_werdnasreturn,
    rc_font_pixelzim,
    rc_pmicon_partner,
    rc_pmicon_star_spirits,
    rc_pmicon_bp,
    rc_icon_check,
    rc_icon_buttons,
    rc_icon_pause,
    rc_icon_macro,
    rc_icon_movie,
    rc_icon_arrow,
    rc_icon_file,
    rc_icon_save,
    rc_icon_osk,
    rc_texture_crosshair,
    rc_texture_control_stick,
};

static void (*res_dtor[RES_MAX])() = {
    rd_font_generic,  gfx_texture_free, gfx_texture_free, gfx_texture_free, gfx_texture_free,
    gfx_texture_free, gfx_texture_free, gfx_texture_free, gfx_texture_free, gfx_texture_free,
};

/* resource interface */
void *resource_get(enum resource_id res) {
    if (!res_data[res]) {
        res_data[res] = res_ctor[res]();
    }
    return res_data[res];
}

void resource_free(enum resource_id res) {
    if (res_data[res]) {
        res_dtor[res](res_data[res]);
        res_data[res] = NULL;
    }
}

struct gfx_texture *resource_load_grc_texture(const char *grc_resource_name) {
    void *p_t;
    grc_resource_get(grc_resource_name, &p_t, NULL);
    if (!p_t) {
        return p_t;
    }
    struct grc_texture *t = p_t;
    struct gfx_texdesc td = {
        t->im_fmt,
        t->im_siz,
        (u32)&t->texture_data,
        t->tile_width,
        t->tile_height,
        t->tiles_x,
        t->tiles_y,
        GFX_FILE_DRAM,
        0,
        0,
    };
    return gfx_texture_load(&td, NULL);
}

struct gfx_texture *resource_load_pmicon_item(u16 item, _Bool safe) {
    if (!safe && item_textures[item].texture) {
        return item_textures[item].texture;
    }
    item_data_t *item_data = &pm_gItemTable[item];
    u32 *script_enabled = (u32 *)pm_gItemHudScripts[item_data->hudElemID].enabled;
    u32 *script_disabled = (u32 *)pm_gItemHudScripts[item_data->hudElemID].disabled;
    u8 pal_count = script_enabled == script_disabled ? 1 : 2;
    struct gfx_texdesc td;
    if (hud_script_to_texdesc(&td, script_enabled, ICONS_ITEMS_ROM_START, pal_count)) {
        if (safe) {
            return gfx_texture_load(&td, NULL);
        } else {
            for (s32 i = 0; i < ARRAY_LENGTH(item_textures); i++) {
                if (item_textures[i].address == td.file_vaddr) {
                    item_textures[item].address = td.file_vaddr;
                    item_textures[item].texture = item_textures[i].texture;
                    return item_textures[item].texture;
                }
            }
            item_textures[item].texture = gfx_texture_load(&td, NULL);
            item_textures[item].address = td.file_vaddr;
            return item_textures[item].texture;
        }
    }
    return NULL;
}

struct gfx_texture *resource_load_pmicon_global(enum icon_global_offset icon_global_offset, s8 palette_count) {
    struct gfx_texdesc td;
    u32 *script = (u32 *)(SCRIPTS_GLOBAL_START + icon_global_offset);
    hud_script_to_texdesc(&td, script, 0x0, palette_count);
    return gfx_texture_load(&td, NULL);
}
