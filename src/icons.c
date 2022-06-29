#include <stdlib.h>
#include "icons.h"

// image_address can be either ROM or RAM
u8 *get_cache_entry_image(s32 image_address, s32 image_size, struct hud_cache_entry *cache, s32 cache_size) {
    s32 idx;
    for (idx = 0; idx < cache_size; idx++) {
        // return cached data
        if (cache[idx].id == image_address) {
            return cache[idx].data;
        }

        // load in data and place in cache
        if (cache[idx].id == 0) {
            cache[idx].id = image_address;
            cache[idx].data_size = image_size;
            cache[idx].data = malloc(image_size);
            // memcpy instead of dma if image_address is a pointer to ram
            if ((uintptr_t)image_address >> 24 == 0x80) {
                memcpy(cache[idx].data, (const void *)image_address, image_size);
            } else {
                nuPiReadRom(image_address, cache[idx].data, image_size);
            }
            return cache[idx].data;
        }
    }
    return NULL;
}

u8 *get_cached_raster(s32 addr, s32 size) {
    return get_cache_entry_image(addr, size, fp.hud_elements.raster_cache, RASTER_CACHE_SIZE);
}

u8 *get_cached_palette(s32 addr) {
    return get_cache_entry_image(addr, ICON_PALETTE_SIZE, fp.hud_elements.palette_cache, PALETTE_CACHE_SIZE);
}

/*
 * Many of the built-in scripts access data from RAM that we can't count on existing at any given time, so it's
 * beneficial to create our own scripts sometimes, though not always. This function takes care of deciding that.
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
HudScript *create_hud_script(enum hud_script_type type, s32 data[]) {
    switch (type) {
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

void hud_element_set_render_pos(HudElement *hudElement, s32 x, s32 y) {
    hudElement->renderPosX = x;
    hudElement->renderPosY = y;
}
void hud_element_set_scale(HudElement *hudElement, f32 scale) {
    s32 drawSizeX;
    s32 drawSizeY;
    s32 imgSizeX;
    s32 imgSizeY;
    f32 xScaled, yScaled;

    hudElement->uniformScale = scale;
    if (!(hudElement->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
        imgSizeX = gHudElementSizes[hudElement->tileSizePreset].width;
        imgSizeY = gHudElementSizes[hudElement->tileSizePreset].height;
        drawSizeX = gHudElementSizes[hudElement->drawSizePreset].width;
        drawSizeY = gHudElementSizes[hudElement->drawSizePreset].height;
    } else {
        imgSizeX = hudElement->customImageSize.x;
        imgSizeY = hudElement->customImageSize.y;
        drawSizeX = hudElement->customDrawSize.x;
        drawSizeY = hudElement->customDrawSize.y;
    }
    hudElement->sizeX = drawSizeX * scale;
    hudElement->sizeY = drawSizeY * scale;
    hudElement->flags &= ~HUD_ELEMENT_FLAGS_FIXEDSCALE;
    hudElement->flags |= HUD_ELEMENT_FLAGS_REPEATED | HUD_ELEMENT_FLAGS_SCALED;

    xScaled = ((f32)drawSizeX / (f32)imgSizeX) * scale;
    yScaled = ((f32)drawSizeY / (f32)imgSizeY) * scale;

    xScaled = 1.0f / xScaled;
    yScaled = 1.0f / yScaled;

    hudElement->widthScale = X10(xScaled);
    hudElement->heightScale = X10(yScaled);
}

void hud_element_set_alpha(HudElement *hudElement, s32 opacity) {
    hudElement->flags |= HUD_ELEMENT_FLAGS_TRANSPARENT;
    hudElement->opacity = opacity;

    if (opacity == 255) {
        hudElement->flags &= ~HUD_ELEMENT_FLAGS_TRANSPARENT;
    }
}

void hud_element_script_init(HudElement *hudElement, HudScript *script) {
    s32 *script_step = (s32 *)script;
    s32 raster;
    s32 palette;
    s32 preset;
    struct hud_cache_entry *entry;
    s32 i;

    if (script_step == NULL) {
        return;
    }

    preset = 0;

    while (1) {
        switch (*script_step++) {
            case HUD_ELEMENT_OP_End: return;
            case HUD_ELEMENT_OP_SetCI: script_step += 3; break;
            case HUD_ELEMENT_OP_SetTileSize:
                preset = *script_step++;
                hudElement->drawSizePreset = hudElement->tileSizePreset = preset;
                break;
            case HUD_ELEMENT_OP_SetSizesAutoScale:
            case HUD_ELEMENT_OP_SetSizesFixedScale:
                preset = *script_step;
                script_step += 2;
                hudElement->drawSizePreset = hudElement->tileSizePreset = preset;
                break;
            case HUD_ELEMENT_OP_SetCustomSize:
                hudElement->customDrawSize.x = hudElement->customImageSize.x = *script_step++;
                hudElement->customDrawSize.y = hudElement->customImageSize.y = *script_step++;
                hudElement->flags |= HUD_ELEMENT_FLAGS_CUSTOM_SIZE;
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
        PRINTF("ERROR: HudElement at 0x%8X returned NULL readPos\n", hud_element);
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

            s32 raster_rom = (s32)*script_step++;
            s32 raster_size = gHudElementSizes[hud_element->drawSizePreset].size;
            hud_element->rasterAddr = get_cached_raster(raster_rom, raster_size);

            s32 palette_rom = (s32)*script_step++;
            hud_element->paletteAddr = get_cached_palette(palette_rom);

            hud_element->readPos = (HudScript *)script_step;

            if (hud_element->flags & HUD_ELEMENT_FLAGS_MEMOFFSET) {
                hud_element->rasterAddr += hud_element->memOffset;
                hud_element->paletteAddr += hud_element->memOffset;
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
        case HUD_ELEMENT_OP_SetImage:
            hud_element->updateTimer = *script_step++;
        
            hud_element->rasterAddr = get_cached_raster(ICONS_ITEMS_ROM_START + *script_step++, gHudElementSizes[hud_element->tileSizePreset].size);
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
            pm_PlaySfx(arg2);
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

HudElement *hud_element_create(HudScript *hud_script) {
    HudElement *hudElement = malloc(sizeof(*hudElement));

    hudElement->flags = HUD_ELEMENT_FLAGS_INITIALIZED;
    hudElement->readPos = hud_script;
    hudElement->updateTimer = 1;
    hudElement->drawSizePreset = -1;
    hudElement->tileSizePreset = -1;
    hudElement->renderPosX = 0;
    hudElement->renderPosY = 0;
    hudElement->loopStartPos = hud_script;
    hudElement->widthScale = X10(1.0f);
    hudElement->heightScale = X10(1.0f);
    hudElement->anim = hudElement->readPos;
    hudElement->uniformScale = 1.0f;
    hudElement->screenPosOffset.x = 0;
    hudElement->screenPosOffset.y = 0;
    hudElement->worldPosOffset.x = 0;
    hudElement->worldPosOffset.y = 0;
    hudElement->worldPosOffset.z = 0;
    hudElement->opacity = 255;
    hudElement->tint.r = 255;
    hudElement->tint.g = 255;
    hudElement->tint.b = 255;

    hud_element_script_init(hudElement, hudElement->readPos);
    while (hud_element_update(hudElement) != 0) {};
    return hudElement;
}

HudElement *hud_element_init(HudScript *hud_script, s32 x, s32 y, u8 alpha, f32 scale) {
    HudElement *element = hud_element_create(hud_script);
    hud_element_set_render_pos(element, x, y);
    hud_element_set_alpha(element, alpha);
    hud_element_set_scale(element, scale);
    return element;
}

HudElement *get_hud_element_partner(Partner partner, s32 x, s32 y, u8 alpha, f32 scale, _Bool grayscale) {
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

HudElement *get_hud_element_item(Item item, s32 x, s32 y, u8 alpha, f32 scale, _Bool grayscale) {
    s32 script_data[] = {item, grayscale};
    HudScript *hud_script = create_hud_script(SCRIPT_TYPE_ITEM, script_data);
    
    return hud_element_init(hud_script, x, y, alpha, scale);
}
