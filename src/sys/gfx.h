#ifndef GFX_H
#define GFX_H
#include "common.h"
#include "util/gu.h"
#include <n64/gbi.h>

#define GFX_FILE_DRAM    (-1)
#define GFX_FILE_DRAM_PM (-2)
// NOLINTBEGIN
#define gfxDisp(...)                                 \
    {                                                \
        Gfx gfxDisp__[] = {__VA_ARGS__};             \
        gfxDispAppend(gfxDisp__, sizeof(gfxDisp__)); \
    }
// NOLINTEND
#define GFX_TEXT_NORMAL 0
#define GFX_TEXT_FAST   1

enum GfxMode {
    GFX_MODE_FILTER,
    GFX_MODE_COMBINE,
    GFX_MODE_COLOR,
    GFX_MODE_DROPSHADOW,
    GFX_MODE_TEXT,
    GFX_MODE_ALL,
};

struct GfxTexdesc {
    g_ifmt_t imFmt;
    g_isiz_t imSiz;
    u32 address;
    s16 tileWidth;
    s16 tileHeight;
    s16 tilesX;
    s16 tilesY;
    u32 fileVaddr;
    size_t fileVsize;
    s8 palCount;
};

struct GfxTexldr {
    u32 fileVaddr;
    void *fileData;
};

struct GfxTexture {
    g_ifmt_t imFmt;
    g_isiz_t imSiz;
    void *data;
    s16 tileWidth;
    s16 tileHeight;
    s16 tilesX;
    s16 tilesY;
    size_t tileSize;
    s8 palCount;
};

struct GfxSprite {
    struct GfxTexture *texture;
    s16 textureTile;
    s8 paletteIndex;
    f32 x;
    f32 y;
    f32 xScale;
    f32 yScale;
};

struct GfxFont {
    struct GfxTexture *texture;
    s16 charWidth;
    s16 charHeight;
    s16 charsXtile;
    s16 charsYtile;
    u8 codeStart;
    s16 letterSpacing;
    s16 lineSpacing;
    s16 baseline;
    s16 median;
    s16 x;
};

void gfxStart(void);
void gfxModeInit(void);
void gfxModeConfigure(enum GfxMode mode, u64 value);
void gfxModeApply(enum GfxMode mode);
void gfxModeSet(enum GfxMode mode, u64 value);
void gfxModePush(enum GfxMode mode);
void gfxModePop(enum GfxMode mode);
void gfxModeReplace(enum GfxMode mode, u64 value);
/* all sizes are specified in number of bytes */
Gfx *gfxDispAppend(Gfx *disp, size_t size);
void *gfxDataAppend(void *data, size_t size);
void gfxFlush(void);

void gfxTexldrInit(struct GfxTexldr *texldr);
struct GfxTexture *gfxTexldrLoad(struct GfxTexldr *texldr, const struct GfxTexdesc *texdesc,
                                 struct GfxTexture *texture);
void gfxTexldrDestroy(struct GfxTexldr *texldr);

struct GfxTexture *gfxTextureCreate(g_ifmt_t imFmt, g_isiz_t imSiz, s32 tileWidth, s32 tileHeight, s32 tilesX,
                                    s32 tilesY);
struct GfxTexture *gfxTextureLoad(const struct GfxTexdesc *texdesc, struct GfxTexture *texture);
void gfxTextureDestroy(struct GfxTexture *texture);
void gfxTextureFree(struct GfxTexture *texture);
void *gfxTextureData(const struct GfxTexture *texture, s16 image);
struct GfxTexture *gfxTextureCopy(const struct GfxTexture *src, struct GfxTexture *dest);
void gfxTextureCopyTile(struct GfxTexture *dest, s32 destTile, const struct GfxTexture *src, s32 srcTile, bool blend);
void gfxTextureColortransform(struct GfxTexture *texture, const MtxF *matrix);
void gfxTextureMirrorHorizontal(struct GfxTexture *texture, s16 tile);
void gfxTextureTranslate(struct GfxTexture *texture, s16 tile, s32 xOffset, s32 yOffset);
void gfxAddGrayscalePalette(struct GfxTexture *texture, s8 basePaletteIndex);

void gfxDispRdpLoadTile(Gfx **disp, const struct GfxTexture *texture, s16 textureTile, s8 paletteIndex);
void gfxRdpLoadTile(const struct GfxTexture *texture, s16 textureTile, s8 paletteIndex);

void gfxSpriteDraw(const struct GfxSprite *sprite);

void gfxRectangleDraw(s16 x, s16 y, s16 width, s16 height, u32 color);
void gfxTextBackgroundDraw(s16 x, s16 y, u8 chCountW, u8 chCountH, s32 chWidth, s32 chHeight, u32 color);

s32 gfxFontXheight(const struct GfxFont *font);
void gfxPrintf(const struct GfxFont *font, s32 x, s32 y, const char *format, ...);
void gfxPrintfN(const struct GfxFont *font, s32 x, s32 y, const char *format, ...);
void gfxPrintfF(const struct GfxFont *font, s32 x, s32 y, const char *format, ...);

extern const MtxF gfxCmDesaturate;

#endif
