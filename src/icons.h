#ifndef ITEM_ICONS_H
#define ITEM_ICONS_H

#include "fp.h"
#include "gfx.h"

#define HUD_ELEMENT_LIST_SIZE 200
#define RASTER_CACHE_SIZE     300
#define PALETTE_CACHE_SIZE    300

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

/// Basic HudScript used for static CI images, setting size with hs_SetTileSize
#define HES_TEMPLATE_CI_ENUM_SIZE(raster, palette, sizeX, sizeY)                                               \
    {                                                                                                          \
        hs_SetVisible hs_SetTileSize(HUD_ELEMENT_SIZE_##sizeX##x##sizeY) hs_Loop hs_SetCI(60, raster, palette) \
            hs_Restart hs_End                                                                                  \
    }

/// Basic HudScript used for static CI images, setting size with hs_SetCustomSize
#define HES_TEMPLATE_CI_CUSTOM_SIZE(raster, palette, sizeX, sizeY) \
    { hs_SetVisible hs_SetCustomSize(sizeX, sizeY) hs_Loop hs_SetCI(60, raster, palette) hs_Restart hs_End }

enum hud_script_type {
    SCRIPT_TYPE_ITEM,
    SCRIPT_TYPE_PARTNER
};

s32 hud_element_update(HudElement *hud_element); /// @returns 0 if frame is completed

HudElement *get_hud_element_item(Item item, s32 x, s32 y, u8 alpha, f32 scale, _Bool grayscale);
HudElement *get_hud_element_partner(u8 partner, s32 x, s32 y, u8 alpha, f32 scale, _Bool grayscale);

#endif
