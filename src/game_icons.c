#include <stdlib.h>
#include "enums.h"
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
static game_icon *game_icon_create(game_icon *icon, HudScript *script);
static game_icon *game_icon_init(HudScript *script);

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

void game_icons_set_render_pos(game_icon *icon, s32 x, s32 y) {
    icon->render_pos_x = x;
    icon->render_pos_y = y;
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

static void game_icon_set_flags(game_icon *icon, s32 flags) {
    icon->flags |= flags;
}

static void game_icon_clear_flags(game_icon *icon, s32 flags) {
    icon->flags &= ~flags;
}

static void game_icon_script_init(game_icon *icon, HudScript *script) {
    s32 *script_step = (s32 *)script;

    if (script_step == NULL) {
        // PRINTF("attempted to initialize icon, but script was NULL\n");
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
            icon->raster_addr = (u8 *)*script_step++;

            if (icon->flags & HUD_ELEMENT_FLAGS_MEMOFFSET) {
                icon->raster_addr += icon->mem_offset;
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
        case HUD_ELEMENT_OP_SetCI: {
            icon->update_timer = *script_step++;

            if (icon->flags & HUD_ELEMENT_FLAGS_FIXEDSCALE) {}
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
            icon->raster_addr = get_cached_raster(raster_rom, raster_size);
            icon->palette_addr = get_cached_palette(palette_rom);

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
            icon->update_timer = *script_step++;
            s32 raster_addr = *script_step++;
            s32 palette_addr = *script_step++;
            script_step += 3;

            icon->raster_addr =
                get_cached_raster(ICONS_ITEMS_ROM_START + raster_addr, gHudElementSizes[icon->tile_size_preset].size);
            icon->palette_addr = get_cached_palette(ICONS_ITEMS_ROM_START + palette_addr);

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

static game_icon *game_icon_create(game_icon *icon, HudScript *script) {
    icon->flags = HUD_ELEMENT_FLAGS_INITIALIZED;
    icon->read_pos = script;
    icon->update_timer = 1;
    icon->draw_size_preset = -1;
    icon->tile_size_preset = -1;
    icon->render_pos_x = 0;
    icon->render_pos_y = 0;
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

    game_icon_script_init(icon, icon->read_pos);
    while (game_icon_update(icon) != 0) {};
    return icon;
}

static game_icon *game_icon_init(HudScript *script) {
    game_icon *icon = malloc(sizeof(*icon));
    icon = game_icon_create(icon, script);
    return icon;
}

game_icon *game_icons_create_global(icon_global icon) {
    s32 script_data[] = {icon};
    return game_icon_init(create_hud_script(SCRIPT_TYPE_GLOBAL, script_data));
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
    return game_icon_init(create_hud_script(SCRIPT_TYPE_PARTNER, data));
}

game_icon *game_icons_create_item(Item item, _Bool grayscale) {
    s32 data[] = {item, grayscale};
    return game_icon_init(create_hud_script(SCRIPT_TYPE_ITEM, data));
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
            return next_icon;
        }
    }
    idx = 0;
    return NULL;
}
