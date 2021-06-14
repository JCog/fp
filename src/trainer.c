#include <math.h>
#include "menu.h"
#include "trainer.h"
#include "settings.h"
#include "gfx.h"
#include "fp.h"

static int checkbox_mod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    uint8_t *p = data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        *p = 1;
    }
    else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        *p = 0;
    }
    else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, *p);
    }
    return 0;
}

static int byte_optionmod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    uint8_t *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != *p)
            menu_option_set(item, *p);
    }
    else if (reason == MENU_CALLBACK_DEACTIVATE)
        *p = menu_option_get(item);
    return 0;
}

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

static void ace_practice_payload_proc(struct menu_item *item, void *data) {
    pm_ace.instructions[0] = 0x3C188010;  // LUI t8, 0x8010
    pm_ace.instructions[1] = 0x3718F190;  // ORI t8, t8, 0xf190
    pm_ace.instructions[2] = 0x8F090000;  // LW  t1, 0x0000(t8)
    pm_ace.instructions[3] = 0x3C18807D;  // LUI t8, 0x807D
    pm_ace.instructions[4] = 0xAF090000;  // SW  t1, 0x0000(t8)
    //
    pm_ace.instructions[5] = 0x3C088006;  // LUI t0, 0x8006
    pm_ace.instructions[6] = 0xAD00A1C0;  // SW r0, 0xA1C0 (t0) - patch remove all effects
    pm_ace.instructions[7] = 0x0C016864;  // JAL 0x8005A190 - remove all effects
    pm_ace.instructions[8] = 0x00000000;  // NOP
    pm_ace.instructions[9] = 0x3C088006;  // LUI t0, 0x8006
    pm_ace.instructions[10] = 0x3C093042; // LUI t1, 0x3042
    pm_ace.instructions[11] = 0x35290004; // ORI t1, t1, 0x0004
    pm_ace.instructions[12] = 0xAD09A1C0; // SW t1, 0xA1C0 (t0) - fix effects function we patched
    pm_ace.instructions[13] = 0x8FBF0070; // LW ra, 0x0070 (sp)
    pm_ace.instructions[14] = 0x8FB10068; // LW s1, 0x0068 (sp)
    pm_ace.instructions[15] = 0x8FB00064; // LW s0, 0x0064 (sp)
    pm_ace.instructions[16] = 0x0800F50C; // J 0x8003D430
    pm_ace.instructions[17] = 0x27BDFF98; // ADDIU sp, sp, -0x68
    fp_log("practice payload placed");
}

static void ace_oot_instr_proc(struct menu_item *item, void *data) {
    for (int i = 0; i < 18; i++) {
        pm_ace.instructions[i] = 0;
    }
    pm_ace.instructions[2] = 0x0801DE67;
    fp_log("oot instruction placed");
}

void create_trainer_menu(struct menu *menu)
{
    static struct menu bowserMenu;
    static struct menu issMenu;
    static struct menu aceMenu;
    
    /* initialize menu */
    menu_init(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&bowserMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&issMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&aceMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu->selector = menu_add_submenu(menu, 0, 0, NULL, "return");

    /*build menu*/
    menu_add_submenu(menu, 0, 1, &bowserMenu, "bowser blocks");
    menu_add_submenu(menu, 0, 2, &issMenu, "ice staircase skip");
    menu_add_submenu(menu, 0, 3, &aceMenu, "oot ace");

    /*build bowser menu*/
    int y_value = 0;
    bowserMenu.selector = menu_add_submenu(&bowserMenu, 0, y_value++, NULL, "return");
    menu_add_static(&bowserMenu, 0, y_value, "enabled", 0xC0C0C0);
    menu_add_checkbox(&bowserMenu, 8, y_value++, checkbox_mod_proc, &fp.bowser_blocks_enabled);
    menu_add_static(&bowserMenu, 0, y_value, "attack", 0xC0C0C0);
    menu_add_option(&bowserMenu, 8, y_value++, "fire\0""butt stomp\0""claw\0""wave\0""lightning\0", byte_optionmod_proc, &fp.bowser_block);

    /*build iss menu*/
    issMenu.selector = menu_add_submenu(&issMenu, 0, 0, NULL, "return");
    menu_add_static_custom(&issMenu, 0, 1, iss_draw_proc, NULL, 0xFFFFFF);

    /*build ace menu*/
    aceMenu.selector = menu_add_submenu(&aceMenu, 0, 0, NULL, "return");
    menu_add_static_custom(&aceMenu, 0, 1, ace_draw_proc, NULL, 0xFFFFFF);
    menu_add_button(&aceMenu, 0, 5, "practice payload", ace_practice_payload_proc, NULL);
    menu_add_button(&aceMenu, 0, 6, "oot instruction", ace_oot_instr_proc, NULL);
}
