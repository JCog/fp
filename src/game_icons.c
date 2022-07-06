#include <stdlib.h>
#include "enums.h"
#include "game_icons.h"
#include "math.h"
#include "util.h"

#define ICON_QUEUE_SIZE                     200
#define IMAGE_CACHE_SIZE                    2 * ICON_QUEUE_SIZE

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

typedef struct {
    u16 r : 5;
    u16 g : 5;
    u16 b : 5;
    u16 a : 1;
} rgba;

static game_icon *game_icon_queue[ICON_QUEUE_SIZE];
static image_entry image_cache[IMAGE_CACHE_SIZE];
static void *image_buffer;
static u16 image_cache_oldest_id;

static void convert_image_to_grayscale(void *new_image, u32 image_address, u32 image_size);
static s32 get_image_cache_entry_id(u32 image_address, u32 image_size, _Bool custom_grayscale);
static s32 get_cached_raster_id(u32 addr, u32 size);
static s32 get_cached_palette_id(u32 addr, _Bool custom_grayscale);

static void free_hud_script(game_icon *icon);
static HudScript *create_hud_script(hud_script_type type, s32 data[]);

static void refresh_icon_image(game_icon *icon);
static void game_icon_set_flags(game_icon *icon, s32 flags);
static void game_icon_clear_flags(game_icon *icon, s32 flags);

static void game_icon_script_init(game_icon *icon, HudScript *script);
static s32 game_icon_update(game_icon *icon);
static game_icon *game_icon_create(game_icon *icon, HudScript *script, _Bool custom_grayscale);
static game_icon *game_icon_init(HudScript *script, _Bool custom_grayscale);

static void convert_image_to_grayscale(void *new_image, u32 image_address, u32 image_size) {
    // assuming 5551 RGBA palettes until I have a reason not to
    s16 image_id = get_image_cache_entry_id(image_address, image_size, 0);
    rgba *base_image;
    if (image_id == -1) {
        base_image = (rgba *)image_buffer;
    } else {
        base_image = (rgba *)image_cache[image_id].image;
    }
    u8 pixel_count = image_size / 2;
    for (u32 i = 0; i < pixel_count; i++) {
        rgba *old_pixel = &base_image[i];
        rgba *new_pixel = &((rgba *)new_image)[i];

        f32 lum = 0.2782f * old_pixel->r + 0.6562f * old_pixel->g + 0.0656f * old_pixel->b;
        u16 gray = (lum * 14 / 31) + 12;
        new_pixel->r = gray;
        new_pixel->g = gray;
        new_pixel->b = gray;
        new_pixel->a = old_pixel->a;
    }
}

// image_address can be either ROM or RAM
static s32 get_image_cache_entry_id(u32 image_address, u32 image_size, _Bool custom_grayscale) {
    // no need to cache if it's a pointer to RAM
    if (!custom_grayscale && image_address >> 24 == 0x80) {
        image_buffer = (void *)image_address;
        return -1;
    }

    // use address as id, or id+1 for a custom grayscale
    s32 id = image_address;
    if (custom_grayscale) {
        id++;
    }

    s32 idx;
    for (idx = 0; idx < IMAGE_CACHE_SIZE; idx++) {
        // return cached data
        if (image_cache[idx].cache_id == id) {
            return idx;
        }

        // load in data and place in cache
        if (image_cache[idx].cache_id == 0) {
            image_cache[idx].cache_id = id;
            image_cache[idx].image = malloc(image_size);
            if (custom_grayscale) {
                convert_image_to_grayscale(image_cache[idx].image, image_address, image_size);
            } else {
                nuPiReadRom(image_address, image_cache[idx].image, image_size);
            }
            return idx;
        }
    }

    // no cache hits, replace oldest item
    idx = (image_cache_oldest_id)++;
    if (image_cache_oldest_id == IMAGE_CACHE_SIZE) {
        image_cache_oldest_id = 0;
    }
    image_cache[idx].cache_id = id;
    free(image_cache[idx].image);
    image_cache[idx].image = malloc(image_size);
    if (custom_grayscale) {
        convert_image_to_grayscale(image_cache[idx].image, image_address, image_size);
    } else {
        nuPiReadRom(image_address, image_cache[idx].image, image_size);
    }
    return idx;
}

static s32 get_cached_raster_id(u32 addr, u32 size) {
    return get_image_cache_entry_id(addr, size, 0);
}

static s32 get_cached_palette_id(u32 addr, _Bool custom_grayscale) {
    return get_image_cache_entry_id(addr, ICON_PALETTE_SIZE, custom_grayscale);
}

static void free_hud_script(game_icon *icon) {
    if ((uintptr_t)icon->script > 0x80400000) {
        free(icon->script);
        icon->script = NULL;
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

static void refresh_icon_image(game_icon *icon) {
    if (image_cache[IMAGE_CACHE_SIZE - 1].image == NULL) {
        return; // cache not full, no chance of stale reference
    }
    s32 *script_step = (s32 *)icon->script;

    while (1) {
        switch (*script_step++) {
            case HUD_ELEMENT_OP_End: return;
            case HUD_ELEMENT_OP_SetCI: {
                script_step++;
                s32 raster_rom = (s32)*script_step++;
                s32 palette_rom = (s32)*script_step;

                s32 raster_size;
                if (icon->flags & HUD_ELEMENT_FLAGS_MEMOFFSET) {
                    raster_rom += icon->mem_offset;
                    palette_rom += icon->mem_offset;
                }
                if (icon->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE) {
                    raster_size = (icon->custom_image_size.x * icon->custom_image_size.y) / 2;
                } else {
                    raster_size = gHudElementSizes[icon->draw_size_preset].size;
                }
                s16 raster_index = get_cached_raster_id(raster_rom, raster_size);
                s16 palette_index = get_cached_palette_id(palette_rom, icon->custom_grayscale);
                if (raster_index == -1) {
                    icon->raster.image = image_buffer;
                } else {
                    icon->raster.image = image_cache[raster_index].image;
                }
                icon->raster.cache_id = raster_index;
                if (palette_index == -1) {
                    icon->palette.image = image_buffer;
                } else {
                    icon->palette.image = image_cache[palette_index].image;
                }
                icon->palette.cache_id = palette_index;
            }
                return;
            case HUD_ELEMENT_OP_SetTileSize: script_step++; break;
            case HUD_ELEMENT_OP_SetSizesAutoScale:
            case HUD_ELEMENT_OP_SetSizesFixedScale: script_step += 3; break;
            case HUD_ELEMENT_OP_SetCustomSize: script_step += 2; break;
            case HUD_ELEMENT_OP_AddTexelOffsetX:
            case HUD_ELEMENT_OP_AddTexelOffsetY:
            case HUD_ELEMENT_OP_SetScale:
            case HUD_ELEMENT_OP_SetAlpha:
            case HUD_ELEMENT_OP_op_15:
            case HUD_ELEMENT_OP_RandomBranch:
            case HUD_ELEMENT_OP_SetFlags:
            case HUD_ELEMENT_OP_ClearFlags:
            case HUD_ELEMENT_OP_PlaySound: script_step++; break;
            case HUD_ELEMENT_OP_SetRGBA: {
                script_step++;
                u32 raster_addr = *script_step;

                if (icon->flags & HUD_ELEMENT_FLAGS_MEMOFFSET) {
                    raster_addr += icon->mem_offset;
                }

                if (icon->flags & HUD_ELEMENT_FLAGS_FIXEDSCALE) {
                    s32 image_width, image_height;

                    if (!(icon->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
                        s32 tile_size_preset = icon->tile_size_preset;

                        image_width = gHudElementSizes[tile_size_preset].width;
                        image_height = gHudElementSizes[tile_size_preset].height;
                    } else {
                        image_width = icon->custom_image_size.x;
                        image_height = icon->custom_image_size.y;
                    }
                    // TODO: figure out the proper size for custom images sizes
                    s16 raster_index = get_cached_raster_id(raster_addr, gHudElementSizes[icon->draw_size_preset].size);
                    icon->raster.cache_id = raster_index;
                    if (raster_index == -1) {
                        icon->raster.image = image_buffer;
                    } else {
                        icon->raster.image = image_cache[raster_index].image;
                    }
                }
            }
                return;
            case HUD_ELEMENT_OP_SetTexelOffset:
            case HUD_ELEMENT_OP_RandomDelay:
            case HUD_ELEMENT_OP_RandomRestart:
            case HUD_ELEMENT_OP_SetPivot: script_step += 2; break;
            case HUD_ELEMENT_OP_SetImage: {
                // possibly only used with item icons?
                script_step++;
                s32 raster_addr = *script_step++;
                s32 palette_addr = *script_step++;

                s16 raster_index = get_cached_raster_id(ICONS_ITEMS_ROM_START + raster_addr,
                                                        gHudElementSizes[icon->tile_size_preset].size);
                s16 palette_index = get_cached_palette_id(ICONS_ITEMS_ROM_START + palette_addr, icon->custom_grayscale);

                icon->raster.cache_id = raster_index;
                if (raster_index == -1) {
                    icon->raster.image = image_buffer;
                } else {
                    icon->raster.image = image_cache[raster_index].image;
                }

                icon->palette.cache_id = palette_index;
                if (palette_index == -1) {
                    icon->palette.image = image_buffer;
                } else {
                    icon->palette.image = image_cache[palette_index].image;
                }
            }
                return;
        }
    }
}

void game_icons_set_pos(game_icon *icon, s16 x, s16 y) {
    icon->render_pos.x = x;
    icon->render_pos.y = y;
}

void game_icons_set_pos_offset(game_icon *icon, s16 x, s16 y) {
    icon->render_pos_offset.x = x;
    icon->render_pos_offset.y = y;
}

void game_icons_set_scale(game_icon *icon, f32 scale) {
    s32 draw_size_x;
    s32 draw_size_y;
    s32 img_size_x;
    s32 img_size_y;
    f32 x_scaled, y_scaled;

    icon->uniform_scale = scale;
    if (!(icon->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
        img_size_x = gHudElementSizes[icon->tile_size_preset].width;
        img_size_y = gHudElementSizes[icon->tile_size_preset].height;
        draw_size_x = gHudElementSizes[icon->draw_size_preset].width;
        draw_size_y = gHudElementSizes[icon->draw_size_preset].height;
    } else {
        img_size_x = icon->custom_image_size.x;
        img_size_y = icon->custom_image_size.y;
        draw_size_x = icon->custom_draw_size.x;
        draw_size_y = icon->custom_draw_size.y;
    }
    icon->size_x = draw_size_x * scale;
    icon->size_y = draw_size_y * scale;
    icon->flags &= ~HUD_ELEMENT_FLAGS_FIXEDSCALE;
    icon->flags |= HUD_ELEMENT_FLAGS_REPEATED | HUD_ELEMENT_FLAGS_SCALED;

    x_scaled = ((f32)draw_size_x / (f32)img_size_x) * scale;
    y_scaled = ((f32)draw_size_y / (f32)img_size_y) * scale;

    x_scaled = 1.0f / x_scaled;
    y_scaled = 1.0f / y_scaled;

    icon->width_scale = X10(x_scaled);
    icon->height_scale = X10(y_scaled);
}

void game_icons_set_alpha(game_icon *icon, s32 alpha) {
    icon->flags |= HUD_ELEMENT_FLAGS_TRANSPARENT;
    icon->alpha = alpha;

    if (alpha == 255) {
        icon->flags &= ~HUD_ELEMENT_FLAGS_TRANSPARENT;
    }
}

void game_icons_set_drop_shadow(game_icon *icon, _Bool drop_shadow) {
    if (drop_shadow) {
        game_icon_set_flags(icon, HUD_ELEMENT_FLAGS_DROP_SHADOW);
    } else {
        game_icon_clear_flags(icon, HUD_ELEMENT_FLAGS_DROP_SHADOW);
    }
}

void game_icons_set_tint(game_icon *icon, u8 r, u8 g, u8 b) {
    icon->tint.r = r;
    icon->tint.g = g;
    icon->tint.b = b;
}

s32 game_icons_get_width(game_icon *icon) {
    if (!(icon->flags & HUD_ELEMENT_FLAGS_FIXEDSCALE)) {
        if (!(icon->flags & HUD_ELEMENT_FLAGS_SCALED)) {
            if (!(icon->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
                return gHudElementSizes[icon->draw_size_preset].width;
            } else {
                return icon->custom_draw_size.x;
            }
        } else {
            return icon->size_x;
        }
    } else {
        return icon->unk_img_scale[0];
    }
}

s32 game_icons_get_height(game_icon *icon) {
    if (!(icon->flags & HUD_ELEMENT_FLAGS_FIXEDSCALE)) {
        if (!(icon->flags & HUD_ELEMENT_FLAGS_SCALED)) {
            if (!(icon->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
                return gHudElementSizes[icon->draw_size_preset].height;
            } else {
                return icon->custom_draw_size.y;
            }
        } else {
            return icon->size_y;
        }
    } else {
        return icon->unk_img_scale[1];
    }
}

static void game_icon_set_flags(game_icon *icon, s32 flags) {
    icon->flags |= flags;
}

static void game_icon_clear_flags(game_icon *icon, s32 flags) {
    icon->flags &= ~flags;
}

static game_icon *game_icon_init(HudScript *script, _Bool custom_grayscale) {
    game_icon *icon = malloc(sizeof(*icon));
    icon = game_icon_create(icon, script, custom_grayscale);
    return icon;
}

game_icon *game_icons_create_global(icon_global icon, _Bool grayscale) {
    s32 script_data[] = {icon};
    return game_icon_init(create_hud_script(SCRIPT_TYPE_GLOBAL, script_data), grayscale);
}

game_icon *game_icons_create_partner(Partner partner, _Bool grayscale) {
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

    s32 data[] = {partner_index, grayscale};
    return game_icon_init(create_hud_script(SCRIPT_TYPE_PARTNER, data), 0);
}

game_icon *game_icons_create_item(Item item, _Bool grayscale) {
    IconHudScriptPair *script_pair = &gItemHudScripts[gItemTable[item].hudElemID];
    _Bool custom_grayscale = grayscale && script_pair->enabled == script_pair->disabled;

    s32 data[] = {item, grayscale};
    return game_icon_init(create_hud_script(SCRIPT_TYPE_ITEM, data), custom_grayscale);
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
                } else if (game_icon_queue[idx]->read_pos != NULL) {
                    game_icon_queue[idx]->update_timer--;
                    if (game_icon_queue[idx]->update_timer == 0) {
                        while (game_icon_update(game_icon_queue[idx]) != 0) {};
                    }
                    if (game_icon_queue[idx]->flags & HUD_ELEMENT_FLAGS_FIXEDSCALE) {
                        game_icon_queue[idx]->unk_img_scale[0] += game_icon_queue[idx]->delta_size_x;
                        game_icon_queue[idx]->unk_img_scale[1] += game_icon_queue[idx]->delta_size_y;
                    }
                    next_icon = game_icon_queue[idx];
                }
            }
        }
        game_icon_queue[idx] = NULL;
        idx++;
        if (next_icon) {
            refresh_icon_image(next_icon);
            return next_icon;
        }
    }
    idx = 0;
    return NULL;
}

static game_icon *game_icon_create(game_icon *icon, HudScript *script, _Bool custom_grayscale) {
    icon->raster.cache_id = 0;
    icon->raster.image = NULL;
    icon->palette.cache_id = 0;
    icon->palette.image = NULL;
    icon->flags = HUD_ELEMENT_FLAGS_INITIALIZED;
    icon->read_pos = script;
    icon->update_timer = 1;
    icon->draw_size_preset = -1;
    icon->tile_size_preset = -1;
    icon->render_pos.x = 0;
    icon->render_pos.y = 0;
    icon->render_pos_offset.x = 0;
    icon->render_pos_offset.y = 0;
    icon->loop_start_pos = script;
    icon->width_scale = X10(1.0f);
    icon->height_scale = X10(1.0f);
    icon->script = icon->read_pos;
    icon->uniform_scale = 1.0f;
    icon->screen_pos_offset.x = 0;
    icon->screen_pos_offset.y = 0;
    icon->world_pos_offset.x = 0;
    icon->world_pos_offset.y = 0;
    icon->world_pos_offset.z = 0;
    icon->alpha = 255;
    icon->tint.r = 255;
    icon->tint.g = 255;
    icon->tint.b = 255;
    icon->custom_grayscale = custom_grayscale;

    game_icon_script_init(icon, icon->read_pos);
    while (game_icon_update(icon) != 0) {};
    return icon;
}

static void game_icon_script_init(game_icon *icon, HudScript *script) {
    s32 *script_step = (s32 *)script;

    if (script_step == NULL) {
        PRINTF("attempted to initialize icon, but script was NULL\n");
        return;
    }

    while (1) {
        switch (*script_step++) {
            case HUD_ELEMENT_OP_End: return;
            case HUD_ELEMENT_OP_SetCI: script_step += 3; break;
            case HUD_ELEMENT_OP_SetTileSize:
                icon->draw_size_preset = icon->tile_size_preset = *script_step++;
                ;
                break;
            case HUD_ELEMENT_OP_SetSizesAutoScale:
            case HUD_ELEMENT_OP_SetSizesFixedScale:
                script_step += 2;
                icon->draw_size_preset = icon->tile_size_preset = *script_step++;
                ;
                break;
            case HUD_ELEMENT_OP_SetCustomSize:
                icon->custom_draw_size.x = icon->custom_image_size.x = *script_step++;
                icon->custom_draw_size.y = icon->custom_image_size.y = *script_step++;
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
    game_icon_transform *hudTransform = icon->transform;
    s32 *script_step = (s32 *)icon->read_pos;
    if (script_step == NULL) {
        PRINTF("game_icon at 0x%8X returned NULL read_pos\n", icon);
        return 0;
    }

    switch (*script_step++) {
        case HUD_ELEMENT_OP_End: {
            icon->update_timer = 60;
            icon->flags |= HUD_ELEMENT_FLAGS_ANIMATION_FINISHED;
            break;
        }
        case HUD_ELEMENT_OP_Delete: {
            icon->update_timer = 60;
            icon->flags |= HUD_ELEMENT_FLAGS_DELETE;
            break;
        }
        case HUD_ELEMENT_OP_UseIA8: {
            icon->flags |= HUD_ELEMENT_FLAGS_FMT_IA8;

            icon->read_pos = (HudScript *)script_step;
            return 1;
        }
        case HUD_ELEMENT_OP_SetVisible: {
            icon->flags |= HUD_ELEMENT_FLAGS_FMT_CI4;

            icon->read_pos = (HudScript *)script_step;
            return 1;
        }
        case HUD_ELEMENT_OP_SetHidden: {
            icon->flags &= ~HUD_ELEMENT_FLAGS_FMT_CI4;

            icon->read_pos = (HudScript *)script_step;
            return 1;
        }
        case HUD_ELEMENT_OP_SetFlags: {
            icon->flags |= *script_step++;

            icon->read_pos = (HudScript *)script_step;
            return 1;
        }
        case HUD_ELEMENT_OP_ClearFlags: {
            icon->flags &= ~(*script_step++);

            icon->read_pos = (HudScript *)script_step;
            return 1;
        }
        case HUD_ELEMENT_OP_SetRGBA: {
            icon->update_timer = *script_step++;
            u32 raster_addr = *script_step++;

            if (icon->flags & HUD_ELEMENT_FLAGS_MEMOFFSET) {
                raster_addr += icon->mem_offset;
            }

            if (icon->flags & HUD_ELEMENT_FLAGS_FIXEDSCALE) {
                s32 image_width, image_height, draw_width, draw_height;

                if (!(icon->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
                    s32 tile_size_preset = icon->tile_size_preset;
                    s32 draw_size_preset = icon->draw_size_preset;

                    image_width = gHudElementSizes[tile_size_preset].width;
                    image_height = gHudElementSizes[tile_size_preset].height;
                    draw_width = gHudElementSizes[draw_size_preset].width;
                    draw_height = gHudElementSizes[draw_size_preset].height;
                } else {
                    image_width = icon->custom_image_size.x;
                    image_height = icon->custom_image_size.y;
                    draw_width = icon->custom_draw_size.x;
                    draw_height = icon->custom_draw_size.y;
                }
                // TODO: figure out the proper size for custom images sizes
                s16 raster_index = get_cached_raster_id(raster_addr, gHudElementSizes[icon->draw_size_preset].size);
                icon->raster.cache_id = raster_index;
                if (raster_index == -1) {
                    icon->raster.image = image_buffer;
                } else {
                    icon->raster.image = image_cache[raster_index].image;
                }

                if (!(icon->flags & HUD_ELEMENT_FLAGS_200)) {
                    icon->flags |= HUD_ELEMENT_FLAGS_200;
                    icon->unk_img_scale[0] = image_width;
                    icon->unk_img_scale[1] = image_height;
                    icon->delta_size_x = ((f32)draw_width - (f32)image_width) / (f32)icon->update_timer;
                    icon->delta_size_y = ((f32)draw_height - (f32)image_height) / (f32)icon->update_timer;
                } else {
                    icon->flags &= ~HUD_ELEMENT_FLAGS_200;
                    icon->unk_img_scale[0] = draw_width;
                    icon->unk_img_scale[1] = draw_height;
                    icon->delta_size_x = ((f32)image_width - (f32)draw_width) / (f32)icon->update_timer;
                    icon->delta_size_y = ((f32)image_height - (f32)draw_height) / (f32)icon->update_timer;
                }
            }

            icon->read_pos = (HudScript *)script_step;
            break;
        }
        case HUD_ELEMENT_OP_SetCI: {
            icon->update_timer = *script_step++;
            s32 raster_rom = (s32)*script_step++;
            s32 palette_rom = (s32)*script_step++;

            s32 raster_size;
            if (icon->flags & HUD_ELEMENT_FLAGS_MEMOFFSET) {
                raster_rom += icon->mem_offset;
                palette_rom += icon->mem_offset;
            }
            if (icon->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE) {
                raster_size = (icon->custom_image_size.x * icon->custom_image_size.y) / 2;
            } else {
                raster_size = gHudElementSizes[icon->draw_size_preset].size;
            }

            s16 raster_index = get_cached_raster_id(raster_rom, raster_size);
            icon->raster.cache_id = raster_index;
            if (raster_index == -1) {
                icon->raster.image = image_buffer;
            } else {
                icon->raster.image = image_cache[raster_index].image;
            }

            s16 palette_index = get_cached_palette_id(palette_rom, icon->custom_grayscale);
            icon->palette.cache_id = palette_index;
            if (palette_index == -1) {
                icon->palette.image = image_buffer;
            } else {
                icon->palette.image = image_cache[palette_index].image;
            }

            if (icon->flags & HUD_ELEMENT_FLAGS_FIXEDSCALE) {
                s32 image_width, image_height, draw_width, draw_height;

                if (!(icon->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
                    s32 tile_size_preset = icon->tile_size_preset;
                    s32 draw_size_preset = icon->draw_size_preset;

                    image_width = gHudElementSizes[tile_size_preset].width;
                    image_height = gHudElementSizes[tile_size_preset].height;
                    draw_width = gHudElementSizes[draw_size_preset].width;
                    draw_height = gHudElementSizes[draw_size_preset].height;
                } else {
                    image_width = icon->custom_image_size.x;
                    image_height = icon->custom_image_size.y;
                    draw_width = icon->custom_draw_size.x;
                    draw_height = icon->custom_draw_size.y;
                }

                if (!(icon->flags & HUD_ELEMENT_FLAGS_200)) {
                    icon->flags |= HUD_ELEMENT_FLAGS_200;
                    icon->unk_img_scale[0] = image_width;
                    icon->unk_img_scale[1] = image_height;
                    icon->delta_size_x = ((f32)draw_width - (f32)image_width) / (f32)icon->update_timer;
                    icon->delta_size_y = ((f32)draw_height - (f32)image_height) / (f32)icon->update_timer;
                } else {
                    icon->flags &= ~HUD_ELEMENT_FLAGS_200;
                    icon->unk_img_scale[0] = draw_width;
                    icon->unk_img_scale[1] = draw_height;
                    icon->delta_size_x = ((f32)image_width - (f32)draw_width) / (f32)icon->update_timer;
                    icon->delta_size_y = ((f32)image_height - (f32)draw_height) / (f32)icon->update_timer;
                }
            }

            icon->read_pos = (HudScript *)script_step;
            break;
        }
        case HUD_ELEMENT_OP_SetImage: {
            // possibly only used with item icons?
            icon->update_timer = *script_step++;
            s32 raster_addr = *script_step++;
            s32 palette_addr = *script_step++;
            script_step += 3;

            s16 raster_index = get_cached_raster_id(ICONS_ITEMS_ROM_START + raster_addr,
                                                    gHudElementSizes[icon->tile_size_preset].size);
            icon->raster.cache_id = raster_index;
            if (raster_index == -1) {
                icon->raster.image = image_buffer;
            } else {
                icon->raster.image = image_cache[raster_index].image;
            }

            s16 palette_index = get_cached_palette_id(ICONS_ITEMS_ROM_START + palette_addr, icon->custom_grayscale);
            icon->palette.cache_id = palette_index;
            if (palette_index == -1) {
                icon->palette.image = image_buffer;
            } else {
                icon->palette.image = image_cache[palette_index].image;
            }

            if (icon->flags & HUD_ELEMENT_FLAGS_FIXEDSCALE) {
                s32 image_width, image_height, draw_width, draw_height;

                if (!(icon->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
                    s32 tile_size_preset = icon->tile_size_preset;
                    s32 draw_size_preset = icon->draw_size_preset;

                    image_width = gHudElementSizes[tile_size_preset].width;
                    image_height = gHudElementSizes[tile_size_preset].height;
                    draw_width = gHudElementSizes[draw_size_preset].width;
                    draw_height = gHudElementSizes[draw_size_preset].height;
                } else {
                    image_width = icon->custom_image_size.x;
                    image_height = icon->custom_image_size.y;
                    draw_width = icon->custom_draw_size.x;
                    draw_height = icon->custom_draw_size.y;
                }

                if (!(icon->flags & HUD_ELEMENT_FLAGS_200)) {
                    icon->flags |= HUD_ELEMENT_FLAGS_200;
                    icon->unk_img_scale[0] = image_width;
                    icon->unk_img_scale[1] = image_height;
                    icon->delta_size_x = ((f32)draw_width - (f32)image_width) / (f32)icon->update_timer;
                    icon->delta_size_y = ((f32)draw_height - (f32)image_height) / (f32)icon->update_timer;
                } else {
                    icon->flags &= ~HUD_ELEMENT_FLAGS_200;
                    icon->unk_img_scale[0] = draw_width;
                    icon->unk_img_scale[1] = draw_height;
                    icon->delta_size_x = ((f32)image_width - (f32)draw_width) / (f32)icon->update_timer;
                    icon->delta_size_y = ((f32)image_height - (f32)draw_height) / (f32)icon->update_timer;
                }
            }

            icon->read_pos = (HudScript *)script_step;
            break;
        }
        case HUD_ELEMENT_OP_Restart: {
            icon->read_pos = icon->loop_start_pos;
            return 1;
        }
        case HUD_ELEMENT_OP_Loop: {
            icon->loop_start_pos = (HudScript *)script_step;

            icon->read_pos = (HudScript *)script_step;
            return 1;
        }
        case HUD_ELEMENT_OP_RandomRestart: {
            s32 s1 = *script_step++;
            s32 s2 = *script_step++;

            if (pm_player.timer % s1 < s2) {
                icon->read_pos = icon->loop_start_pos;
            } else {
                icon->read_pos = (HudScript *)script_step;
            }
            return 1;
        }
        case HUD_ELEMENT_OP_SetTileSize: {
            u8 size_preset = *script_step++;
            icon->width_scale = X10(1);
            icon->height_scale = X10(1);
            icon->draw_size_preset = size_preset;
            icon->tile_size_preset = size_preset;
            icon->flags &= ~HUD_ELEMENT_FLAGS_FIXEDSCALE;
            icon->flags &= ~HUD_ELEMENT_FLAGS_REPEATED;

            icon->read_pos = (HudScript *)script_step;
            return 1;
        }
        case HUD_ELEMENT_OP_SetSizesAutoScale: {
            icon->tile_size_preset = *script_step++;
            icon->draw_size_preset = *script_step++;

            s32 image_width = gHudElementSizes[icon->tile_size_preset].width;
            s32 image_height = gHudElementSizes[icon->tile_size_preset].height;
            s32 draw_width = gHudElementSizes[icon->draw_size_preset].width;
            s32 draw_height = gHudElementSizes[icon->draw_size_preset].height;

            f32 x_scaled = (f32)draw_width / (f32)image_width;
            f32 y_scaled = (f32)draw_height / (f32)image_height;

            x_scaled = 1.0f / x_scaled;
            y_scaled = 1.0f / y_scaled;

            icon->width_scale = X10(x_scaled);
            icon->height_scale = X10(y_scaled);

            icon->flags &= ~HUD_ELEMENT_FLAGS_FIXEDSCALE;
            icon->flags |= HUD_ELEMENT_FLAGS_REPEATED;

            icon->read_pos = (HudScript *)script_step;
            return 1;
        }
        case HUD_ELEMENT_OP_SetSizesFixedScale: {
            s32 tile_size_preset = *script_step++;
            s32 draw_size_preset = *script_step++;

            icon->width_scale = X10(1);
            icon->height_scale = X10(1);
            icon->tile_size_preset = tile_size_preset;
            icon->draw_size_preset = draw_size_preset;
            icon->flags |= HUD_ELEMENT_FLAGS_FIXEDSCALE;
            icon->flags &= ~HUD_ELEMENT_FLAGS_REPEATED;
            icon->flags &= ~HUD_ELEMENT_FLAGS_200;

            icon->read_pos = (HudScript *)script_step;
            return 1;
        }
        case HUD_ELEMENT_OP_AddTexelOffsetX: {
            icon->screen_pos_offset.x += *script_step++;

            icon->read_pos = (HudScript *)script_step;
            return 1;
        }
        case HUD_ELEMENT_OP_AddTexelOffsetY: {
            s32 offset = *script_step++;
            if (!(icon->flags & HUD_ELEMENT_FLAGS_FLIPY)) {
                icon->screen_pos_offset.y += offset;
            } else {
                icon->screen_pos_offset.y -= offset;
            }

            icon->read_pos = (HudScript *)script_step;
            return 1;
        }
        case HUD_ELEMENT_OP_SetTexelOffset: {
            s32 offset_x = *script_step++;
            s32 offset_y = *script_step++;

            icon->screen_pos_offset.x = offset_x;
            if (!(icon->flags & HUD_ELEMENT_FLAGS_FLIPY)) {
                icon->screen_pos_offset.y = offset_y;
            } else {
                icon->screen_pos_offset.y = -offset_y;
            }

            icon->read_pos = (HudScript *)script_step;
            return 1;
        }
        case HUD_ELEMENT_OP_SetScale: {
            s32 image_width, image_height, draw_width, draw_height;

            icon->uniform_scale = ((f32)*script_step++) / 65536;

            if (!(icon->flags & HUD_ELEMENT_FLAGS_CUSTOM_SIZE)) {
                image_width = gHudElementSizes[icon->tile_size_preset].width;
                image_height = gHudElementSizes[icon->tile_size_preset].height;
                draw_width = gHudElementSizes[icon->draw_size_preset].width;
                draw_height = gHudElementSizes[icon->draw_size_preset].height;
            } else {
                image_width = icon->custom_image_size.x;
                image_height = icon->custom_image_size.y;
                draw_width = icon->custom_draw_size.x;
                draw_height = icon->custom_draw_size.y;
            }

            icon->size_x = draw_width * icon->uniform_scale;
            icon->size_y = draw_height * icon->uniform_scale;

            f32 x_scaled = 1.0f / ((f32)draw_width / (f32)image_width * icon->uniform_scale);
            f32 y_scaled = 1.0f / ((f32)draw_height / (f32)image_height * icon->uniform_scale);

            icon->width_scale = X10(x_scaled);
            icon->height_scale = X10(y_scaled);

            icon->flags &= ~HUD_ELEMENT_FLAGS_FIXEDSCALE;
            icon->flags |= HUD_ELEMENT_FLAGS_REPEATED | HUD_ELEMENT_FLAGS_SCALED;

            icon->read_pos = (HudScript *)script_step;
            return 1;
        }
        case HUD_ELEMENT_OP_SetAlpha: {
            icon->alpha = *script_step++;
            icon->flags |= HUD_ELEMENT_FLAGS_TRANSPARENT;
            if (icon->alpha == 255) {
                icon->flags &= ~HUD_ELEMENT_FLAGS_TRANSPARENT;
            }

            icon->read_pos = (HudScript *)script_step;
            return 1;
        }
        case HUD_ELEMENT_OP_RandomDelay: {
            s32 s1 = *script_step++;
            s32 s2 = *script_step++;
            icon->update_timer = pm_player.timer % (s2 - s1) + s1;

            icon->read_pos = (HudScript *)script_step;
            break;
        }
        case HUD_ELEMENT_OP_SetCustomSize: {
            icon->custom_draw_size.x = icon->custom_image_size.x = *script_step++;
            icon->custom_draw_size.y = icon->custom_image_size.y = *script_step++;
            icon->width_scale = X10(1);
            icon->height_scale = X10(1);
            icon->draw_size_preset = 0;
            icon->tile_size_preset = 0;
            icon->flags &= ~HUD_ELEMENT_FLAGS_FIXEDSCALE;
            icon->flags &= ~HUD_ELEMENT_FLAGS_REPEATED;
            icon->flags |= HUD_ELEMENT_FLAGS_CUSTOM_SIZE;

            icon->read_pos = (HudScript *)script_step;
            return 1;
        }
        case HUD_ELEMENT_OP_op_15: {
            s32 flag_var = *script_step++;
            icon->flags &= ~(HUD_ELEMENT_FLAGS_1000000 | HUD_ELEMENT_FLAGS_2000000 | HUD_ELEMENT_FLAGS_4000000 |
                             HUD_ELEMENT_FLAGS_8000000);
            icon->flags |= flag_var << 24;

            icon->read_pos = (HudScript *)script_step;
            return 1;
        }
        case HUD_ELEMENT_OP_RandomBranch: {
            s32 s1 = *script_step++;

            icon->read_pos = (HudScript *)script_step[pm_player.timer % (s1 - 1)];
            game_icon_script_init(icon, icon->read_pos);
            return 1;
        }
        case HUD_ELEMENT_OP_PlaySound: {
            // yeah we're not playing sound from icons lol
            icon->read_pos = (HudScript *)script_step;
            return 1;
        }
        case HUD_ELEMENT_OP_SetPivot: {
            s32 pivot_x = *script_step++;
            s32 pivot_y = *script_step++;
            if (icon->flags & HUD_ELEMENT_FLAGS_TRANSFORM) {
                hudTransform->pivot.x = pivot_x;
                // decomp needed to match, maybe take a look later
                do {
                    hudTransform->pivot.y = pivot_y;
                } while (0);
            }

            icon->read_pos = (HudScript *)script_step;
            return 1;
        }
        case HUD_ELEMENT_OP_op_16: {
            break;
        }
    }
    return 0;
}
