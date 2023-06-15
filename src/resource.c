#include "resource.h"
#include "gfx.h"
#include "gu.h"
#include <grc.h>
#include <stdlib.h>

#define VSIZE(x, y, im_size, palette_count, tile_count) \
    (x * y * G_SIZ_BITS(im_size) / 8 + ICON_PALETTE_SIZE * palette_count) * tile_count

struct ItemIconCacheEntry {
    u32 vaddr;
    struct GfxTexture *texture;
};
static struct ItemIconCacheEntry itemTextures[0x16D];

/* resource data table */
static void *resData[RES_MAX] = {NULL};

/* resource constructors */
static void *rcFontGeneric(struct GfxTexdesc *texdesc, s32 charWidth, s32 charHeight, s32 codeStart, s32 letterSpacing,
                           s32 lineSpacing, s32 baseline, s32 median, s32 x) {
    struct GfxFont *font = malloc(sizeof(*font));
    if (!font) {
        return font;
    }
    struct GfxTexture *texture = gfxTextureLoad(texdesc, NULL);
    if (!texture) {
        free(font);
        return NULL;
    }
    font->texture = texture;
    font->charWidth = charWidth;
    font->charHeight = charHeight;
    font->charsXtile = texture->tileWidth / font->charWidth;
    font->charsYtile = texture->tileHeight / font->charHeight;
    font->codeStart = codeStart;
    font->letterSpacing = letterSpacing;
    font->lineSpacing = lineSpacing;
    font->baseline = baseline;
    font->median = median;
    font->x = x;
    return font;
}

static void *rcGrcFontGeneric(const char *grcResourceName, s32 charWidth, s32 charHeight, s32 codeStart,
                              s32 letterSpacing, s32 lineSpacing, s32 baseline, s32 median, s32 x) {
    void *pT;
    grc_resource_get(grcResourceName, &pT, NULL);
    if (!pT) {
        return pT;
    }
    struct grc_texture *t = pT;
    struct GfxTexdesc td = {
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
    return rcFontGeneric(&td, charWidth, charHeight, codeStart, letterSpacing, lineSpacing, baseline, median, x);
}

static void *rcFontFipps(void) {
    return rcGrcFontGeneric("fipps", 10, 14, 33, -2, -5, 10, 3, 2);
}

static void *rcFontNotalot35(void) {
    return rcGrcFontGeneric("notalot35", 8, 9, 33, -1, -1, 7, 2, 2);
}

static void *rcFontOrigamimommy(void) {
    return rcGrcFontGeneric("origamimommy", 8, 10, 33, -2, -2, 8, 1, 0);
}

static void *rcFontPcsenior(void) {
    return rcGrcFontGeneric("pcsenior", 8, 8, 33, 0, 0, 7, 2, 0);
}

static void *rcFontPixelintv(void) {
    return rcGrcFontGeneric("pixelintv", 8, 12, 33, 0, -4, 10, 5, 1);
}

static void *rcFontPressstart2p(void) {
    return rcGrcFontGeneric("pressstart2p", 8, 8, 33, 0, 0, 7, 2, 0);
}

static void *rcFontSmwtextnc(void) {
    return rcGrcFontGeneric("smwtextnc", 12, 8, 33, -4, 0, 7, 2, 3);
}

static void *rcFontWerdnasreturn(void) {
    return rcGrcFontGeneric("werdnasreturn", 8, 12, 33, 0, -4, 11, 6, 1);
}

static void *rcFontPixelzim(void) {
    return rcGrcFontGeneric("pixelzim", 3, 6, 33, 1, 0, 5, 3, 0);
}

static s32 hudScriptToTexdesc(struct GfxTexdesc *tdOut, const u32 *hudScript, u32 vaddrOffset, s8 palCount) {
    g_ifmt_t im_fmt = G_IM_FMT_RGBA;
    g_isiz_t im_siz = G_IM_SIZ_32b;
    u32 address = UINT32_MAX;
    s16 tileWidth = 0;
    s16 tileHeight = 0;

    const u32 *scriptPos = hudScript;
    s32 tileSizePreset;
    bool scriptDone = 0;
    while (!scriptDone) {
        switch (*scriptPos++) {
            case HUD_ELEMENT_OP_End: scriptDone = TRUE; break;
            case HUD_ELEMENT_OP_SetCI:
                if (address == UINT32_MAX) {
                    im_fmt = G_IM_FMT_CI;
                    im_siz = G_IM_SIZ_4b;
                    scriptPos++;
                    address = *scriptPos++;
                    scriptPos++;
                } else {
                    scriptPos += 3;
                }
                break;
            case HUD_ELEMENT_OP_SetImage:
                if (address == UINT32_MAX) {
                    im_fmt = G_IM_FMT_CI;
                    im_siz = G_IM_SIZ_4b;
                    scriptPos++;
                    address = *scriptPos++;
                    scriptPos += 3;
                } else {
                    scriptPos += 5;
                }
                break;
            case HUD_ELEMENT_OP_UseIA8:
                im_fmt = G_IM_FMT_IA;
                im_siz = G_IM_SIZ_8b;
                break;
            case HUD_ELEMENT_OP_SetRGBA:
                if (address == UINT32_MAX) {
                    scriptPos++;
                    address = *scriptPos++;
                } else {
                    scriptPos += 2;
                }
                break;
            case HUD_ELEMENT_OP_SetTileSize:
                tileSizePreset = *scriptPos++;
                tileWidth = pm_gHudElementSizes[tileSizePreset].width;
                tileHeight = pm_gHudElementSizes[tileSizePreset].height;
                break;
            case HUD_ELEMENT_OP_SetCustomSize:
                tileWidth = *scriptPos++;
                tileHeight = *scriptPos++;
                break;
            case HUD_ELEMENT_OP_AddTexelOffsetX:
            case HUD_ELEMENT_OP_AddTexelOffsetY:
            case HUD_ELEMENT_OP_SetScale:
            case HUD_ELEMENT_OP_SetAlpha:
            case HUD_ELEMENT_OP_op_15:
            case HUD_ELEMENT_OP_RandomBranch:
            case HUD_ELEMENT_OP_SetFlags:
            case HUD_ELEMENT_OP_ClearFlags:
            case HUD_ELEMENT_OP_PlaySound: scriptPos++; break;
            case HUD_ELEMENT_OP_SetTexelOffset:
            case HUD_ELEMENT_OP_RandomDelay:
            case HUD_ELEMENT_OP_RandomRestart:
            case HUD_ELEMENT_OP_SetPivot: scriptPos += 2; break;
            case HUD_ELEMENT_OP_SetSizesAutoScale:
            case HUD_ELEMENT_OP_SetSizesFixedScale: scriptPos += 3; break;
        }
    }
    if (address == UINT32_MAX || tileWidth == 0 || tileHeight == 0) {
        PRINTF("error loading hud_script 0x%X\n addr: 0x%X, w: %d, h: %d\n\n", hudScript, address, tileWidth,
               tileHeight);
        return 0;
    }

    u32 fileVaddr;
    size_t fileVsize;
    if (address >> 24 == 0x80) {
        fileVaddr = GFX_FILE_DRAM_PM;
        fileVsize = 0;
    } else {
        fileVaddr = address + vaddrOffset;
        fileVsize = VSIZE(tileWidth, tileHeight, im_siz, palCount, 1);
    }

    *tdOut = (struct GfxTexdesc){
        im_fmt, im_siz, address, tileWidth, tileHeight, 1, 1, fileVaddr, fileVsize, palCount,
    };
    return 1;
}

static void *rcPmiconPartner(void) {
    struct GfxTexdesc td = {
        G_IM_FMT_CI, G_IM_SIZ_4b, 0, 32, 32, 1, 12, ICONS_PARTNERS_ROM_START, VSIZE(32, 32, G_IM_SIZ_4b, 2, 12), 2,
    };
    return gfxTextureLoad(&td, NULL);
}

static void *rcPmiconStarSpirits(void) {
    struct GfxTexdesc td = {
        G_IM_FMT_CI, G_IM_SIZ_4b, 0, 32, 32, 1, 9, ICONS_STAR_SPIRITS_ROM_START, VSIZE(32, 32, G_IM_SIZ_4b, 2, 9), 2,
    };
    return gfxTextureLoad(&td, NULL);
}

static void *rcPmiconBp(void) {
    struct GfxTexdesc td = {
        G_IM_FMT_CI, G_IM_SIZ_4b, 0, 16, 16, 1, 1, ICONS_BP_ROM_START, VSIZE(16, 16, G_IM_SIZ_4b, 1, 1), 1,
    };
    return gfxTextureLoad(&td, NULL);
}

static void *rcIconCheck(void) {
    return resourceLoadGrcTexture("check_icons");
}

static void *rcIconButtons(void) {
    return resourceLoadGrcTexture("button_icons");
}

static void *rcIconPause(void) {
    return resourceLoadGrcTexture("pause_icons");
}

static void *rcIconMacro(void) {
    return resourceLoadGrcTexture("macro_icons");
}

static void *rcIconMovie(void) {
    return resourceLoadGrcTexture("movie_icons");
}

static void *rcIconArrow(void) {
    return resourceLoadGrcTexture("arrow_icons");
}

static void *rcIconFile(void) {
    return resourceLoadGrcTexture("file_icons");
}

static void *rcIconSave(void) {
    return resourceLoadGrcTexture("save_icons");
}

static void *rcIconOsk(void) {
    return resourceLoadGrcTexture("osk_icons");
}

static void *rcTextureCrosshair(void) {
    return resourceLoadGrcTexture("crosshair");
}

static void *rcTextureControlStick(void) {
    return resourceLoadGrcTexture("control_stick");
}

/* resource destructors */
static void rdFontGeneric(void *data) {
    struct GfxFont *font = data;
    gfxTextureFree(font->texture);
    free(font);
}

/* resource management tables */
static void *(*resCtor[RES_MAX])(void) = {
    rcFontFipps,         rcFontNotalot35,    rcFontOrigamimommy,    rcFontPcsenior, rcFontPixelintv,
    rcFontPressstart2p,  rcFontSmwtextnc,    rcFontWerdnasreturn,   rcFontPixelzim, rcPmiconPartner,
    rcPmiconStarSpirits, rcPmiconBp,         rcIconCheck,           rcIconButtons,  rcIconPause,
    rcIconMacro,         rcIconMovie,        rcIconArrow,           rcIconFile,     rcIconSave,
    rcIconOsk,           rcTextureCrosshair, rcTextureControlStick,
};

static void (*resDtor[RES_MAX])() = {
    rdFontGeneric,  gfxTextureFree, gfxTextureFree, gfxTextureFree, gfxTextureFree,
    gfxTextureFree, gfxTextureFree, gfxTextureFree, gfxTextureFree, gfxTextureFree,
};

/* resource interface */
void *resourceGet(enum ResourceId res) {
    if (!resData[res]) {
        resData[res] = resCtor[res]();
    }
    return resData[res];
}

void resourceFree(enum ResourceId res) {
    if (resData[res]) {
        resDtor[res](resData[res]);
        resData[res] = NULL;
    }
}

struct GfxTexture *resourceLoadGrcTexture(const char *grcResourceName) {
    void *pT;
    grc_resource_get(grcResourceName, &pT, NULL);
    if (!pT) {
        return pT;
    }
    struct grc_texture *t = pT;
    struct GfxTexdesc td = {
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
    return gfxTextureLoad(&td, NULL);
}

struct GfxTexture *resourceLoadPmiconItem(u16 item, bool safe) {
    if (!safe && itemTextures[item].texture) {
        return itemTextures[item].texture;
    }
    pm_ItemData *itemData = &pm_gItemTable[item];
    u32 *scriptEnabled = (u32 *)pm_gItemHudScripts[itemData->hudElemID].enabled;
    u32 *scriptDisabled = (u32 *)pm_gItemHudScripts[itemData->hudElemID].disabled;
    u8 palCount = scriptEnabled == scriptDisabled ? 1 : 2;
    struct GfxTexdesc td;
    if (hudScriptToTexdesc(&td, scriptEnabled, ICONS_ITEMS_ROM_START, palCount)) {
        if (safe) {
            return gfxTextureLoad(&td, NULL);
        } else {
            for (s32 i = 0; i < ARRAY_LENGTH(itemTextures); i++) {
                if (itemTextures[i].vaddr == td.fileVaddr) {
                    itemTextures[item].vaddr = td.fileVaddr;
                    itemTextures[item].texture = itemTextures[i].texture;
                    return itemTextures[item].texture;
                }
            }
            itemTextures[item].texture = gfxTextureLoad(&td, NULL);
            itemTextures[item].vaddr = td.fileVaddr;
            return itemTextures[item].texture;
        }
    }
    return NULL;
}

struct GfxTexture *resourceLoadPmiconGlobal(enum IconGlobalOffset iconGlobalOffset, s8 paletteCount) {
    struct GfxTexdesc td;
    u32 *script = (u32 *)(SCRIPTS_GLOBAL_START + iconGlobalOffset);
    hudScriptToTexdesc(&td, script, 0x0, paletteCount);
    return gfxTextureLoad(&td, NULL);
}
