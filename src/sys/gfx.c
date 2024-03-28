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
static Gfx *gfxDisp;
static Gfx *gfxDispW;
static Gfx *gfxDispP;
static Gfx *gfxDispD;
static u32 *fpDispPos;

#define GFX_STACK_LENGTH 8
static u64 gfxModes[GFX_MODE_ALL];
static u64 gfxModeStack[GFX_MODE_ALL][GFX_STACK_LENGTH];
static s32 gfxModeStackPos[GFX_MODE_ALL];
static bool gfxSynced;

#define CHAR_TILE_MAX 8
struct GfxChar {
    s32 tileChar;
    u32 color;
    s32 x;
    s32 y;
};
static const struct GfxFont *gfxCharFont;
static struct vector gfxChars[CHAR_TILE_MAX];

static void drawChars(const struct GfxFont *font, s32 x, s32 y, const char *buf, size_t l);
static void flushChars(void);
static void gfxPrintfNVa(const struct GfxFont *font, s32 x, s32 y, const char *format, va_list args);
static void gfxPrintfFVa(const struct GfxFont *font, s32 x, s32 y, const char *format, va_list args);

static inline void gfxSync(void) {
    if (!gfxSynced) {
        gDPPipeSync(gfxDispP++);
        gfxSynced = TRUE;
    }
}

const MtxF gfxCmDesaturate = guDefMtxF(0.3086f, 0.6094f, 0.0820f, 0.f, 0.3086f, 0.6094f, 0.0820f, 0.f, 0.3086f, 0.6094f,
                                       0.0820f, 0.f, 0.f, 0.f, 0.f, 1.f);

void gfxStart(void) {
    for (s32 i = 0; i < CHAR_TILE_MAX; ++i) {
        vector_init(&gfxChars[i], sizeof(struct GfxChar));
    }
    gfxDisp = malloc(GFX_DISP_SIZE);
    gfxDispW = malloc(GFX_DISP_SIZE);
    gfxDispP = gfxDisp;
    gfxDispD = gfxDisp + (GFX_DISP_SIZE + sizeof(*gfxDisp) - 1) / sizeof(*gfxDisp);
}

void gfxModeInit(void) {
    gfxSync();
    gSPLoadGeometryMode(gfxDispP++, 0);
    gDPSetCycleType(gfxDispP++, G_CYC_1CYCLE);
    gDPSetRenderMode(gfxDispP++, G_RM_XLU_SURF, G_RM_XLU_SURF2);
    gDPSetScissor(gfxDispP++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    gDPSetAlphaDither(gfxDispP++, G_AD_DISABLE);
    gDPSetColorDither(gfxDispP++, G_CD_DISABLE);
    gDPSetAlphaCompare(gfxDispP++, G_AC_NONE);
    gDPSetDepthSource(gfxDispP++, G_ZS_PRIM);
    gDPSetCombineKey(gfxDispP++, G_CK_NONE);
    gDPSetTextureConvert(gfxDispP++, G_TC_FILT);
    gDPSetTextureDetail(gfxDispP++, G_TD_CLAMP);
    gDPSetTexturePersp(gfxDispP++, G_TP_NONE);
    gDPSetTextureLOD(gfxDispP++, G_TL_TILE);
    gDPSetTextureLUT(gfxDispP++, G_TT_NONE);
    gDPPipelineMode(gfxDispP++, G_PM_NPRIMITIVE);
    gfxModeApply(GFX_MODE_ALL);
}

void gfxModeConfigure(enum GfxMode mode, u64 value) {
    gfxModes[mode] = value;
}

void gfxModeApply(enum GfxMode mode) {
    Gfx dl[GFX_MODE_ALL];
    Gfx *pdl = dl;
    switch (mode) {
        case GFX_MODE_ALL:
        case GFX_MODE_FILTER: {
            gDPSetTextureFilter(pdl++, gfxModes[GFX_MODE_FILTER]);
            if (mode != GFX_MODE_ALL) {
                break;
            }
        }
        case GFX_MODE_COMBINE: {
            gDPSetCombine(pdl++, gfxModes[GFX_MODE_COMBINE]);
            if (mode != GFX_MODE_ALL) {
                break;
            }
        }
        case GFX_MODE_COLOR: {
            u32 c = gfxModes[GFX_MODE_COLOR];
            gDPSetPrimColor(pdl++, 0, 0, (c >> 24) & 0xFF, (c >> 16) & 0xFF, (c >> 8) & 0xFF, (c >> 0) & 0xFF);
            if (mode != GFX_MODE_ALL) {
                break;
            }
        }
        default: break;
    }
    size_t s = pdl - dl;
    if (s > 0) {
        gfxSync();
        memcpy(gfxDispP, dl, s * sizeof(*dl));
        gfxDispP += s;
    }
}

void gfxModeSet(enum GfxMode mode, u64 value) {
    gfxModeConfigure(mode, value);
    gfxModeApply(mode);
}

void gfxModePush(enum GfxMode mode) {
    if (mode == GFX_MODE_ALL) {
        for (s32 i = 0; i < GFX_MODE_ALL; ++i) {
            s32 *p = &gfxModeStackPos[i];
            gfxModeStack[i][*p] = gfxModes[i];
            *p = (*p + 1) % GFX_STACK_LENGTH;
        }
    } else {
        s32 *p = &gfxModeStackPos[mode];
        gfxModeStack[mode][*p] = gfxModes[mode];
        *p = (*p + 1) % GFX_STACK_LENGTH;
    }
}

void gfxModePop(enum GfxMode mode) {
    if (mode == GFX_MODE_ALL) {
        for (s32 i = 0; i < GFX_MODE_ALL; ++i) {
            s32 *p = &gfxModeStackPos[i];
            *p = (*p + GFX_STACK_LENGTH - 1) % GFX_STACK_LENGTH;
            gfxModeSet(i, gfxModeStack[i][*p]);
        }
    } else {
        s32 *p = &gfxModeStackPos[mode];
        *p = (*p + GFX_STACK_LENGTH - 1) % GFX_STACK_LENGTH;
        gfxModeSet(mode, gfxModeStack[mode][*p]);
    }
}

void gfxModeReplace(enum GfxMode mode, u64 value) {
    gfxModePush(mode);
    gfxModeConfigure(mode, value);
    gfxModeApply(mode);
}

Gfx *gfxDispAppend(Gfx *disp, size_t size) {
    Gfx *p = gfxDispP;
    memcpy(gfxDispP, disp, size);
    gfxDispP += (size + sizeof(*gfxDispP) - 1) / sizeof(*gfxDispP);
    gfxSynced = FALSE;
    return p;
}

void *gfxDataAppend(void *data, size_t size) {
    gfxDispD -= (size + sizeof(*gfxDispD) - 1) / sizeof(*gfxDispD);
    memcpy(gfxDispD, data, size);
    return gfxDispD;
}

void gfxFlush(s32 pendingFrames) {
    flushChars();
    gSPEndDisplayList(gfxDispP++);
    if (pendingFrames) {
        fpDispPos = ((u32 *)pm_masterGfxPos + 1);
        gSPDisplayList(pm_masterGfxPos++, gfxDisp);
    } else {
        *fpDispPos = (u32)gfxDisp;
    }
    Gfx *disp_w = gfxDispW;
    gfxDispW = gfxDisp;
    gfxDisp = disp_w;
    gfxDispP = gfxDisp;
    gfxDispD = gfxDisp + (GFX_DISP_SIZE + sizeof(*gfxDisp) - 1) / sizeof(*gfxDisp);
    gfxSynced = FALSE;
}

void gfxTexldrInit(struct GfxTexldr *texldr) {
    texldr->fileVaddr = GFX_FILE_DRAM;
    texldr->fileData = NULL;
}

static s32 getTextureTileRasterSize(const struct GfxTexture *texture) {
    return texture->tileWidth * texture->tileHeight * G_SIZ_BITS(texture->imSiz) / 8;
}

static bool textureDataIsOnHeap(struct GfxTexture *texture) {
    return (uintptr_t)texture->data >= 0x80400000;
}

struct GfxTexture *gfxTexldrLoad(struct GfxTexldr *texldr, const struct GfxTexdesc *texdesc,
                                 struct GfxTexture *texture) {
    struct GfxTexture *newTexture = NULL;
    if (!texture) {
        newTexture = malloc(sizeof(*newTexture));
        if (!newTexture) {
            return newTexture;
        }
        texture = newTexture;
    }
    texture->imFmt = texdesc->imFmt;
    texture->imSiz = texdesc->imSiz;
    texture->tileWidth = texdesc->tileWidth;
    texture->tileHeight = texdesc->tileHeight;
    texture->tilesX = texdesc->tilesX;
    texture->tilesY = texdesc->tilesY;
    texture->tileSize = (getTextureTileRasterSize(texture) + ICON_PALETTE_SIZE * texdesc->palCount + 7) / 8 * 8;
    texture->palCount = texdesc->palCount;
    size_t textureSize = texture->tileSize * texture->tilesX * texture->tilesY;
    void *textureData = NULL;
    void *fileStart = NULL;
    if (texdesc->fileVaddr == GFX_FILE_DRAM_PM) {
        // these textures are always loaded in memory by the base game, so no need to waste memory copying them
        textureData = (void *)texdesc->address;
        texture->tileSize = getTextureTileRasterSize(texture) + ICON_PALETTE_SIZE * texdesc->palCount;
    } else if (texdesc->fileVaddr != GFX_FILE_DRAM) {
        if (texldr->fileVaddr != texdesc->fileVaddr) {
            if (texldr->fileData) {
                free(texldr->fileData);
            }
            texldr->fileData = malloc(texdesc->fileVsize);
            if (!texldr->fileData) {
                texldr->fileVaddr = GFX_FILE_DRAM;
                if (newTexture) {
                    free(newTexture);
                }
                return NULL;
            }
            texldr->fileVaddr = texdesc->fileVaddr;
            nuPiReadRom(texldr->fileVaddr, texldr->fileData, texdesc->fileVsize);
        }
        if (texdesc->fileVsize == textureSize) {
            textureData = texldr->fileData;
            texldr->fileVaddr = GFX_FILE_DRAM;
            texldr->fileData = NULL;
        } else {
            fileStart = texldr->fileData;
        }
    }
    if (!textureData) {
        textureData = malloc(textureSize);
        if (!textureData) {
            if (newTexture) {
                free(newTexture);
            }
            return NULL;
        }
        memcpy(textureData, (char *)fileStart + texdesc->address, textureSize);
    }
    texture->data = textureData;
    return texture;
}

void gfxTexldrDestroy(struct GfxTexldr *texldr) {
    if (texldr->fileData) {
        free(texldr->fileData);
    }
}

struct GfxTexture *gfxTextureCreate(g_ifmt_t imFmt, g_isiz_t imSiz, s32 tileWidth, s32 tileHeight, s32 tilesX,
                                    s32 tilesY) {
    struct GfxTexture *texture = malloc(sizeof(*texture));
    if (!texture) {
        return texture;
    }
    texture->tileSize = (tileWidth * tileHeight * G_SIZ_BITS(imSiz) + 63) / 64 * 8;
    texture->data = memalign(64, tilesX * tilesY * texture->tileSize);
    if (!texture->data) {
        free(texture);
        return NULL;
    }
    texture->imFmt = imFmt;
    texture->imSiz = imSiz;
    texture->tileWidth = tileWidth;
    texture->tileHeight = tileHeight;
    texture->tilesX = tilesX;
    texture->tilesY = tilesY;
    return texture;
}

struct GfxTexture *gfxTextureLoad(const struct GfxTexdesc *texdesc, struct GfxTexture *texture) {
    struct GfxTexldr texldr;
    gfxTexldrInit(&texldr);
    texture = gfxTexldrLoad(&texldr, texdesc, texture);
    gfxTexldrDestroy(&texldr);
    return texture;
}

void gfxTextureDestroy(struct GfxTexture *texture) {
    if (texture->data && textureDataIsOnHeap(texture)) {
        free(texture->data);
    }
}

void gfxTextureFree(struct GfxTexture *texture) {
    gfxTextureDestroy(texture);
    free(texture);
}

void *gfxTextureData(const struct GfxTexture *texture, s16 image) {
    return (char *)texture->data + texture->tileSize * image;
}

struct GfxTexture *gfxTextureCopy(const struct GfxTexture *src, struct GfxTexture *dest) {
    struct GfxTexture *newTexture = NULL;
    if (!dest) {
        newTexture = malloc(sizeof(*newTexture));
        if (!newTexture) {
            return newTexture;
        }
        dest = newTexture;
    }
    size_t textureSize = src->tileSize * src->tilesX * src->tilesY;
    void *textureData = memalign(64, textureSize);
    if (!textureData) {
        if (newTexture) {
            free(newTexture);
        }
        return NULL;
    }
    *dest = *src;
    dest->data = textureData;
    memcpy(dest->data, src->data, textureSize);
    return dest;
}

void gfxTextureCopyTile(struct GfxTexture *dest, s32 destTile, const struct GfxTexture *src, s32 srcTile, bool blend) {
    if (src->imFmt != G_IM_FMT_RGBA || src->imSiz != G_IM_SIZ_32b || dest->imFmt != src->imFmt ||
        dest->imSiz != src->imSiz || dest->tileWidth != src->tileWidth || dest->tileHeight != src->tileHeight) {
        return;
    }
    struct RGBA32 {
        u8 r;
        u8 g;
        u8 b;
        u8 a;
    };
    size_t tilePixels = src->tileWidth * src->tileHeight;
    struct RGBA32 *pDest = gfxTextureData(dest, destTile);
    struct RGBA32 *pSrc = gfxTextureData(src, srcTile);
    for (size_t i = 0; i < tilePixels; ++i) {
        if (blend) {
            pDest->r = pDest->r + (pSrc->r - pDest->r) * pSrc->a / 0xFF;
            pDest->g = pDest->g + (pSrc->g - pDest->g) * pSrc->a / 0xFF;
            pDest->b = pDest->b + (pSrc->b - pDest->b) * pSrc->a / 0xFF;
            pDest->a = pSrc->a + (0xFF - pSrc->a) * pDest->a / 0xFF;
        } else {
            *pDest = *pSrc;
        }
        ++pDest;
        ++pSrc;
    }
}

void gfxTextureColortransform(struct GfxTexture *texture, const MtxF *matrix) {
    if (texture->imFmt != G_IM_FMT_RGBA || texture->imSiz != G_IM_SIZ_32b) {
        return;
    }
    struct RGBA32 {
        u8 r;
        u8 g;
        u8 b;
        u8 a;
    };
    size_t texturePixels = texture->tileWidth * texture->tileHeight * texture->tilesX * texture->tilesY;
    struct RGBA32 *pixelData = texture->data;
    MtxF m = *matrix;
    for (size_t i = 0; i < texturePixels; ++i) {
        struct RGBA32 p = pixelData[i];
        f32 r = p.r * m.xx + p.g * m.xy + p.b * m.xz + p.a * m.xw;
        f32 g = p.r * m.yx + p.g * m.yy + p.b * m.yz + p.a * m.yw;
        f32 b = p.r * m.zx + p.g * m.zy + p.b * m.zz + p.a * m.zw;
        f32 a = p.r * m.wx + p.g * m.wy + p.b * m.wz + p.a * m.ww;
        struct RGBA32 n = {
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
        pixelData[i] = n;
    }
}

void gfxTextureMirrorHorizontal(struct GfxTexture *texture, s16 tile) {
    s32 adjWidth = texture->tileWidth;
    s32 bytesPerPixel = G_SIZ_BITS(texture->imSiz) / 8;
    if (bytesPerPixel == 0) {
        bytesPerPixel = 1; // 0.5, but we'll handle it manually
        adjWidth /= 2;
    }

    if (!textureDataIsOnHeap(texture)) {
        void *newData = malloc(texture->tilesX * texture->tilesY * texture->tileSize);
        memcpy(newData, texture->data, sizeof(*newData));
        texture->data = newData;
    }

    unsigned char *newRaster = malloc(getTextureTileRasterSize(texture));
    unsigned char *newPos = newRaster;
    unsigned char *oldRaster = texture->data + tile * texture->tileSize;
    unsigned char *oldPos;
    for (s32 y = 0; y < texture->tileHeight; y++) {
        s32 yOffset = bytesPerPixel * (texture->tileHeight * y + texture->tileWidth - 1);
        if (texture->imSiz == G_IM_SIZ_4b) {
            yOffset /= 2;
        }
        oldPos = oldRaster + yOffset;
        for (s32 x = 0; x < adjWidth; x++) {
            memcpy(newPos, oldPos, bytesPerPixel);
            if (texture->imSiz == G_IM_SIZ_4b) {
                *newPos = (*newPos & 0xF0) >> 4 | (*newPos & 0x0F) << 4;
            }
            newPos += bytesPerPixel;
            oldPos -= bytesPerPixel;
        }
    }
    memcpy(oldRaster, newRaster, getTextureTileRasterSize(texture));
    free(newRaster);
}

void gfxTextureTranslate(struct GfxTexture *texture, s16 tile, s32 xOffset, s32 yOffset) {
    s32 adjWidth = texture->tileWidth;
    s32 adjHeight = texture->tileHeight;
    s32 bytesPerPixel = G_SIZ_BITS(texture->imSiz) / 8;
    s32 adjXOffset = xOffset;
    if (bytesPerPixel == 0) {
        bytesPerPixel = 1; // 0.5, but we'll handle it manually
        adjWidth /= 2;
        adjHeight /= 2;
        adjXOffset /= 2;
    }

    if (!textureDataIsOnHeap(texture)) {
        void *newData = malloc(texture->tilesX * texture->tilesY * texture->tileSize);
        memcpy(newData, texture->data, sizeof(*newData));
        texture->data = newData;
    }

    if (xOffset != 0) {
        unsigned char *newRaster = calloc(getTextureTileRasterSize(texture), 1);
        unsigned char *newPos;
        unsigned char *oldRaster = texture->data + tile * texture->tileSize;
        unsigned char *oldPos;
        for (s32 y = 0; y < texture->tileHeight; y++) {
            newPos = newRaster + bytesPerPixel * (adjHeight * y);
            oldPos = oldRaster + bytesPerPixel * (adjHeight * y - adjXOffset);
            for (s32 x = 0; x < adjWidth; x++) {
                if (x >= adjXOffset && x < adjWidth + adjXOffset) {
                    if (texture->imSiz == G_IM_SIZ_4b && (xOffset % 2 == 1 || xOffset % 2 == -1)) {
                        if (xOffset > 0) {
                            *newPos = (*oldPos & 0xF0) >> 4;
                            if (x > adjXOffset) {
                                *newPos |= (*(oldPos - 1) & 0x0F) << 4;
                            }
                        } else {
                            *newPos = (*oldPos & 0x0F) << 4;
                            if (x < adjWidth + adjXOffset - 1) {
                                *newPos |= (*(oldPos + 1) & 0xF0) >> 4;
                            }
                        }
                    } else {
                        memcpy(newPos, oldPos, bytesPerPixel);
                    }
                }
                newPos += bytesPerPixel;
                oldPos += bytesPerPixel;
            }
        }
        memcpy(oldRaster, newRaster, getTextureTileRasterSize(texture));
        free(newRaster);
    }
    if (yOffset != 0) {
        unsigned char *newRaster = calloc(getTextureTileRasterSize(texture), 1);
        unsigned char *newPos;
        unsigned char *oldRaster = texture->data + tile * texture->tileSize;
        unsigned char *oldPos;
        for (s32 y = 0; y < texture->tileHeight; y++) {
            if (y >= yOffset && y < texture->tileHeight + yOffset) {
                newPos = newRaster + bytesPerPixel * (adjHeight * y);
                oldPos = oldRaster + bytesPerPixel * (adjHeight * y - yOffset * adjWidth);
                for (s32 x = 0; x < adjWidth; x++) {
                    memcpy(newPos, oldPos, bytesPerPixel);
                    newPos += bytesPerPixel;
                    oldPos += bytesPerPixel;
                }
            }
        }
        memcpy(oldRaster, newRaster, getTextureTileRasterSize(texture));
        free(newRaster);
    }
}

void gfxAddGrayscalePalette(struct GfxTexture *texture, s8 basePaletteIndex) {
    if (texture->imFmt != G_IM_FMT_CI || texture->imSiz != G_IM_SIZ_4b) {
        return;
    }
    typedef struct {
        u16 r : 5;
        u16 g : 5;
        u16 b : 5;
        u16 a : 1;
    } RGBA;

    s32 tileCount = texture->tilesX * texture->tilesY;
    char *newTextureData = malloc((texture->tileSize + ICON_PALETTE_SIZE) * tileCount);
    size_t newTileSize = texture->tileSize + ICON_PALETTE_SIZE;

    u32 rasterSize = getTextureTileRasterSize(texture);

    for (s32 iTile = 0; iTile < tileCount; iTile++) {
        char *baseTile = texture->data + texture->tileSize * iTile;
        char *newTile = newTextureData + newTileSize * iTile;
        memcpy(newTile, baseTile, texture->tileSize);

        RGBA *basePalette = (RGBA *)(baseTile + rasterSize + ICON_PALETTE_SIZE * basePaletteIndex);
        RGBA *newPalette = (RGBA *)(newTextureData + rasterSize + ICON_PALETTE_SIZE * texture->palCount);
        u8 pixelCount = ICON_PALETTE_SIZE / 2;
        for (u32 iPixel = 0; iPixel < pixelCount; iPixel++) {
            RGBA *oldPixel = &basePalette[iPixel];
            RGBA *newPixel = &((RGBA *)newPalette)[iPixel];

            f32 lum = 0.2782f * oldPixel->r + 0.6562f * oldPixel->g + 0.0656f * oldPixel->b;
            u16 gray = (lum * 14 / 31) + 12;
            newPixel->r = gray;
            newPixel->g = gray;
            newPixel->b = gray;
            newPixel->a = oldPixel->a;
        }
    }

    if (textureDataIsOnHeap(texture)) {
        free(texture->data);
    }
    texture->data = newTextureData;
    texture->tileSize = newTileSize;
    texture->palCount++;
}

void gfxDispRdpLoadTile(Gfx **disp, const struct GfxTexture *texture, s16 textureTile, s8 paletteIndex) {
    if (texture->imFmt == G_IM_FMT_CI) {
        gDPSetTextureLUT((*disp)++, G_TT_RGBA16);
        gDPLoadTLUT_pal16((*disp)++, 0,
                          gfxTextureData(texture, textureTile) + getTextureTileRasterSize(texture) +
                              ICON_PALETTE_SIZE * paletteIndex);
    } else {
        gDPSetTextureLUT((*disp)++, G_TT_NONE);
    }
    if (texture->imSiz == G_IM_SIZ_4b) {
        gDPLoadTextureTile_4b((*disp)++, gfxTextureData(texture, textureTile), texture->imFmt, texture->tileWidth,
                              texture->tileHeight, 0, 0, texture->tileWidth - 1, texture->tileHeight - 1, 0,
                              G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMASK, G_TX_NOMASK,
                              G_TX_NOLOD, G_TX_NOLOD);
    } else {
        gDPLoadTextureTile((*disp)++, gfxTextureData(texture, textureTile), texture->imFmt, texture->imSiz,
                           texture->tileWidth, texture->tileHeight, 0, 0, texture->tileWidth - 1,
                           texture->tileHeight - 1, 0, G_TX_NOMIRROR | G_TX_WRAP, G_TX_NOMIRROR | G_TX_WRAP,
                           G_TX_NOMASK, G_TX_NOMASK, G_TX_NOLOD, G_TX_NOLOD);
    }
}

void gfxRdpLoadTile(const struct GfxTexture *texture, s16 textureTile, s8 paletteIndex) {
    gfxDispRdpLoadTile(&gfxDispP, texture, textureTile, paletteIndex);
    gfxSynced = TRUE;
}

void gfxSpriteDraw(const struct GfxSprite *sprite) {
    struct GfxTexture *texture = sprite->texture;
    gfxRdpLoadTile(texture, sprite->textureTile, sprite->paletteIndex);
    if (gfxModes[GFX_MODE_DROPSHADOW]) {
        u8 a = gfxModes[GFX_MODE_COLOR] & 0xFF;
        a = a * a / 0xFF;
        gfxModeReplace(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0x00, 0x00, a));
        gSPScisTextureRectangle(gfxDispP++, qs102(sprite->x + 1) & ~3, qs102(sprite->y + 1) & ~3,
                                qs102(sprite->x + texture->tileWidth * sprite->xScale + 1) & ~3,
                                qs102(sprite->y + texture->tileHeight * sprite->yScale + 1) & ~3, G_TX_RENDERTILE,
                                qu105(0), qu105(0), qu510(1.f / sprite->xScale), qu510(1.f / sprite->yScale));
        gfxModePop(GFX_MODE_COLOR);
    }
    gfxSync();
    gSPScisTextureRectangle(gfxDispP++, qs102(sprite->x) & ~3, qs102(sprite->y) & ~3,
                            qs102(sprite->x + texture->tileWidth * sprite->xScale) & ~3,
                            qs102(sprite->y + texture->tileHeight * sprite->yScale) & ~3, G_TX_RENDERTILE, qu105(0),
                            qu105(0), qu510(1.f / sprite->xScale), qu510(1.f / sprite->yScale));
    gfxSynced = FALSE;
}

s32 gfxFontXheight(const struct GfxFont *font) {
    return font->baseline - font->median;
}

void gfxPrintf(const struct GfxFont *font, s32 x, s32 y, const char *format, ...) {
    if (gfxModes[GFX_MODE_TEXT] == GFX_TEXT_NORMAL) {
        va_list args;
        va_start(args, format);
        gfxPrintfNVa(font, x, y, format, args);
        va_end(args);
    } else if (gfxModes[GFX_MODE_TEXT] == GFX_TEXT_FAST) {
        va_list args;
        va_start(args, format);
        gfxPrintfFVa(font, x, y, format, args);
        va_end(args);
    }
}

void gfxPrintfN(const struct GfxFont *font, s32 x, s32 y, const char *format, ...) {
    va_list args;
    va_start(args, format);
    gfxPrintfNVa(font, x, y, format, args);
    va_end(args);
}

void gfxPrintfF(const struct GfxFont *font, s32 x, s32 y, const char *format, ...) {
    va_list args;
    va_start(args, format);
    gfxPrintfFVa(font, x, y, format, args);
    va_end(args);
}

static void drawChars(const struct GfxFont *font, s32 x, s32 y, const char *buf, size_t l) {
    x -= font->x;
    y -= font->baseline;
    struct GfxTexture *texture = font->texture;
    s32 charsPerTile = font->charsXtile * font->charsYtile;
    s32 nTiles = texture->tilesX * texture->tilesY;
    s32 nChars = charsPerTile * nTiles;
    for (s32 i = 0; i < nTiles; ++i) {
        s32 tileBegin = charsPerTile * i;
        s32 tileEnd = tileBegin + charsPerTile;
        bool tileLoaded = FALSE;
        s32 cx = 0;
        s32 cy = 0;
        for (s32 j = 0; j < l; ++j, cx += font->charWidth + font->letterSpacing) {
            u8 c = buf[j];
            if (c < font->codeStart || c >= font->codeStart + nChars) {
                continue;
            }
            c -= font->codeStart;
            if (c < tileBegin || c >= tileEnd) {
                continue;
            }
            c -= tileBegin;
            if (!tileLoaded) {
                tileLoaded = TRUE;
                gfxRdpLoadTile(texture, i, 0);
            }
            gSPScisTextureRectangle(gfxDispP++, qs102(x + cx), qs102(y + cy), qs102(x + cx + font->charWidth),
                                    qs102(y + cy + font->charHeight), G_TX_RENDERTILE,
                                    qu105(c % font->charsXtile * font->charWidth),
                                    qu105(c / font->charsXtile * font->charHeight), qu510(1), qu510(1));
        }
    }
    gfxSynced = FALSE;
}

static void flushChars(void) {
    const struct GfxFont *font = gfxCharFont;
    u32 color = 0;
    bool first = TRUE;
    for (s32 i = 0; i < CHAR_TILE_MAX; ++i) {
        struct vector *tileVect = &gfxChars[i];
        for (s32 j = 0; j < tileVect->size; ++j) {
            struct GfxChar *gc = vector_at(tileVect, j);
            if (j == 0) {
                gfxRdpLoadTile(font->texture, i, 0);
            }
            if (first || color != gc->color) {
                color = gc->color;
                gfxSync();
                gDPSetPrimColor(gfxDispP++, 0, 0, (color >> 24) & 0xFF, (color >> 16) & 0xFF, (color >> 8) & 0xFF,
                                (color >> 0) & 0xFF);
            }
            first = FALSE;
            gSPScisTextureRectangle(gfxDispP++, qs102(gc->x), qs102(gc->y), qs102(gc->x + font->charWidth),
                                    qs102(gc->y + font->charHeight), G_TX_RENDERTILE,
                                    qu105(gc->tileChar % font->charsXtile * font->charWidth),
                                    qu105(gc->tileChar / font->charsXtile * font->charHeight), qu510(1), qu510(1));
            gfxSynced = FALSE;
        }
        vector_clear(tileVect);
    }
}

static void gfxPrintfNVa(const struct GfxFont *font, s32 x, s32 y, const char *format, va_list args) {
    const size_t bufsize = 1024;
    char buf[bufsize];
    s32 l = vsnprintf(buf, bufsize, format, args);
    if (l > bufsize - 1) {
        l = bufsize - 1;
    }
    if (gfxModes[GFX_MODE_DROPSHADOW]) {
        u8 a = gfxModes[GFX_MODE_COLOR] & 0xFF;
        a = a * a / 0xFF;
        gfxModeReplace(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0x00, 0x00, a));
        drawChars(font, x + 1, y + 1, buf, l);
        gfxModePop(GFX_MODE_COLOR);
    }
    drawChars(font, x, y, buf, l);
}

static void gfxPrintfFVa(const struct GfxFont *font, s32 x, s32 y, const char *format, va_list args) {
    const size_t bufsize = 1024;
    char buf[bufsize];
    s32 l = vsnprintf(buf, bufsize, format, args);
    if (l > bufsize - 1) {
        l = bufsize - 1;
    }
    x -= font->x;
    y -= font->baseline;
    struct GfxTexture *texture = font->texture;
    s32 charsPerTile = font->charsXtile * font->charsYtile;
    s32 nTiles = texture->tilesX * texture->tilesY;
    s32 nChars = charsPerTile * nTiles;
    if (gfxModes[GFX_MODE_DROPSHADOW]) {
        u8 a = gfxModes[GFX_MODE_COLOR] & 0xFF;
        a = a * a / 0xFF;
        u32 color = GPACK_RGBA8888(0x00, 0x00, 0x00, a);
        s32 cx = x + 1;
        s32 cy = y + 1;
        for (s32 i = 0; i < l; ++i, cx += font->charWidth + font->letterSpacing) {
            u8 c = buf[i];
            if (c < font->codeStart || c >= font->codeStart + nChars) {
                continue;
            }
            c -= font->codeStart;
            s32 tileIdx = c / charsPerTile;
            s32 tileChar = c % charsPerTile;
            struct GfxChar gc = {
                tileChar,
                color,
                cx,
                cy,
            };
            vector_push_back(&gfxChars[tileIdx], 1, &gc);
        }
    }
    s32 cx = x;
    s32 cy = y;
    for (s32 i = 0; i < l; ++i, cx += font->charWidth + font->letterSpacing) {
        u8 c = buf[i];
        if (c < font->codeStart || c >= font->codeStart + nChars) {
            continue;
        }
        c -= font->codeStart;
        s32 tileIdx = c / charsPerTile;
        s32 tileChar = c % charsPerTile;
        struct GfxChar gc = {
            tileChar,
            gfxModes[GFX_MODE_COLOR],
            cx,
            cy,
        };
        vector_push_back(&gfxChars[tileIdx], 1, &gc);
    }

    gfxCharFont = font;
}
