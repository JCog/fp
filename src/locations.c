#include "common.h"
#include "fp.h"
#include "menu.h"
#include "warp_info.h"

static u16 area;
static u16 map;
static u16 entrance;

static void area_prev_proc(struct menu_item *item, void *data) {
    if (area == 0) {
        area = AREA_COUNT - 1;
    } else {
        area--;
    }

    return;
}

static void area_next_proc(struct menu_item *item, void *data) {
    if (area == AREA_COUNT - 1) {
        area = 0;
    } else {
        area++;
    }

    return;
}

static void map_prev_proc(struct menu_item *item, void *data) {
    if (map == 0) {
        map = AREAS[area].map_count - 1;
    } else {
        map--;
    }

    return;
}

static void map_next_proc(struct menu_item *item, void *data) {
    if (map == AREAS[area].map_count - 1) {
        map = 0;
    } else {
        map++;
    }

    return;
}

static void entrance_prev_proc(struct menu_item *item, void *data) {
    if (entrance == 0) {
        entrance = AREAS[area].maps[map].entrance_count - 1;
    } else {
        entrance--;
    }

    return;
}

static void entrance_next_proc(struct menu_item *item, void *data) {
    if (entrance == AREAS[area].maps[map].entrance_count - 1) {
        entrance = 0;
    } else {
        entrance++;
    }

    return;
}

static s32 warp_info_draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color, draw_params->alpha));
    struct gfx_font *font = draw_params->font;
    s32 ch = menu_get_cell_height(item->owner, TRUE);
    s32 x = draw_params->x;
    s32 y = draw_params->y;

    if (map >= AREAS[area].map_count) {
        map = 0;
    }
    if (entrance >= AREAS[area].maps[map].entrance_count) {
        entrance = 0;
    }

    gfx_printf(font, x, y + ch * 0, "%x %s", area, AREAS[area].area_name);
    gfx_printf(font, x, y + ch * 3, "%x %s", map, AREAS[area].maps[map].map_name);
    gfx_printf(font, x, y + ch * 6, "%d/%d", entrance, AREAS[area].maps[map].entrance_count);

    return 1;
}

static s32 current_map_draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color, draw_params->alpha));
    struct gfx_font *font = draw_params->font;
    s32 ch = menu_get_cell_height(item->owner, TRUE);
    s32 x = draw_params->x;
    s32 y = draw_params->y;
    gfx_printf(font, x, y + ch * 0, "current map");
    gfx_printf(font, x, y + ch * 1, "a: %x %s", pm_gGameStatus.areaID, AREAS[pm_gGameStatus.areaID].area_name);
    gfx_printf(font, x, y + ch * 2, "m: %x %s", pm_gGameStatus.mapID,
               AREAS[pm_gGameStatus.areaID].maps[pm_gGameStatus.mapID].map_name);
    gfx_printf(font, x, y + ch * 3, "e: %x", pm_gGameStatus.entryID);

    return 1;
}

static void warp_proc(struct menu_item *item, void *data) {
    if (fp_warp(area, map, entrance)) {
        fp.saved_area = area;
        fp.saved_map = map;
        fp.saved_entrance = entrance;
    }
}

void create_locations_menu(struct menu *menu) {

    /* initialize menu */
    menu_init(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu->selector = menu_add_submenu(menu, 0, 0, NULL, "return");

    menu_add_static(menu, 0, 1, "area:", 0xC0C0C0);
    menu_add_button(menu, 0, 2, "<", area_prev_proc, NULL);
    menu_add_button(menu, 1, 2, ">", area_next_proc, NULL);

    menu_add_static(menu, 0, 4, "map:", 0xC0C0C0);
    menu_add_button(menu, 0, 5, "<", map_prev_proc, NULL);
    menu_add_button(menu, 1, 5, ">", map_next_proc, NULL);

    menu_add_static(menu, 0, 7, "entrance:", 0xC0C0C0);
    menu_add_button(menu, 0, 8, "<", entrance_prev_proc, NULL);
    menu_add_button(menu, 1, 8, ">", entrance_next_proc, NULL);

    menu_add_static_custom(menu, 3, 2, warp_info_draw_proc, NULL, 0xC0C0C0);

    menu_add_button(menu, 0, 9, "warp", warp_proc, NULL);

    menu_add_static_custom(menu, 0, 12, current_map_draw_proc, NULL, 0xC0C0C0);
}
