#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "menu.h"
#include "settings.h"
#include "pm64.h"
#include "warp_info.h"

static uint16_t group;
static uint16_t room;
static uint16_t entrance;

static void group_prev_proc(struct menu_item *item, void *data) {
    if (group == 0) {
        group = GROUP_COUNT - 1;
    }
    else {
        group--;
    }

    return;
}

static void group_next_proc(struct menu_item *item, void *data) {
    if (group == GROUP_COUNT - 1) {
        group = 0;
    }
    else {
        group++;
    }

    return;
}

static void room_prev_proc(struct menu_item *item, void *data) {
    if (room == 0) {
        room = GROUPS[group].room_count - 1;
    }
    else {
        room--;
    }

    return;
}

static void room_next_proc(struct menu_item *item, void *data) {
    if (room == GROUPS[group].room_count - 1) {
        room = 0;
    }
    else {
        room++;
    }

    return;
}

static void entrance_prev_proc(struct menu_item *item, void *data) {
    if (entrance == 0) {
        entrance = GROUPS[group].rooms[room].entrance_count - 1;
    }
    else {
        entrance--;
    }

    return;
}

static void entrance_next_proc(struct menu_item *item, void *data) {
    if (entrance == GROUPS[group].rooms[room].entrance_count - 1) {
        entrance = 0;
    }
    else {
        entrance++;
    }

    return;
}

static int warp_info_draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color,
                                               draw_params->alpha));
    struct gfx_font *font = draw_params->font;
    int ch = menu_get_cell_height(item->owner, 1);
    int x = draw_params->x;
    int y = draw_params->y;

    if (room >= GROUPS[group].room_count) {
        room = 0;
    }
    if (entrance >= GROUPS[group].rooms[room].entrance_count) {
        entrance = 0;
    }

    gfx_printf(font, x, y + ch * 0, "%s", GROUPS[group].group_name);
    gfx_printf(font, x, y + ch * 3, "%s", GROUPS[group].rooms[room].room_name);
    gfx_printf(font, x, y + ch * 6, "%d/%d", entrance, GROUPS[group].rooms[room].entrance_count);

    return 1;
}

static int current_room_draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
  gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color,
                                             draw_params->alpha));
  struct gfx_font *font = draw_params->font;
  int ch = menu_get_cell_height(item->owner, 1);
  int x = draw_params->x;
  int y = draw_params->y;
  gfx_printf(font, x, y + ch * 0, "current room");
  gfx_printf(font, x, y + ch * 1, "g: %s", GROUPS[pm_status.group_id].group_name);
  gfx_printf(font, x, y + ch * 2, "r: %s", GROUPS[pm_status.group_id].rooms[pm_status.room_id].room_name);
  gfx_printf(font, x, y + ch * 3, "e: %x", pm_status.entrance_id);

  return 1;
}

static void warp_proc() {
    //would be nice to know why warping from this room crashes
    if (!(pm_status.group_id == 0 && pm_status.room_id == 0xe)) {
        pm_PlayAmbientSounds(-1, 0);
        pm_status.loading_zone_tangent = 0;
        pm_status.group_id = group;
        pm_status.room_id = room;
        pm_status.entrance_id = entrance;

        pm_unk2.room_change_state = 1;

        uint32_t val = 0x80035DFC;
        pm_warp.room_change_ptr = val;
    }

    return;
}

struct menu *create_warps_menu(void) {
    static struct menu menu;
    
    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu.selector = menu_add_submenu(&menu, 0, 0, NULL, "return");

    menu_add_static(&menu, 0, 1, "group:", 0xC0C0C0);
    menu_add_button(&menu, 0, 2, "<", group_prev_proc, NULL);
    menu_add_button(&menu, 1, 2, ">", group_next_proc, NULL);

    menu_add_static(&menu, 0, 4, "room:", 0xC0C0C0);
    menu_add_button(&menu, 0, 5, "<", room_prev_proc, NULL);
    menu_add_button(&menu, 1, 5, ">", room_next_proc, NULL);

    menu_add_static(&menu, 0, 7, "entrance:", 0xC0C0C0);
    menu_add_button(&menu, 0, 8, "<", entrance_prev_proc, NULL);
    menu_add_button(&menu, 1, 8, ">", entrance_next_proc, NULL);

    menu_add_static_custom(&menu, 3, 2, warp_info_draw_proc, NULL, 0xC0C0C0);

    menu_add_button(&menu, 0, 9, "warp", warp_proc, NULL);

    menu_add_static_custom(&menu, 0, 12, current_room_draw_proc, NULL, 0xC0C0C0);


    /*build menu*/
    
    return &menu;
}
