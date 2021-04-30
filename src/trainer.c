#include <math.h>
#include "menu.h"
#include "trainer.h"
#include "settings.h"
#include "gfx.h"
#include "fp.h"

static int iss_draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color, draw_params->alpha));
    struct gfx_font *font = draw_params->font;
    int chHeight = menu_get_cell_height(item->owner, 1);
    int chWidth = menu_get_cell_width(item->owner, 1);
    int x = draw_params->x;
    int y = draw_params->y;

    int xPos = ceil(pm_player.position.x);
    int zPos = ceil(pm_player.position.z);
    _Bool goodPos = 0;
    _Bool willClip = 0;

    if (pm_player.position.z >= -26.3686f) {
        //check if in a known position that will clip and respawn OoB
        if (zPos == -24 && xPos == -184) {
            goodPos = 1;
        }
        else if (zPos == -25 && (xPos >= -186 && xPos <= -183)) {
            goodPos = 1;
        }
        else if (zPos == -26 && (xPos >= -186 && xPos <= -182)) {
            goodPos = 1;
        }

        //check if in a known position that will clip
        if (xPos == -186 && (zPos >= -26 && zPos <= -21)) {
            willClip = 1;
        }
        else if (xPos == -185 && (zPos >= -26 && zPos <= -22)) {
            willClip = 1;
        }
        else if (xPos == -184 && (zPos >= -26 && zPos <= -23)) {
            willClip = 1;
        }
        else if (xPos == -183 && (zPos >= -26 && zPos <= -24)) {
            willClip = 1;
        }
        else if (xPos == -182 && (zPos >= -26 && zPos <= -25)) {
            willClip = 1;
        }
        else if (xPos == -181 && zPos == -26) {
            willClip = 1;
        }
    }

    
    int menuY = 0;
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0xFF, 0xFF, 0xFF));
    gfx_printf(font, x, y + chHeight * menuY++, "x: %.4f", pm_player.position.x);
    gfx_printf(font, x, y + chHeight * menuY++, "z: %.4f", pm_player.position.z);
    gfx_printf(font, x, y + chHeight * menuY, "angle: ");
    if (pm_player.facing_angle >= 43.9f && pm_player.facing_angle <= 46.15f) {
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0xFF, 0x00, 0xFF));
    }
    gfx_printf(font, x + chWidth * 7, y + chHeight * menuY++, "%.2f", pm_player.facing_angle);
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0xFF, 0xFF, 0xFF));
    gfx_printf(font, x, y + chHeight * menuY, "position: ");
    if (goodPos) {
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0xFF, 0x00, 0xFF));
        gfx_printf(font, x + chWidth * 10, y + chHeight * menuY++, "good");
    }
    else if (willClip) {
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0xFF, 0x00, 0xFF));
        gfx_printf(font, x + chWidth * 10, y + chHeight * menuY++, "inconsistent");
    }
    else {
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0x00, 0x00, 0xFF));
        gfx_printf(font, x + chWidth * 10, y + chHeight * menuY++, "bad");
    }
    return 1;
}

static int ace_draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color, draw_params->alpha));
    struct gfx_font *font = draw_params->font;
    int chHeight = menu_get_cell_height(item->owner, 1);
    int chWidth = menu_get_cell_width(item->owner, 1);
    int x = draw_params->x;
    int y = draw_params->y;

    int effect_count = 0;
    int i;
    for (i = 0; i<96; i++) {
        if (pm_effects.effects[i]) {
            effect_count++;
        }
    }

    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0xFF, 0xFF, 0xFF));
    gfx_printf(font, x, y + chHeight * 0, "effects: ");
    if (effect_count == 81) {
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0xFF, 0x00, 0xFF));
    }
    gfx_printf(font, x + chWidth * 14, y + chHeight * 0, "%d", effect_count);
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0xFF, 0xFF, 0xFF));
    gfx_printf(font, x + chWidth * 0, y + chHeight * 1, "flags: ");
    if (pm_player.anim_flags == 0x01000000) {
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0xFF, 0x00, 0xFF));
        gfx_printf(font, x + chWidth * 14, y + chHeight * 1, "good");
    }
    else {
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0x00, 0x00, 0xFF));
        gfx_printf(font, x + chWidth * 14, y + chHeight * 1, "bad");
    }
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0xFF, 0xFF, 0xFF));
    gfx_printf(font, x + chWidth * 0, y + chHeight * 2, "frame window: %d", fp.ace_frame_window);
    return 1;
}

void create_trainer_menu(struct menu *menu)
{
    static struct menu issMenu;
    static struct menu aceMenu;
    
    /* initialize menu */
    menu_init(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&issMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&aceMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu->selector = menu_add_submenu(menu, 0, 0, NULL, "return");

    /*build menu*/
    menu_add_submenu(menu, 0, 1, &issMenu, "ice staircase skip");
    menu_add_submenu(menu, 0, 2, &aceMenu, "oot ace");

    /*build iss menu*/
    issMenu.selector = menu_add_submenu(&issMenu, 0, 0, NULL, "return");
    menu_add_static_custom(&issMenu, 0, 1, iss_draw_proc, NULL, 0xFFFFFF);

    /*build ace menu*/
    aceMenu.selector = menu_add_submenu(&aceMenu, 0, 0, NULL, "return");
    menu_add_static_custom(&aceMenu, 0, 1, ace_draw_proc, NULL, 0xFFFFFF);
}