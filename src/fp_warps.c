#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "menu.h"
#include "settings.h"
#include "pm64.h"

const char *GROUPS = "00 goomba village\0""01 toad town\0""02 sewers\0""03 whale\0""04 peach's castle\0"
"05 star haven\0""06 koopa village\0""07 koopa bros. fortress\0""08 mt. rugged\0""09 dry dry outpost\0"
"0a dry dry desert\0""0b dry dry ruins\0""0c forever forest\0""0d boo's mansion\0""0e gusty gulch\0"
"0f tubba's mansion\0""10 shy guy's toybox\0""11 lava lava island\0""12 volcano\0""13 flower fields\0""14 shiver city\0"
"15 crystal palace\0""16 bowser's castle\0""17 outside peach's castle\0""18 credits\0""19 mini games\0""1a gave over\0"
"1b debug\0";

static uint16_t group;
static uint16_t room;
static uint16_t entrance;

static int halfword_mod_proc(struct menu_item *item,
                             enum menu_callback_reason reason,
                             void *data)
{
  uint16_t *p = data;
  if (reason == MENU_CALLBACK_THINK_INACTIVE) {
    if (menu_intinput_get(item) != *p)
      menu_intinput_set(item, *p);
  }
  else if (reason == MENU_CALLBACK_CHANGED)
    *p = menu_intinput_get(item);
  return 0;
}

static int option_mod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    uint16_t *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != *p)
            menu_option_set(item, *p);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        *p = menu_option_get(item);
    return 0;
}

static int warp_info_draw_proc(struct menu_item *item,
                               struct menu_draw_params *draw_params)
{
  gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color,
                                             draw_params->alpha));
  struct gfx_font *font = draw_params->font;
  int ch = menu_get_cell_height(item->owner, 1);
  int x = draw_params->x;
  int y = draw_params->y;
  gfx_printf(font, x, y + ch * 0, "current group: %" PRIx16, pm_status.group_id);
  gfx_printf(font, x, y + ch * 1, "current  room: %" PRIx16, pm_status.room_id);
  gfx_printf(font, x, y + ch * 2, "last entrance: %" PRIx16, pm_status.entrance_id);

  return 1;
}

static void warp_proc(){
	pm_status.group_id    = group;
	pm_status.room_id     = room;
	pm_status.entrance_id = entrance;

	pm_unk2.room_change_state = 1;

	uint32_t val = 0x80035DFC;
	pm_warp.room_change_ptr = val;

}

struct menu *create_warps_menu(void)
{
    static struct menu menu;
    
    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu.selector = menu_add_submenu(&menu, 0, 0, NULL, "return");

    menu_add_static(&menu, 0, 1, "places", 0xFFFFFF);

    menu_add_static(&menu, 0, 2, "group:", 0xC0C0C0);
    menu_add_option(&menu, 10, 2, GROUPS, option_mod_proc, &group);

    menu_add_static(&menu, 0, 3, "room:", 0xC0C0C0);
    menu_add_intinput(&menu, 10, 3, 16, 2, halfword_mod_proc, &room);

    menu_add_static(&menu, 0, 4, "entrance:", 0xC0C0C0);
    menu_add_intinput(&menu, 10, 4, 16, 2, halfword_mod_proc, &entrance);

    menu_add_button(&menu, 0, 5, "warp", warp_proc, NULL);

    menu_add_static_custom(&menu, 0, 7, warp_info_draw_proc, NULL, 0xC0C0C0);


    /*build menu*/
    
    return &menu;
}
