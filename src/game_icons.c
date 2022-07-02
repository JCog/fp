#include <stdlib.h>
#include "game_icons.h"
#include "util.h"

#define ICON_QUEUE_SIZE                     200
#define RASTER_CACHE_SIZE                   300
#define PALETTE_CACHE_SIZE                  300

#define HS_PTR(sym)                         (s32) & sym

#define hs_End                              HUD_ELEMENT_OP_End,
#define hs_SetRGBA(time, image)             HUD_ELEMENT_OP_SetRGBA, time, (s32)image,
#define hs_SetCI(time, raster, palette)     HUD_ELEMENT_OP_SetCI, time, raster, palette,
#define hs_Restart                          HUD_ELEMENT_OP_Restart,
#define hs_Loop                             HUD_ELEMENT_OP_Loop,
#define hs_SetTileSize(size)                HUD_ELEMENT_OP_SetTileSize, size,
#define hs_SetSizesAutoScale(size1, size2)  HUD_ELEMENT_OP_SetSizesAutoScale, size1, size2,
#define hs_SetSizesFixedScale(size1, size2) HUD_ELEMENT_OP_SetSizesFixedScale, size1, size2,
#define hs_SetVisible                       HUD_ELEMENT_OP_SetVisible,
#define hs_SetHidden                        HUD_ELEMENT_OP_SetHidden,
#define hs_AddTexelOffsetX(x)               HUD_ELEMENT_OP_AddTexelOffsetX, x,
#define hs_AddTexelOffsetY(y)               HUD_ELEMENT_OP_AddTexelOffsetY, y,
#define hs_SetTexelOffset(x, y)             HUD_ELEMENT_OP_SetTexelOffset, x, y,
#define hs_SetIcon(time, icon)              HUD_ELEMENT_OP_SetImage, time, ICON_##icon##_raster, ICON_##icon##_palette, 0, 0,
#define hs_SetScale(scale)                  HUD_ELEMENT_OP_SetScale, (s32)(scale * 65536.0f),
#define hs_SetAlpha(alpha)                  HUD_ELEMENT_OP_SetAlpha, alpha,
#define hs_RandomDelay(arg0, arg1)          HUD_ELEMENT_OP_RandomDelay, arg0, arg1,
#define hs_Delete                           HUD_ELEMENT_OP_Delete,
#define hs_UseIA8                           HUD_ELEMENT_OP_UseIA8,
#define hs_SetCustomSize(arg0, arg1)        HUD_ELEMENT_OP_SetCustomSize, arg0, arg1,

/// Restarts the loop if cutoff < rand_int(max)
#define hs_RandomRestart(max, cutoff)       HUD_ELEMENT_OP_RandomRestart, max, cutoff,

#define hs_op_15(arg0)                      HUD_ELEMENT_OP_op_15, arg0,
#define hs_RandomBranch(...)                HUD_ELEMENT_OP_RandomBranch, (sizeof((s32[]){__VA_ARGS__}) / sizeof(s32)), __VA_ARGS__,
#define hs_SetFlags(arg0)                   HUD_ELEMENT_OP_SetFlags, arg0,
#define hs_ClearFlags(arg0)                 HUD_ELEMENT_OP_ClearFlags, arg0,
#define hs_PlaySound(arg0)                  HUD_ELEMENT_OP_PlaySound, arg0,
#define hs_SetPivot(arg0, arg1)             HUD_ELEMENT_OP_SetPivot, arg0, arg1,

// clang-format off
/// Basic HudScript used for static CI images, setting size with hs_SetTileSize
#define HES_TEMPLATE_CI_ENUM_SIZE(raster, palette, sizeX, sizeY) {  \
    hs_SetVisible                                                   \
    hs_SetTileSize(HUD_ELEMENT_SIZE_##sizeX##x##sizeY)              \
        hs_Loop hs_SetCI(60, raster, palette)                       \
    hs_Restart                                                      \
    hs_End                                                          \
}

/// Basic HudScript used for static CI images, setting size with hs_SetCustomSize
#define HES_TEMPLATE_CI_CUSTOM_SIZE(raster, palette, sizeX, sizeY) { \
    hs_SetVisible                                                    \
    hs_SetCustomSize(sizeX, sizeY)                                   \
    hs_Loop                                                          \
        hs_SetCI(60, raster, palette)                                \
    hs_Restart                                                       \
    hs_End                                                           \
}
// clang-format on

typedef enum {
    SCRIPT_TYPE_GLOBAL,
    SCRIPT_TYPE_ITEM,
    SCRIPT_TYPE_PARTNER
} hud_script_type;

typedef enum {
    CACHE_RASTER,
    CACHE_PALETTE
} cache_type;

typedef struct image_cache_entry {
    s32 id;
    void *image;
} image_cache_entry;

static game_icon *game_icon_queue[ICON_QUEUE_SIZE];
static struct image_cache_entry raster_cache[RASTER_CACHE_SIZE];
static struct image_cache_entry palette_cache[PALETTE_CACHE_SIZE];
static u16 raster_cache_oldest_id;
static u16 palette_cache_oldest_id;

static u8 *get_cache_entry_image(u32 image_address, u32 image_size, cache_type cache_type);
static u8 *get_cached_raster(u32 addr, u32 size);
static u8 *get_cached_palette(u32 addr);
static void free_hud_script(game_icon *icon);
static HudScript *create_hud_script(hud_script_type type, s32 data[]);
static void game_icon_set_flags(game_icon *icon, s32 flags);
static void game_icon_clear_flags(game_icon *icon, s32 flags);
static void game_icon_script_init(game_icon *icon, HudScript *script);
static s32 game_icon_update(game_icon *icon);
static game_icon *game_icon_create(game_icon *icon, HudScript *hud_script);
static game_icon *game_icon_init(HudScript *hud_script, s32 x, s32 y, u8 alpha, f32 scale);

static void game_icons_draw_rect(game_icon *icon, Gfx *gfx_disp_p, s16 texSizeX, s16 texSizeY, s16 drawSizeX,
                                 s16 drawSizeY, s16 offsetX, s16 offsetY, s32 clamp, s32 dropShadow);

// image_address can be either ROM or RAM
static u8 *get_cache_entry_image(u32 image_address, u32 image_size, cache_type cache_type) {
    // simply return the address if it's a pointer to RAM
    if (image_address >> 24 == 0x80) {
        return (u8 *)image_address;
    }

    image_cache_entry *cache;
    u16 *cache_oldest_id;
    u16 cache_size;
    if (cache_type == CACHE_RASTER) {
        cache = raster_cache;
        cache_oldest_id = &raster_cache_oldest_id;
        cache_size = RASTER_CACHE_SIZE;
    } else {
        cache = palette_cache;
        cache_oldest_id = &palette_cache_oldest_id;
        cache_size = PALETTE_CACHE_SIZE;
    }

    s32 idx;
    for (idx = 0; idx < cache_size; idx++) {
        // return cached data
        if (cache[idx].id == image_address) {
            return cache[idx].image;
        }

        // load in data and place in cache
        if (cache[idx].id == 0) {
            cache[idx].id = image_address;
            cache[idx].image = malloc(image_size);
            nuPiReadRom(image_address, cache[idx].image, image_size);
            return cache[idx].image;
        }
    }

    // no cache hits, replace oldest item
    idx = (*cache_oldest_id)++;
    if (*cache_oldest_id == cache_size) {
        *cache_oldest_id = 0;
    }
    cache[idx].id = image_address;
    free(cache[idx].image);
    cache[idx].image = malloc(image_size);
    nuPiReadRom(image_address, cache[idx].image, image_size);
    return cache[idx].image;
}

static u8 *get_cached_raster(u32 addr, u32 size) {
    u8 *entry = get_cache_entry_image(addr, size, CACHE_RASTER);
    return entry;
}

static u8 *get_cached_palette(u32 addr) {
    return get_cache_entry_image(addr, ICON_PALETTE_SIZE, CACHE_PALETTE);
}

static void free_hud_script(game_icon *icon) {
    if ((uintptr_t)icon->anim > 0x80400000) {
        free(icon->anim);
        icon->anim = NULL;
    }
}

/**
 * Many of the built-in scripts access data from RAM that we can't count on existing at any given time, so it's
 * beneficial to create our own scripts sometimes, though not always. This function takes care of deciding that.
 *
 * data[] requirements:
 * SCRIPT_TYPE_GLOBAL:
 *  0: offset
 *
 * SCRIPT_TYPE_PARTNER:
 *  0: partner_index
 *  1: grayscale
 *
 *  SCRIPT_TYPE_ITEM:
 *  0: item_index
 *  1: grayscale
 */
static HudScript *create_hud_script(hud_script_type type, s32 data[]) {
    switch (type) {
        case SCRIPT_TYPE_GLOBAL: {
            s32 offset = data[0];
            return (HudScript *)(SCRIPTS_GLOBAL_START + offset);
        }
        case SCRIPT_TYPE_PARTNER: {
            s32 partner_index = data[0];
            s32 grayscale = data[1];

            s32 raster_addr = ICONS_PARTNERS_ROM_START +
                              (gHudElementSizes[HUD_ELEMENT_SIZE_32x32].size + ICON_PALETTE_SIZE * 2) * partner_index;
            s32 palette_addr = raster_addr + gHudElementSizes[HUD_ELEMENT_SIZE_32x32].size;
            if (grayscale) {
                palette_addr += ICON_PALETTE_SIZE;
            }
            s32 script_array[] = HES_TEMPLATE_CI_ENUM_SIZE(raster_addr, palette_addr, 32, 32);
            HudScript *result_script = malloc(sizeof(script_array));
            memcpy(result_script, script_array, sizeof(script_array));
            return result_script;
        }
        case SCRIPT_TYPE_ITEM: {
            s32 item_index = data[0];
            s32 grayscale = data[1];

            ItemData *item_data = &gItemTable[item_index];
            if (grayscale) {
                return gItemHudScripts[item_data->hudElemID].disabled;
            } else {
                return gItemHudScripts[item_data->hudElemID].enabled;
            }
        }
        default: return NULL;
    }
}

void game_icons_set_render_pos(game_icon *icon, s32 x, s32 y) {
    icon->renderPosX = x;
    icon->renderPosY = y;
}
void game_icons_set_scale(game_icon *icon, f32 scale) {
    s32 drawSizeX;
    s32 drawSizeY;
    s32 imgSizeX;
    s32 imgSizeY;
    f32 xScaled, yScaled;

    icon->uniformScale = scale;
    if (!(icon->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
        imgSizeX = gHudElementSizes[icon->tileSizePreset].width;
        imgSizeY = gHudElementSizes[icon->tileSizePreset].height;
        drawSizeX = gHudElementSizes[icon->drawSizePreset].width;
        drawSizeY = gHudElementSizes[icon->drawSizePreset].height;
    } else {
        imgSizeX = icon->customImageSize.x;
        imgSizeY = icon->customImageSize.y;
        drawSizeX = icon->customDrawSize.x;
        drawSizeY = icon->customDrawSize.y;
    }
    icon->sizeX = drawSizeX * scale;
    icon->sizeY = drawSizeY * scale;
    icon->flags &= ~HUD_ELEMENT_FLAGS_FIXEDSCALE;
    icon->flags |= HUD_ELEMENT_FLAGS_REPEATED | HUD_ELEMENT_FLAGS_SCALED;

    xScaled = ((f32)drawSizeX / (f32)imgSizeX) * scale;
    yScaled = ((f32)drawSizeY / (f32)imgSizeY) * scale;

    xScaled = 1.0f / xScaled;
    yScaled = 1.0f / yScaled;

    icon->widthScale = X10(xScaled);
    icon->heightScale = X10(yScaled);
}

void game_icons_set_alpha(game_icon *icon, s32 opacity) {
    icon->flags |= HUD_ELEMENT_FLAGS_TRANSPARENT;
    icon->opacity = opacity;

    if (opacity == 255) {
        icon->flags &= ~HUD_ELEMENT_FLAGS_TRANSPARENT;
    }
}

void game_icons_set_tint(game_icon *icon, u8 r, u8 g, u8 b) {
    icon->tint.r = r;
    icon->tint.g = g;
    icon->tint.b = b;
}

static void game_icon_set_flags(game_icon *icon, s32 flags) {
    icon->flags |= flags;
}

static void game_icon_clear_flags(game_icon *icon, s32 flags) {
    icon->flags &= ~flags;
}

static void game_icon_script_init(game_icon *icon, HudScript *script) {
    s32 *script_step = (s32 *)script;
    s32 raster;
    s32 palette;
    s32 preset;
    struct image_cache_entry *entry;

    if (script_step == NULL) {
        // PRINTF("attempted to initialize icon, but script was NULL\n");
        return;
    }

    preset = 0;

    while (1) {
        switch (*script_step++) {
            case HUD_ELEMENT_OP_End: return;
            case HUD_ELEMENT_OP_SetCI: script_step += 3; break;
            case HUD_ELEMENT_OP_SetTileSize:
                preset = *script_step++;
                icon->drawSizePreset = icon->tileSizePreset = preset;
                break;
            case HUD_ELEMENT_OP_SetSizesAutoScale:
            case HUD_ELEMENT_OP_SetSizesFixedScale:
                preset = *script_step;
                script_step += 2;
                icon->drawSizePreset = icon->tileSizePreset = preset;
                break;
            case HUD_ELEMENT_OP_SetCustomSize:
                icon->customDrawSize.x = icon->customImageSize.x = *script_step++;
                icon->customDrawSize.y = icon->customImageSize.y = *script_step++;
                icon->flags |= HUD_ELEMENT_FLAGS_CUSTOM_SIZE;
                break;
            case HUD_ELEMENT_OP_AddTexelOffsetX:
            case HUD_ELEMENT_OP_AddTexelOffsetY:
            case HUD_ELEMENT_OP_SetScale:
            case HUD_ELEMENT_OP_SetAlpha:
            case HUD_ELEMENT_OP_op_15:
            case HUD_ELEMENT_OP_RandomBranch:
            case HUD_ELEMENT_OP_SetFlags:
            case HUD_ELEMENT_OP_ClearFlags:
            case HUD_ELEMENT_OP_PlaySound: script_step++; break;
            case HUD_ELEMENT_OP_SetRGBA:
            case HUD_ELEMENT_OP_SetTexelOffset:
            case HUD_ELEMENT_OP_RandomDelay:
            case HUD_ELEMENT_OP_RandomRestart:
            case HUD_ELEMENT_OP_SetPivot: script_step += 2; break;
            case HUD_ELEMENT_OP_SetImage: script_step += 5; break;
        }
    }
}

static s32 game_icon_update(game_icon *icon) {
    s32 drawSizePreset;
    s32 tileSizePreset;
    u8 sizePreset;
    f32 xScaled, yScaled;
    s32 imageWidth, imageHeight, drawWidth, drawHeight;
    u32 min, max;
    u32 flags;
    s32 s1, s2;
    s32 arg1, arg2;
    f32 uniformScale;
    HudScript *newReadPos;

    game_icon_transform *hudTransform = icon->hudTransform;
    s32 *script_step = (s32 *)icon->readPos;
    if (script_step == NULL) {
        // PRINTF("game_icon at 0x%8X returned NULL readPos\n", icon);
        return 0;
    }

    switch (*script_step++) {
        case HUD_ELEMENT_OP_End:
            icon->updateTimer = 60;
            flags = icon->flags;
            icon->flags = flags | HUD_ELEMENT_FLAGS_ANIMATION_FINISHED;
            break;
        case HUD_ELEMENT_OP_Delete:
            icon->updateTimer = 60;
            icon->flags |= HUD_ELEMENT_FLAGS_DELETE;
            break;
        case HUD_ELEMENT_OP_UseIA8:
            icon->readPos = (HudScript *)script_step;
            icon->flags |= HUD_ELEMENT_FLAGS_FMT_IA8;
            return 1;
        case HUD_ELEMENT_OP_SetVisible:
            icon->readPos = (HudScript *)script_step;
            icon->flags |= HUD_ELEMENT_FLAGS_FMT_CI4;
            return 1;
        case HUD_ELEMENT_OP_SetHidden:
            icon->readPos = (HudScript *)script_step;
            icon->flags &= ~HUD_ELEMENT_FLAGS_FMT_CI4;
            return 1;
        case HUD_ELEMENT_OP_SetFlags:
            s1 = *script_step++;
            icon->readPos = (HudScript *)script_step;
            icon->flags |= s1;
            return 1;
        case HUD_ELEMENT_OP_ClearFlags:
            s1 = *script_step++;
            icon->readPos = (HudScript *)script_step;
            icon->flags &= ~s1;
            return 1;
        case HUD_ELEMENT_OP_SetRGBA:
            icon->updateTimer = *script_step++;
            icon->rasterAddr = (u8 *)*script_step++;
            icon->readPos = (HudScript *)script_step;

            if (icon->flags & HUD_ELEMENT_FLAGS_MEMOFFSET) {
                icon->rasterAddr += icon->memOffset;
            }

            if (icon->flags & HUD_ELEMENT_FLAGS_FIXEDSCALE) {
                if (!(icon->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
                    tileSizePreset = icon->tileSizePreset;
                    drawSizePreset = icon->drawSizePreset;
                    imageWidth = gHudElementSizes[tileSizePreset].width;
                    imageHeight = gHudElementSizes[tileSizePreset].height;
                    drawWidth = gHudElementSizes[drawSizePreset].width;
                    drawHeight = gHudElementSizes[drawSizePreset].height;
                } else {
                    imageWidth = icon->customImageSize.x;
                    imageHeight = icon->customImageSize.y;
                    drawWidth = icon->customDrawSize.x;
                    drawHeight = icon->customDrawSize.y;
                }

                if (!(icon->flags & HUD_ELEMENT_FLAGS_200)) {
                    icon->flags |= HUD_ELEMENT_FLAGS_200;
                    icon->unkImgScale[0] = imageWidth;
                    icon->unkImgScale[1] = imageHeight;
                    icon->deltaSizeX = ((f32)drawWidth - (f32)imageWidth) / (f32)icon->updateTimer;
                    icon->deltaSizeY = ((f32)drawHeight - (f32)imageHeight) / (f32)icon->updateTimer;
                } else {
                    icon->flags &= ~HUD_ELEMENT_FLAGS_200;
                    icon->unkImgScale[0] = drawWidth;
                    icon->unkImgScale[1] = drawHeight;
                    icon->deltaSizeX = ((f32)imageWidth - (f32)drawWidth) / (f32)icon->updateTimer;
                    icon->deltaSizeY = ((f32)imageHeight - (f32)drawHeight) / (f32)icon->updateTimer;
                }
            }
            break;
        case HUD_ELEMENT_OP_SetCI:
            icon->updateTimer = *script_step++;

            if (icon->flags & HUD_ELEMENT_FLAGS_FIXEDSCALE) {}
            s32 raster_rom = (s32)*script_step++;
            s32 palette_rom = (s32)*script_step++;
            s32 raster_size;
            if (icon->flags & HUD_ELEMENT_FLAGS_MEMOFFSET) {
                raster_rom += icon->memOffset;
                palette_rom += icon->memOffset;
            }
            if (icon->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE) {
                raster_size = (icon->customImageSize.x * icon->customImageSize.y) / 2;
            } else {
                raster_size = gHudElementSizes[icon->drawSizePreset].size;
            }
            icon->rasterAddr = get_cached_raster(raster_rom, raster_size);
            icon->paletteAddr = get_cached_palette(palette_rom);

            icon->readPos = (HudScript *)script_step;

            if (icon->flags & HUD_ELEMENT_FLAGS_FIXEDSCALE) {
                if (!(icon->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
                    tileSizePreset = icon->tileSizePreset;
                    drawSizePreset = icon->drawSizePreset;
                    imageWidth = gHudElementSizes[tileSizePreset].width;
                    imageHeight = gHudElementSizes[tileSizePreset].height;
                    drawWidth = gHudElementSizes[drawSizePreset].width;
                    drawHeight = gHudElementSizes[drawSizePreset].height;
                } else {
                    imageWidth = icon->customImageSize.x;
                    imageHeight = icon->customImageSize.y;
                    drawWidth = icon->customDrawSize.x;
                    drawHeight = icon->customDrawSize.y;
                }

                if (!(icon->flags & HUD_ELEMENT_FLAGS_200)) {
                    icon->flags |= HUD_ELEMENT_FLAGS_200;
                    icon->unkImgScale[0] = imageWidth;
                    icon->unkImgScale[1] = imageHeight;
                    icon->deltaSizeX = ((f32)drawWidth - (f32)imageWidth) / (f32)icon->updateTimer;
                    icon->deltaSizeY = ((f32)drawHeight - (f32)imageHeight) / (f32)icon->updateTimer;
                } else {
                    icon->flags &= ~HUD_ELEMENT_FLAGS_200;
                    icon->unkImgScale[0] = drawWidth;
                    icon->unkImgScale[1] = drawHeight;
                    icon->deltaSizeX = ((f32)imageWidth - (f32)drawWidth) / (f32)icon->updateTimer;
                    icon->deltaSizeY = ((f32)imageHeight - (f32)drawHeight) / (f32)icon->updateTimer;
                }
            }
            break;
        case HUD_ELEMENT_OP_SetImage:
            icon->updateTimer = *script_step++;

            icon->rasterAddr =
                get_cached_raster(ICONS_ITEMS_ROM_START + *script_step++, gHudElementSizes[icon->tileSizePreset].size);
            icon->paletteAddr = get_cached_palette(ICONS_ITEMS_ROM_START + *script_step++);
            script_step += 3;
            icon->readPos = (HudScript *)script_step;

            if (icon->flags & HUD_ELEMENT_FLAGS_FIXEDSCALE) {
                if (!(icon->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
                    tileSizePreset = icon->tileSizePreset;
                    drawSizePreset = icon->drawSizePreset;
                    imageWidth = gHudElementSizes[tileSizePreset].width;
                    imageHeight = gHudElementSizes[tileSizePreset].height;
                    drawWidth = gHudElementSizes[drawSizePreset].width;
                    drawHeight = gHudElementSizes[drawSizePreset].height;
                } else {
                    imageWidth = icon->customImageSize.x;
                    imageHeight = icon->customImageSize.y;
                    drawWidth = icon->customDrawSize.x;
                    drawHeight = icon->customDrawSize.y;
                }

                if (!(icon->flags & HUD_ELEMENT_FLAGS_200)) {
                    icon->flags |= HUD_ELEMENT_FLAGS_200;
                    icon->unkImgScale[0] = imageWidth;
                    icon->unkImgScale[1] = imageHeight;
                    icon->deltaSizeX = ((f32)drawWidth - (f32)imageWidth) / (f32)icon->updateTimer;
                    icon->deltaSizeY = ((f32)drawHeight - (f32)imageHeight) / (f32)icon->updateTimer;
                } else {
                    icon->flags &= ~HUD_ELEMENT_FLAGS_200;
                    icon->unkImgScale[0] = drawWidth;
                    icon->unkImgScale[1] = drawHeight;
                    icon->deltaSizeX = ((f32)imageWidth - (f32)drawWidth) / (f32)icon->updateTimer;
                    icon->deltaSizeY = ((f32)imageHeight - (f32)drawHeight) / (f32)icon->updateTimer;
                }
            }
            break;
        case HUD_ELEMENT_OP_Restart: icon->readPos = icon->loopStartPos; return 1;
        case HUD_ELEMENT_OP_Loop:
            icon->loopStartPos = (HudScript *)script_step;
            icon->readPos = (HudScript *)script_step;
            return 1;
        case HUD_ELEMENT_OP_RandomRestart:
            s1 = *script_step++;
            s2 = *script_step++;
            if (pm_player.timer % s1 < s2) {
                icon->readPos = icon->loopStartPos;
            } else {
                icon->readPos = (HudScript *)script_step;
            }
            return 1;
        case HUD_ELEMENT_OP_SetTileSize:
            sizePreset = *script_step++;
            icon->widthScale = X10(1);
            icon->heightScale = X10(1);
            icon->readPos = (HudScript *)script_step;
            icon->drawSizePreset = sizePreset;
            icon->tileSizePreset = sizePreset;
            icon->flags &= ~HUD_ELEMENT_FLAGS_FIXEDSCALE;
            icon->flags &= ~HUD_ELEMENT_FLAGS_REPEATED;
            return 1;
        case HUD_ELEMENT_OP_SetSizesAutoScale:
            tileSizePreset = *script_step++;
            arg2 = *script_step++;

            icon->readPos = (HudScript *)script_step;
            icon->tileSizePreset = tileSizePreset;
            icon->drawSizePreset = arg2;

            imageWidth = gHudElementSizes[tileSizePreset].width;
            imageHeight = gHudElementSizes[tileSizePreset].height;
            drawWidth = gHudElementSizes[arg2].width;
            drawHeight = gHudElementSizes[arg2].height;

            xScaled = (f32)drawWidth / (f32)imageWidth;
            yScaled = (f32)drawHeight / (f32)imageHeight;

            xScaled = 1.0f / xScaled;
            yScaled = 1.0f / yScaled;

            icon->widthScale = X10(xScaled);
            icon->heightScale = X10(yScaled);

            icon->flags &= ~HUD_ELEMENT_FLAGS_FIXEDSCALE;
            icon->flags |= HUD_ELEMENT_FLAGS_REPEATED;
            return 1;
        case HUD_ELEMENT_OP_SetSizesFixedScale:
            tileSizePreset = *script_step++;
            drawSizePreset = *script_step++;

            icon->widthScale = X10(1);
            icon->heightScale = X10(1);
            icon->readPos = (HudScript *)script_step;
            icon->tileSizePreset = tileSizePreset;
            icon->drawSizePreset = drawSizePreset;
            icon->flags |= HUD_ELEMENT_FLAGS_FIXEDSCALE;
            icon->flags &= ~HUD_ELEMENT_FLAGS_REPEATED;
            icon->flags &= ~HUD_ELEMENT_FLAGS_200;
            return 1;
        case HUD_ELEMENT_OP_AddTexelOffsetX:
            s1 = *script_step++;
            icon->readPos = (HudScript *)script_step;
            icon->screenPosOffset.x += s1;
            return 1;
        case HUD_ELEMENT_OP_AddTexelOffsetY:
            s2 = *script_step++;
            if (!(icon->flags & HUD_ELEMENT_FLAGS_FLIPY)) {
                icon->screenPosOffset.y += s2;
            } else {
                icon->screenPosOffset.y -= s2;
            }
            icon->readPos = (HudScript *)script_step;
            return 1;
        case HUD_ELEMENT_OP_SetTexelOffset:
            s1 = *script_step++;
            s2 = *script_step++;
            icon->screenPosOffset.x = s1;
            if (!(icon->flags & HUD_ELEMENT_FLAGS_FLIPY)) {
                icon->screenPosOffset.y = s2;
            } else {
                icon->screenPosOffset.y = -s2;
            }
            icon->readPos = (HudScript *)script_step;
            return 1;
        case HUD_ELEMENT_OP_SetScale:
            uniformScale = (f32)*script_step++;
            uniformScale /= 65536;
            icon->uniformScale = uniformScale;
            if (!(icon->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
                imageWidth = gHudElementSizes[icon->tileSizePreset].width;
                imageHeight = gHudElementSizes[icon->tileSizePreset].height;
                drawWidth = gHudElementSizes[icon->drawSizePreset].width;
                drawHeight = gHudElementSizes[icon->drawSizePreset].height;
            } else {
                imageWidth = icon->customImageSize.x;
                imageHeight = icon->customImageSize.y;
                drawWidth = icon->customDrawSize.x;
                drawHeight = icon->customDrawSize.y;
            }

            icon->sizeX = drawWidth * uniformScale;
            icon->sizeY = drawHeight * uniformScale;

            xScaled = (f32)drawWidth / (f32)imageWidth * uniformScale;
            yScaled = (f32)drawHeight / (f32)imageHeight * uniformScale;

            xScaled = 1.0f / xScaled;
            yScaled = 1.0f / yScaled;

            icon->widthScale = X10(xScaled);
            icon->heightScale = X10(yScaled);

            icon->readPos = (HudScript *)script_step;
            icon->flags &= ~HUD_ELEMENT_FLAGS_FIXEDSCALE;
            icon->flags |= HUD_ELEMENT_FLAGS_REPEATED | HUD_ELEMENT_FLAGS_SCALED;
            return 1;
        case HUD_ELEMENT_OP_SetAlpha:
            s1 = *script_step++;
            icon->opacity = s1;
            icon->flags |= HUD_ELEMENT_FLAGS_TRANSPARENT;
            if (icon->opacity == 255) {
                icon->flags &= ~HUD_ELEMENT_FLAGS_TRANSPARENT;
            }
            icon->readPos = (HudScript *)script_step;
            return 1;
        case HUD_ELEMENT_OP_RandomDelay:
            s1 = *script_step++;
            s2 = *script_step++;
            icon->updateTimer = pm_player.timer % (s2 - s1) + s1;
            icon->readPos = (HudScript *)script_step;
            break;
        case HUD_ELEMENT_OP_SetCustomSize:
            icon->customDrawSize.x = icon->customImageSize.x = *script_step++;
            icon->customDrawSize.y = icon->customImageSize.y = *script_step++;
            icon->readPos = (HudScript *)script_step;
            icon->widthScale = X10(1);
            icon->heightScale = X10(1);
            icon->drawSizePreset = 0;
            icon->tileSizePreset = 0;
            icon->flags &= ~HUD_ELEMENT_FLAGS_FIXEDSCALE;
            icon->flags &= ~HUD_ELEMENT_FLAGS_REPEATED;
            icon->flags |= HUD_ELEMENT_FLAGS_CUSTOM_SIZE;
            return 1;
        case HUD_ELEMENT_OP_op_15:
            s1 = *script_step++;
            icon->readPos = (HudScript *)script_step;
            icon->flags &= ~(HUD_ELEMENT_FLAGS_1000000 | HUD_ELEMENT_FLAGS_2000000 | HUD_ELEMENT_FLAGS_4000000 |
                             HUD_ELEMENT_FLAGS_8000000);
            icon->flags |= s1 << 24;
            return 1;
        case HUD_ELEMENT_OP_RandomBranch:
            // TODO: fix
            // s1 = *script_step++;
            // newReadPos = (HudScript *)script_step[pm_player.timer % (s1 - 1)];
            // icon->readPos = newReadPos;
            // hud_element_load_script(icon, newReadPos);
            return 1;
        case HUD_ELEMENT_OP_PlaySound:
            // yeah we're not doing this lol
            icon->readPos = (HudScript *)script_step++;
            return 1;
        case HUD_ELEMENT_OP_SetPivot:
            arg1 = *script_step++;
            arg2 = *script_step++;
            icon->readPos = (HudScript *)script_step;
            if (icon->flags & HUD_ELEMENT_FLAGS_TRANSFORM) {
                hudTransform->pivot.x = arg1;
                // decomp needed to match
                do {
                    hudTransform->pivot.y = arg2;
                } while (0);
            }
            return 1;
        case HUD_ELEMENT_OP_op_16: break;
    }

    return 0;
}

static game_icon *game_icon_create(game_icon *icon, HudScript *hud_script) {
    icon->flags = HUD_ELEMENT_FLAGS_INITIALIZED;
    icon->readPos = hud_script;
    icon->updateTimer = 1;
    icon->drawSizePreset = -1;
    icon->tileSizePreset = -1;
    icon->renderPosX = 0;
    icon->renderPosY = 0;
    icon->loopStartPos = hud_script;
    icon->widthScale = X10(1.0f);
    icon->heightScale = X10(1.0f);
    icon->anim = icon->readPos;
    icon->uniformScale = 1.0f;
    icon->screenPosOffset.x = 0;
    icon->screenPosOffset.y = 0;
    icon->worldPosOffset.x = 0;
    icon->worldPosOffset.y = 0;
    icon->worldPosOffset.z = 0;
    icon->opacity = 255;
    icon->tint.r = 255;
    icon->tint.g = 255;
    icon->tint.b = 255;

    game_icon_script_init(icon, icon->readPos);
    while (game_icon_update(icon) != 0) {};
    return icon;
}

static game_icon *game_icon_init(HudScript *hud_script, s32 x, s32 y, u8 alpha, f32 scale) {
    game_icon *icon = malloc(sizeof(*icon));
    icon = game_icon_create(icon, hud_script);
    game_icons_set_render_pos(icon, x, y);
    game_icons_set_alpha(icon, alpha);
    game_icons_set_scale(icon, scale);
    return icon;
}

game_icon *game_icons_create_global(icon_global icon, s32 x, s32 y, u8 alpha, f32 scale) {
    s32 script_data[] = {icon};
    HudScript *hud_script = create_hud_script(SCRIPT_TYPE_GLOBAL, script_data);
    return game_icon_init(hud_script, x, y, alpha, scale);
}

game_icon *game_icons_create_partner(Partner partner, s32 x, s32 y, u8 alpha, f32 scale, _Bool grayscale) {
    u8 partner_index;
    switch (partner) {
        case PARTNER_GOOMBARIO: partner_index = 1; break;
        case PARTNER_KOOPER: partner_index = 2; break;
        case PARTNER_BOMBETTE: partner_index = 3; break;
        case PARTNER_PARAKARRY: partner_index = 4; break;
        case PARTNER_WATT: partner_index = 6; break;
        case PARTNER_SUSHIE: partner_index = 7; break;
        case PARTNER_LAKILESTER: partner_index = 8; break;
        case PARTNER_BOW: partner_index = 5; break;
        default: partner_index = 0; break;
    }

    s32 script_data[] = {partner_index, grayscale};
    HudScript *hud_script = create_hud_script(SCRIPT_TYPE_PARTNER, script_data);

    return game_icon_init(hud_script, x, y, alpha, scale);
}

game_icon *game_icons_create_item(Item item, s32 x, s32 y, u8 alpha, f32 scale, _Bool grayscale) {
    s32 script_data[] = {item, grayscale};
    HudScript *hud_script = create_hud_script(SCRIPT_TYPE_ITEM, script_data);

    return game_icon_init(hud_script, x, y, alpha, scale);
}

void game_icons_draw(game_icon *icon) {
    for (s32 i = 0; i < ICON_QUEUE_SIZE; i++) {
        if (game_icon_queue[i] == NULL) {
            game_icon_queue[i] = icon;
            return;
        }
    }
}

void game_icons_delete(game_icon *icon) {
    game_icon_set_flags(icon, HUD_ELEMENT_FLAGS_DELETE);
}

game_icon *game_icons_update_next() {
    static u16 idx = 0;
    game_icon *next_icon = NULL;
    while (idx < ICON_QUEUE_SIZE) {
        if (game_icon_queue[idx] == NULL) {
            // no more icons in queue
            break;
        } else {
            if (game_icon_queue[idx]->flags != 0 && !(game_icon_queue[idx]->flags & HUD_ELEMENT_FLAGS_DISABLED)) {
                if (game_icon_queue[idx]->flags & HUD_ELEMENT_FLAGS_DELETE) {
                    // icon flagged for deletion, free script if we created it before freeing the hud_element
                    free_hud_script(game_icon_queue[idx]);
                    free(game_icon_queue[idx]);
                    game_icon_queue[idx] = NULL;
                } else if (game_icon_queue[idx]->readPos != NULL) {
                    game_icon_queue[idx]->updateTimer--;
                    if (game_icon_queue[idx]->updateTimer == 0) {
                        while (game_icon_update(game_icon_queue[idx]) != 0) {};
                    }
                    if (game_icon_queue[idx]->flags & HUD_ELEMENT_FLAGS_FIXEDSCALE) {
                        game_icon_queue[idx]->unkImgScale[0] += game_icon_queue[idx]->deltaSizeX;
                        game_icon_queue[idx]->unkImgScale[1] += game_icon_queue[idx]->deltaSizeY;
                    }
                    next_icon = game_icon_queue[idx];
                }
            }
        }
        game_icon_queue[idx] = NULL;
        idx++;
        if (next_icon) {
            return next_icon;
        }
    }
    idx = 0;
    return NULL;
}
