#include <stdlib.h>
#include "icons.h"
#include "util.h"

#define HUD_ELEMENT_QUEUE_SIZE              200
#define RASTER_CACHE_SIZE                   300
#define PALETTE_CACHE_SIZE                  300
#define CUSTOM_SCRIPT_LIST_SIZE             200

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

struct hud_cache_entry {
    s32 id;
    void *data;
};

struct element_script_pair {
    HudElement *hud_element; // used to id correct hud_script to free()
    HudScript *hud_script;
};

static u16 custom_hud_script_count;
static HudElement *hud_element_queue[HUD_ELEMENT_QUEUE_SIZE];
static struct hud_cache_entry raster_cache[RASTER_CACHE_SIZE];
static struct hud_cache_entry palette_cache[PALETTE_CACHE_SIZE];
static struct element_script_pair custom_hud_script_list[CUSTOM_SCRIPT_LIST_SIZE];

void icons_init() {
    custom_hud_script_count = 0;
    for (s32 i = 0; i < HUD_ELEMENT_QUEUE_SIZE; i++) {
        hud_element_queue[i] = NULL;
    }
    for (s32 i = 0; i < RASTER_CACHE_SIZE; i++) {
        raster_cache[i].id = 0;
        raster_cache[i].data = NULL;
    }
    for (s32 i = 0; i < PALETTE_CACHE_SIZE; i++) {
        palette_cache[i].id = 0;
        palette_cache[i].data = NULL;
    }
    for (s32 i = 0; i < CUSTOM_SCRIPT_LIST_SIZE; i++) {
        custom_hud_script_list[i].hud_element = NULL;
        custom_hud_script_list[i].hud_script = NULL;
    }
}

// image_address can be either ROM or RAM
u8 *get_cache_entry_image(u32 image_address, u32 image_size, struct hud_cache_entry *cache, u32 cache_size) {
    s32 idx;
    for (idx = 0; idx < cache_size; idx++) {
        // return cached data
        if (cache[idx].id == image_address) {
            return cache[idx].data;
        }

        // load in data and place in cache
        if (cache[idx].id == 0) {
            cache[idx].id = image_address;
            cache[idx].data = malloc(image_size);
            // dma if image_address is a pointer to ram
            if (image_address >> 24 == 0x80) {
                cache[idx].data = (void *)image_address;
            } else {
                nuPiReadRom(image_address, cache[idx].data, image_size);
            }
            return cache[idx].data;
        }
    }
    return NULL;
}

u8 *get_cached_raster(u32 addr, u32 size) {
    return get_cache_entry_image(addr, size, raster_cache, RASTER_CACHE_SIZE);
}

u8 *get_cached_palette(u32 addr) {
    return get_cache_entry_image(addr, ICON_PALETTE_SIZE, palette_cache, PALETTE_CACHE_SIZE);
}

void free_custom_hud_script(HudElement *hud_element) {
    for (s32 i = 0; i < CUSTOM_SCRIPT_LIST_SIZE; i++) {
        if (custom_hud_script_list[i].hud_element == hud_element) {
            free(custom_hud_script_list[i].hud_script);
            custom_hud_script_list[i].hud_element = NULL;
            custom_hud_script_list[i].hud_script = NULL;
            custom_hud_script_count--;
            return;
        }
    }
}

/*
 * Many of the built-in scripts access data from RAM that we can't count on existing at any given time, so it's
 * beneficial to create our own scripts sometimes, though not always. This function takes care of deciding that.
 *
 * data[] requirements:
 * SCRIPT_TYPE_GLOBAL:
 *  0: offset
 *
 * data[] requirements:
 * SCRIPT_TYPE_PARTNER:
 *  0: partner_index
 *  1: grayscale
 *
 *  SCRIPT_TYPE_ITEM:
 *  0: item_index
 *  1: grayscale
 */
HudScript *create_hud_script(hud_script_type type, s32 data[]) {
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

void hud_element_set_render_pos(HudElement *hud_element, s32 x, s32 y) {
    hud_element->renderPosX = x;
    hud_element->renderPosY = y;
}
void hud_element_set_scale(HudElement *hud_element, f32 scale) {
    s32 drawSizeX;
    s32 drawSizeY;
    s32 imgSizeX;
    s32 imgSizeY;
    f32 xScaled, yScaled;

    hud_element->uniformScale = scale;
    if (!(hud_element->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
        imgSizeX = gHudElementSizes[hud_element->tileSizePreset].width;
        imgSizeY = gHudElementSizes[hud_element->tileSizePreset].height;
        drawSizeX = gHudElementSizes[hud_element->drawSizePreset].width;
        drawSizeY = gHudElementSizes[hud_element->drawSizePreset].height;
    } else {
        imgSizeX = hud_element->customImageSize.x;
        imgSizeY = hud_element->customImageSize.y;
        drawSizeX = hud_element->customDrawSize.x;
        drawSizeY = hud_element->customDrawSize.y;
    }
    hud_element->sizeX = drawSizeX * scale;
    hud_element->sizeY = drawSizeY * scale;
    hud_element->flags &= ~HUD_ELEMENT_FLAGS_FIXEDSCALE;
    hud_element->flags |= HUD_ELEMENT_FLAGS_REPEATED | HUD_ELEMENT_FLAGS_SCALED;

    xScaled = ((f32)drawSizeX / (f32)imgSizeX) * scale;
    yScaled = ((f32)drawSizeY / (f32)imgSizeY) * scale;

    xScaled = 1.0f / xScaled;
    yScaled = 1.0f / yScaled;

    hud_element->widthScale = X10(xScaled);
    hud_element->heightScale = X10(yScaled);
}

void hud_element_set_alpha(HudElement *hudElement, s32 opacity) {
    hudElement->flags |= HUD_ELEMENT_FLAGS_TRANSPARENT;
    hudElement->opacity = opacity;

    if (opacity == 255) {
        hudElement->flags &= ~HUD_ELEMENT_FLAGS_TRANSPARENT;
    }
}

void hud_element_set_flags(HudElement *hud_element, s32 flags) {
    hud_element->flags |= flags;
}

void hud_element_clear_flags(HudElement *hud_element, s32 flags) {
    hud_element->flags &= ~flags;
}

void hud_element_script_init(HudElement *hud_element, HudScript *script) {
    s32 *script_step = (s32 *)script;
    s32 raster;
    s32 palette;
    s32 preset;
    struct hud_cache_entry *entry;
    s32 i;

    if (script_step == NULL) {
        PRINTF("attempted to initialize hud_element, but script was NULL\n");
        return;
    }

    preset = 0;

    while (1) {
        switch (*script_step++) {
            case HUD_ELEMENT_OP_End: return;
            case HUD_ELEMENT_OP_SetCI: script_step += 3; break;
            case HUD_ELEMENT_OP_SetTileSize:
                preset = *script_step++;
                hud_element->drawSizePreset = hud_element->tileSizePreset = preset;
                break;
            case HUD_ELEMENT_OP_SetSizesAutoScale:
            case HUD_ELEMENT_OP_SetSizesFixedScale:
                preset = *script_step;
                script_step += 2;
                hud_element->drawSizePreset = hud_element->tileSizePreset = preset;
                break;
            case HUD_ELEMENT_OP_SetCustomSize:
                hud_element->customDrawSize.x = hud_element->customImageSize.x = *script_step++;
                hud_element->customDrawSize.y = hud_element->customImageSize.y = *script_step++;
                hud_element->flags |= HUD_ELEMENT_FLAGS_CUSTOM_SIZE;
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

s32 hud_element_update(HudElement *hud_element) {
    s32 i;
    s32 drawSizePreset;
    s32 tileSizePreset;
    u8 sizePreset;
    f32 xScaled, yScaled;
    s32 imageWidth, imageHeight, drawWidth, drawHeight;
    u32 min, max;
    u32 flags;
    s32 raster, palette;
    s32 s1, s2;
    s32 arg1, arg2;
    f32 uniformScale;
    HudScript *newReadPos;

    HudTransform *hudTransform = hud_element->hudTransform;
    s32 *script_step = (s32 *)hud_element->readPos;
    if (script_step == NULL) {
        PRINTF("HudElement at 0x%8X returned NULL readPos\n", hud_element);
        return 0;
    }

    switch (*script_step++) {
        case HUD_ELEMENT_OP_End:
            hud_element->updateTimer = 60;
            flags = hud_element->flags;
            hud_element->flags = flags | HUD_ELEMENT_FLAGS_ANIMATION_FINISHED;
            break;
        case HUD_ELEMENT_OP_Delete:
            hud_element->updateTimer = 60;
            hud_element->flags |= HUD_ELEMENT_FLAGS_DELETE;
            break;
        case HUD_ELEMENT_OP_UseIA8:
            hud_element->readPos = (HudScript *)script_step;
            hud_element->flags |= HUD_ELEMENT_FLAGS_FMT_IA8;
            return 1;
        case HUD_ELEMENT_OP_SetVisible:
            hud_element->readPos = (HudScript *)script_step;
            hud_element->flags |= HUD_ELEMENT_FLAGS_FMT_CI4;
            return 1;
        case HUD_ELEMENT_OP_SetHidden:
            hud_element->readPos = (HudScript *)script_step;
            hud_element->flags &= ~HUD_ELEMENT_FLAGS_FMT_CI4;
            return 1;
        case HUD_ELEMENT_OP_SetFlags:
            s1 = *script_step++;
            hud_element->readPos = (HudScript *)script_step;
            hud_element->flags |= s1;
            return 1;
        case HUD_ELEMENT_OP_ClearFlags:
            s1 = *script_step++;
            hud_element->readPos = (HudScript *)script_step;
            hud_element->flags &= ~s1;
            return 1;
        case HUD_ELEMENT_OP_SetRGBA:
            hud_element->updateTimer = *script_step++;
            hud_element->rasterAddr = (u8 *)*script_step++;
            hud_element->readPos = (HudScript *)script_step;

            if (hud_element->flags & HUD_ELEMENT_FLAGS_MEMOFFSET) {
                hud_element->rasterAddr += hud_element->memOffset;
            }

            if (hud_element->flags & HUD_ELEMENT_FLAGS_FIXEDSCALE) {
                if (!(hud_element->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
                    tileSizePreset = hud_element->tileSizePreset;
                    drawSizePreset = hud_element->drawSizePreset;
                    imageWidth = gHudElementSizes[tileSizePreset].width;
                    imageHeight = gHudElementSizes[tileSizePreset].height;
                    drawWidth = gHudElementSizes[drawSizePreset].width;
                    drawHeight = gHudElementSizes[drawSizePreset].height;
                } else {
                    imageWidth = hud_element->customImageSize.x;
                    imageHeight = hud_element->customImageSize.y;
                    drawWidth = hud_element->customDrawSize.x;
                    drawHeight = hud_element->customDrawSize.y;
                }

                if (!(hud_element->flags & HUD_ELEMENT_FLAGS_200)) {
                    hud_element->flags |= HUD_ELEMENT_FLAGS_200;
                    hud_element->unkImgScale[0] = imageWidth;
                    hud_element->unkImgScale[1] = imageHeight;
                    hud_element->deltaSizeX = ((f32)drawWidth - (f32)imageWidth) / (f32)hud_element->updateTimer;
                    hud_element->deltaSizeY = ((f32)drawHeight - (f32)imageHeight) / (f32)hud_element->updateTimer;
                } else {
                    hud_element->flags &= ~HUD_ELEMENT_FLAGS_200;
                    hud_element->unkImgScale[0] = drawWidth;
                    hud_element->unkImgScale[1] = drawHeight;
                    hud_element->deltaSizeX = ((f32)imageWidth - (f32)drawWidth) / (f32)hud_element->updateTimer;
                    hud_element->deltaSizeY = ((f32)imageHeight - (f32)drawHeight) / (f32)hud_element->updateTimer;
                }
            }
            break;
        case HUD_ELEMENT_OP_SetCI:
            hud_element->updateTimer = *script_step++;
            
        
            if (hud_element->flags & HUD_ELEMENT_FLAGS_FIXEDSCALE) {
            
            }
            s32 raster_rom = (s32)*script_step++;
            s32 palette_rom = (s32)*script_step++;
            s32 raster_size;
            if (hud_element->flags & HUD_ELEMENT_FLAGS_MEMOFFSET) {
                raster_rom += hud_element->memOffset;
                palette_rom += hud_element->memOffset;
            }
            if (hud_element->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE){
                raster_size = (hud_element->customImageSize.x * hud_element->customImageSize.y) / 2;
            } else {
                raster_size = gHudElementSizes[hud_element->drawSizePreset].size;
            }
            hud_element->rasterAddr = get_cached_raster(raster_rom, raster_size);
            hud_element->paletteAddr = get_cached_palette(palette_rom);
            
            hud_element->readPos = (HudScript *)script_step;

            if (hud_element->flags & HUD_ELEMENT_FLAGS_FIXEDSCALE) {
                if (!(hud_element->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
                    tileSizePreset = hud_element->tileSizePreset;
                    drawSizePreset = hud_element->drawSizePreset;
                    imageWidth = gHudElementSizes[tileSizePreset].width;
                    imageHeight = gHudElementSizes[tileSizePreset].height;
                    drawWidth = gHudElementSizes[drawSizePreset].width;
                    drawHeight = gHudElementSizes[drawSizePreset].height;
                } else {
                    imageWidth = hud_element->customImageSize.x;
                    imageHeight = hud_element->customImageSize.y;
                    drawWidth = hud_element->customDrawSize.x;
                    drawHeight = hud_element->customDrawSize.y;
                }

                if (!(hud_element->flags & HUD_ELEMENT_FLAGS_200)) {
                    hud_element->flags |= HUD_ELEMENT_FLAGS_200;
                    hud_element->unkImgScale[0] = imageWidth;
                    hud_element->unkImgScale[1] = imageHeight;
                    hud_element->deltaSizeX = ((f32)drawWidth - (f32)imageWidth) / (f32)hud_element->updateTimer;
                    hud_element->deltaSizeY = ((f32)drawHeight - (f32)imageHeight) / (f32)hud_element->updateTimer;
                } else {
                    hud_element->flags &= ~HUD_ELEMENT_FLAGS_200;
                    hud_element->unkImgScale[0] = drawWidth;
                    hud_element->unkImgScale[1] = drawHeight;
                    hud_element->deltaSizeX = ((f32)imageWidth - (f32)drawWidth) / (f32)hud_element->updateTimer;
                    hud_element->deltaSizeY = ((f32)imageHeight - (f32)drawHeight) / (f32)hud_element->updateTimer;
                }
            }
            break;
        case HUD_ELEMENT_OP_SetImage:
            hud_element->updateTimer = *script_step++;

            hud_element->rasterAddr = get_cached_raster(ICONS_ITEMS_ROM_START + *script_step++,
                                                        gHudElementSizes[hud_element->tileSizePreset].size);
            hud_element->paletteAddr = get_cached_palette(ICONS_ITEMS_ROM_START + *script_step++);
            script_step += 3;
            hud_element->readPos = (HudScript *)script_step;

            if (hud_element->flags & HUD_ELEMENT_FLAGS_FIXEDSCALE) {
                if (!(hud_element->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
                    tileSizePreset = hud_element->tileSizePreset;
                    drawSizePreset = hud_element->drawSizePreset;
                    imageWidth = gHudElementSizes[tileSizePreset].width;
                    imageHeight = gHudElementSizes[tileSizePreset].height;
                    drawWidth = gHudElementSizes[drawSizePreset].width;
                    drawHeight = gHudElementSizes[drawSizePreset].height;
                } else {
                    imageWidth = hud_element->customImageSize.x;
                    imageHeight = hud_element->customImageSize.y;
                    drawWidth = hud_element->customDrawSize.x;
                    drawHeight = hud_element->customDrawSize.y;
                }

                if (!(hud_element->flags & HUD_ELEMENT_FLAGS_200)) {
                    hud_element->flags |= HUD_ELEMENT_FLAGS_200;
                    hud_element->unkImgScale[0] = imageWidth;
                    hud_element->unkImgScale[1] = imageHeight;
                    hud_element->deltaSizeX = ((f32)drawWidth - (f32)imageWidth) / (f32)hud_element->updateTimer;
                    hud_element->deltaSizeY = ((f32)drawHeight - (f32)imageHeight) / (f32)hud_element->updateTimer;
                } else {
                    hud_element->flags &= ~HUD_ELEMENT_FLAGS_200;
                    hud_element->unkImgScale[0] = drawWidth;
                    hud_element->unkImgScale[1] = drawHeight;
                    hud_element->deltaSizeX = ((f32)imageWidth - (f32)drawWidth) / (f32)hud_element->updateTimer;
                    hud_element->deltaSizeY = ((f32)imageHeight - (f32)drawHeight) / (f32)hud_element->updateTimer;
                }
            }
            break;
        case HUD_ELEMENT_OP_Restart: hud_element->readPos = hud_element->loopStartPos; return 1;
        case HUD_ELEMENT_OP_Loop:
            hud_element->loopStartPos = (HudScript *)script_step;
            hud_element->readPos = (HudScript *)script_step;
            return 1;
        case HUD_ELEMENT_OP_RandomRestart:
            s1 = *script_step++;
            s2 = *script_step++;
            if (pm_player.timer % s1 < s2) {
                hud_element->readPos = hud_element->loopStartPos;
            } else {
                hud_element->readPos = (HudScript *)script_step;
            }
            return 1;
        case HUD_ELEMENT_OP_SetTileSize:
            sizePreset = *script_step++;
            hud_element->widthScale = X10(1);
            hud_element->heightScale = X10(1);
            hud_element->readPos = (HudScript *)script_step;
            hud_element->drawSizePreset = sizePreset;
            hud_element->tileSizePreset = sizePreset;
            hud_element->flags &= ~HUD_ELEMENT_FLAGS_FIXEDSCALE;
            hud_element->flags &= ~HUD_ELEMENT_FLAGS_REPEATED;
            return 1;
        case HUD_ELEMENT_OP_SetSizesAutoScale:
            tileSizePreset = *script_step++;
            arg2 = *script_step++;

            hud_element->readPos = (HudScript *)script_step;
            hud_element->tileSizePreset = tileSizePreset;
            hud_element->drawSizePreset = arg2;

            imageWidth = gHudElementSizes[tileSizePreset].width;
            imageHeight = gHudElementSizes[tileSizePreset].height;
            drawWidth = gHudElementSizes[arg2].width;
            drawHeight = gHudElementSizes[arg2].height;

            xScaled = (f32)drawWidth / (f32)imageWidth;
            yScaled = (f32)drawHeight / (f32)imageHeight;

            xScaled = 1.0f / xScaled;
            yScaled = 1.0f / yScaled;

            hud_element->widthScale = X10(xScaled);
            hud_element->heightScale = X10(yScaled);

            hud_element->flags &= ~HUD_ELEMENT_FLAGS_FIXEDSCALE;
            hud_element->flags |= HUD_ELEMENT_FLAGS_REPEATED;
            return 1;
        case HUD_ELEMENT_OP_SetSizesFixedScale:
            tileSizePreset = *script_step++;
            drawSizePreset = *script_step++;

            hud_element->widthScale = X10(1);
            hud_element->heightScale = X10(1);
            hud_element->readPos = (HudScript *)script_step;
            hud_element->tileSizePreset = tileSizePreset;
            hud_element->drawSizePreset = drawSizePreset;
            hud_element->flags |= HUD_ELEMENT_FLAGS_FIXEDSCALE;
            hud_element->flags &= ~HUD_ELEMENT_FLAGS_REPEATED;
            hud_element->flags &= ~HUD_ELEMENT_FLAGS_200;
            return 1;
        case HUD_ELEMENT_OP_AddTexelOffsetX:
            s1 = *script_step++;
            hud_element->readPos = (HudScript *)script_step;
            hud_element->screenPosOffset.x += s1;
            return 1;
        case HUD_ELEMENT_OP_AddTexelOffsetY:
            s2 = *script_step++;
            if (!(hud_element->flags & HUD_ELEMENT_FLAGS_FLIPY)) {
                hud_element->screenPosOffset.y += s2;
            } else {
                hud_element->screenPosOffset.y -= s2;
            }
            hud_element->readPos = (HudScript *)script_step;
            return 1;
        case HUD_ELEMENT_OP_SetTexelOffset:
            s1 = *script_step++;
            s2 = *script_step++;
            hud_element->screenPosOffset.x = s1;
            if (!(hud_element->flags & HUD_ELEMENT_FLAGS_FLIPY)) {
                hud_element->screenPosOffset.y = s2;
            } else {
                hud_element->screenPosOffset.y = -s2;
            }
            hud_element->readPos = (HudScript *)script_step;
            return 1;
        case HUD_ELEMENT_OP_SetScale:
            uniformScale = (f32)*script_step++;
            uniformScale /= 65536;
            hud_element->uniformScale = uniformScale;
            if (!(hud_element->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
                imageWidth = gHudElementSizes[hud_element->tileSizePreset].width;
                imageHeight = gHudElementSizes[hud_element->tileSizePreset].height;
                drawWidth = gHudElementSizes[hud_element->drawSizePreset].width;
                drawHeight = gHudElementSizes[hud_element->drawSizePreset].height;
            } else {
                imageWidth = hud_element->customImageSize.x;
                imageHeight = hud_element->customImageSize.y;
                drawWidth = hud_element->customDrawSize.x;
                drawHeight = hud_element->customDrawSize.y;
            }

            hud_element->sizeX = drawWidth * uniformScale;
            hud_element->sizeY = drawHeight * uniformScale;

            xScaled = (f32)drawWidth / (f32)imageWidth * uniformScale;
            yScaled = (f32)drawHeight / (f32)imageHeight * uniformScale;

            xScaled = 1.0f / xScaled;
            yScaled = 1.0f / yScaled;

            hud_element->widthScale = X10(xScaled);
            hud_element->heightScale = X10(yScaled);

            hud_element->readPos = (HudScript *)script_step;
            hud_element->flags &= ~HUD_ELEMENT_FLAGS_FIXEDSCALE;
            hud_element->flags |= HUD_ELEMENT_FLAGS_REPEATED | HUD_ELEMENT_FLAGS_SCALED;
            return 1;
        case HUD_ELEMENT_OP_SetAlpha:
            s1 = *script_step++;
            hud_element->opacity = s1;
            hud_element->flags |= HUD_ELEMENT_FLAGS_TRANSPARENT;
            if (hud_element->opacity == 255) {
                hud_element->flags &= ~HUD_ELEMENT_FLAGS_TRANSPARENT;
            }
            hud_element->readPos = (HudScript *)script_step;
            return 1;
        case HUD_ELEMENT_OP_RandomDelay:
            s1 = *script_step++;
            s2 = *script_step++;
            hud_element->updateTimer = pm_player.timer % (s2 - s1) + s1;
            hud_element->readPos = (HudScript *)script_step;
            break;
        case HUD_ELEMENT_OP_SetCustomSize:
            hud_element->customDrawSize.x = hud_element->customImageSize.x = *script_step++;
            hud_element->customDrawSize.y = hud_element->customImageSize.y = *script_step++;
            hud_element->readPos = (HudScript *)script_step;
            hud_element->widthScale = X10(1);
            hud_element->heightScale = X10(1);
            hud_element->drawSizePreset = 0;
            hud_element->tileSizePreset = 0;
            hud_element->flags &= ~HUD_ELEMENT_FLAGS_FIXEDSCALE;
            hud_element->flags &= ~HUD_ELEMENT_FLAGS_REPEATED;
            hud_element->flags |= HUD_ELEMENT_FLAGS_CUSTOM_SIZE;
            return 1;
        case HUD_ELEMENT_OP_op_15:
            s1 = *script_step++;
            hud_element->readPos = (HudScript *)script_step;
            hud_element->flags &= ~(HUD_ELEMENT_FLAGS_1000000 | HUD_ELEMENT_FLAGS_2000000 | HUD_ELEMENT_FLAGS_4000000 |
                                    HUD_ELEMENT_FLAGS_8000000);
            hud_element->flags |= s1 << 24;
            return 1;
        case HUD_ELEMENT_OP_RandomBranch:
            // TODO: fix
            // s1 = *script_step++;
            // newReadPos = (HudScript *)script_step[pm_player.timer % (s1 - 1)];
            // hud_element->readPos = newReadPos;
            // hud_element_load_script(hud_element, newReadPos);
            return 1;
        case HUD_ELEMENT_OP_PlaySound:
            arg2 = *script_step++;
            //pm_PlaySfx(arg2);
            hud_element->readPos = (HudScript *)script_step;
            return 1;
        case HUD_ELEMENT_OP_SetPivot:
            arg1 = *script_step++;
            arg2 = *script_step++;
            hud_element->readPos = (HudScript *)script_step;
            if (hud_element->flags & HUD_ELEMENT_FLAGS_TRANSFORM) {
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

HudElement *hud_element_create(HudElement *hud_element, HudScript *hud_script) {
    if ((uintptr_t)hud_script > 0x80400000) {
        s32 i;
        for (i = 0; i < CUSTOM_SCRIPT_LIST_SIZE; i++) {
            if (custom_hud_script_list[i].hud_element == NULL) {
                custom_hud_script_list[i].hud_element = hud_element;
                custom_hud_script_list[i].hud_script = hud_script;
                break;
            }
        }
        if (i == CUSTOM_SCRIPT_LIST_SIZE) {
            PRINTF("custom script list full, unable to create icon\n");
            return NULL;
        }
    }
    hud_element->flags = HUD_ELEMENT_FLAGS_INITIALIZED;
    hud_element->readPos = hud_script;
    hud_element->updateTimer = 1;
    hud_element->drawSizePreset = -1;
    hud_element->tileSizePreset = -1;
    hud_element->renderPosX = 0;
    hud_element->renderPosY = 0;
    hud_element->loopStartPos = hud_script;
    hud_element->widthScale = X10(1.0f);
    hud_element->heightScale = X10(1.0f);
    hud_element->anim = hud_element->readPos;
    hud_element->uniformScale = 1.0f;
    hud_element->screenPosOffset.x = 0;
    hud_element->screenPosOffset.y = 0;
    hud_element->worldPosOffset.x = 0;
    hud_element->worldPosOffset.y = 0;
    hud_element->worldPosOffset.z = 0;
    hud_element->opacity = 255;
    hud_element->tint.r = 255;
    hud_element->tint.g = 255;
    hud_element->tint.b = 255;

    hud_element_script_init(hud_element, hud_element->readPos);
    while (hud_element_update(hud_element) != 0) {};
    return hud_element;
}

HudElement *hud_element_init(HudScript *hud_script, s32 x, s32 y, u8 alpha, f32 scale) {
    HudElement *hud_element = malloc(sizeof(*hud_element));
    hud_element = hud_element_create(hud_element, hud_script);
    if (hud_element == NULL) {
        free(hud_element);
        free(hud_script);
        return NULL;
    }
    hud_element_set_render_pos(hud_element, x, y);
    hud_element_set_alpha(hud_element, alpha);
    hud_element_set_scale(hud_element, scale);
    return hud_element;
}

Icon *icons_create_global(icon_global icon, s32 x, s32 y, u8 alpha, f32 scale) {
    s32 script_data[] = {icon};
    HudScript *hud_script = create_hud_script(SCRIPT_TYPE_GLOBAL, script_data);
    return hud_element_init(hud_script, x, y, alpha, scale);
}

Icon *icons_create_partner(u8 partner, s32 x, s32 y, u8 alpha, f32 scale, _Bool grayscale) {
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

    return hud_element_init(hud_script, x, y, alpha, scale);
}

Icon *icons_create_item(Item item, s32 x, s32 y, u8 alpha, f32 scale, _Bool grayscale) {
    s32 script_data[] = {item, grayscale};
    HudScript *hud_script = create_hud_script(SCRIPT_TYPE_ITEM, script_data);

    return hud_element_init(hud_script, x, y, alpha, scale);
}

void icons_update() {
    for (s32 i = 0; i < HUD_ELEMENT_QUEUE_SIZE; i++) {
        HudElement *hud_element = hud_element_queue[i];

        if (hud_element != NULL && hud_element->flags && !(hud_element->flags & HUD_ELEMENT_FLAGS_DISABLED)) {
            if (hud_element->flags & HUD_ELEMENT_FLAGS_DELETE) {
                free_custom_hud_script(hud_element);
                free(hud_element);
            } else if (hud_element->readPos != NULL) {
                hud_element->updateTimer--;
                if (hud_element->updateTimer == 0) {
                    while (hud_element_update(hud_element) != 0) {};
                }
                if (hud_element->flags & HUD_ELEMENT_FLAGS_FIXEDSCALE) {
                    hud_element->unkImgScale[0] += hud_element->deltaSizeX;
                    hud_element->unkImgScale[1] += hud_element->deltaSizeY;
                }
            } else {
                break;
            }
        }
    }

    for (s32 i = 0; i < HUD_ELEMENT_QUEUE_SIZE; i++) {
        if (hud_element_queue[i] != NULL) {
            draw_hud_element(hud_element_queue[i]);
            hud_element_queue[i] = NULL;
        }
    }
}

void icons_draw(Icon *icon) {
    for (s32 i = 0; i < HUD_ELEMENT_QUEUE_SIZE; i++) {
        if (hud_element_queue[i] == NULL) {
            hud_element_queue[i] = icon;
            return;
        }
    }
}

void icons_delete(Icon *icon) {
    hud_element_set_flags(icon, HUD_ELEMENT_FLAGS_DELETE);
}
