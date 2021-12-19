#include <math.h>
#include "menu.h"
#include "trainer.h"
#include "settings.h"
#include "fp.h"

char messageForASM[] = "Success";

extern void setACEHook(void);

int getMatrixTotal(void) {
    int matrixCount = 0;

    for (int i = 0; i < 0x60; i++) {
        if (pm_effects[i] != NULL) {
            matrixCount += pm_effects[i]->matrixTotal;
        }
    }
    return matrixCount;
}

void clearAllEffectsManual(int matrixCount) {
    int var = 0;

    if (matrixCount == 0x215) {
        var = 1;
        fp.ace_last_flag_status = pm_player.anim_flags == 0x01000000;
        fp.ace_last_timer = pm_player.idle_timer;
        fp.ace_last_jump_status = (pm_player.flags & 0xff) == 3;
        fp_log("Successful ACE, jump prevented");
    }
    if (matrixCount > 0x215) { // matrix limit reached, destroy all effects
        var = 1;
        fp_log("Matrix overflow, crash prevented");
    }

    if (var == 1) {
        for (int i = 0; i < 0x60; i++) {
            if (pm_effects[i] != NULL) {
                pm_RemoveEffect(pm_effects[i]);
            }
        }
    }
}

asm(".set noreorder;"
    "ace_remove_matrices:;"
    "JAL getMatrixTotal;"
    "NOP;"
    "JAL clearAllEffectsManual;"
    "DADDU $a0, $v0, $zero;"
    "LUI $s1, 0x800B;"
    "ADDIU $s1, $s1, 0x4378;"
    "J 0x80059A7C;"
    "NOP;"
    "jump_instruction_ace_remove_matrices:;"
    "J ace_remove_matrices;"
    "NOP;");

asm(".set noreorder;"
    "setACEHook:;"
    "LA $t0, 0x80059A74;" // where to hook
    "LA $t1, jump_instruction_ace_remove_matrices;"
    "LW $t1, 0x0000 ($t1);"
    "SW $zero, 0x0004 ($t0);"
    "JR $ra;"
    "SW $t1, 0x0000 ($t0);");

static int checkbox_mod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    uint8_t *p = data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        *p = 1;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        *p = 0;
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, *p);
    }
    return 0;
}

static int byte_optionmod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    uint8_t *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != *p) {
            menu_option_set(item, *p);
        }
    } else if (reason == MENU_CALLBACK_DEACTIVATE) {
        *p = menu_option_get(item);
    }
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
        // check if in a known position that will clip and respawn OoB
        if (zPos == -24 && xPos == -184) {
            goodPos = 1;
        } else if (zPos == -25 && (xPos >= -186 && xPos <= -183)) {
            goodPos = 1;
        } else if (zPos == -26 && (xPos >= -186 && xPos <= -182)) {
            goodPos = 1;
        }

        // check if in a known position that will clip
        if (xPos == -186 && (zPos >= -26 && zPos <= -21)) {
            willClip = 1;
        } else if (xPos == -185 && (zPos >= -26 && zPos <= -22)) {
            willClip = 1;
        } else if (xPos == -184 && (zPos >= -26 && zPos <= -23)) {
            willClip = 1;
        } else if (xPos == -183 && (zPos >= -26 && zPos <= -24)) {
            willClip = 1;
        } else if (xPos == -182 && (zPos >= -26 && zPos <= -25)) {
            willClip = 1;
        } else if (xPos == -181 && zPos == -26) {
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
    } else if (willClip) {
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0xFF, 0x00, 0xFF));
        gfx_printf(font, x + chWidth * 10, y + chHeight * menuY++, "inconsistent");
    } else {
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
    for (i = 0; i < 96; i++) {
        if (pm_effects[i]) {
            effect_count++;
        }
    }

    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xC0, 0xC0, 0xC0, 0xFF)); // gray
    gfx_printf(font, x + chWidth * 0, y + chHeight * 0, "effects:");
    gfx_printf(font, x + chWidth * 0, y + chHeight * 1, "flags:");
    gfx_printf(font, x + chWidth * 0, y + chHeight * 2, "frame window:");

    if (effect_count == 81) {
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0xFF, 0x00, 0xFF)); // green
    } else {
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0xFF, 0xFF, 0xFF)); // white
    }
    gfx_printf(font, x + chWidth * 14, y + chHeight * 0, "%d", effect_count);
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0xFF, 0xFF, 0xFF)); // white
    if (pm_player.anim_flags == 0x01000000) {
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0xFF, 0x00, 0xFF)); // green
        gfx_printf(font, x + chWidth * 14, y + chHeight * 1, "good");
    } else {
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0x00, 0x00, 0xFF)); // red
        gfx_printf(font, x + chWidth * 14, y + chHeight * 1, "bad");
    }
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0xFF, 0xFF, 0xFF)); // white
    gfx_printf(font, x + chWidth * 14, y + chHeight * 2, "%d", fp.ace_frame_window);

    if (fp.ace_last_timer != 0) {
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xC0, 0xC0, 0xC0, 0xFF)); // gray
        gfx_printf(font, x + chWidth * 0, y + chHeight * 7, "last attempt status:");
        gfx_printf(font, x + chWidth * 0, y + chHeight * 8, "timer:");
        gfx_printf(font, x + chWidth * 0, y + chHeight * 9, "flags:");
        gfx_printf(font, x + chWidth * 0, y + chHeight * 10, "jump:");

        if (fp.ace_last_timer <= 0x81f && fp.ace_last_timer > 0x81f - fp.ace_frame_window) {
            gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0xFF, 0x00, 0xFF)); // green
        } else {
            gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0x00, 0x00, 0xFF)); // red
        }
        gfx_printf(font, x + chWidth * 7, y + chHeight * 8, "0x%x", fp.ace_last_timer);

        if (fp.ace_last_flag_status) {
            gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0xFF, 0x00, 0xFF)); // green
            gfx_printf(font, x + chWidth * 7, y + chHeight * 9, "good");
        } else {
            gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0x00, 0x00, 0xFF)); // red
            gfx_printf(font, x + chWidth * 7, y + chHeight * 9, "bad");
        }

        if (fp.ace_last_jump_status) {
            gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0xFF, 0x00, 0xFF)); // green
            gfx_printf(font, x + chWidth * 7, y + chHeight * 10, "good");
        } else {
            gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0x00, 0x00, 0xFF)); // red
            gfx_printf(font, x + chWidth * 7, y + chHeight * 10, "bad");
        }

        if (fp.ace_last_flag_status && fp.ace_last_jump_status && fp.ace_last_timer <= 0x81f &&
            fp.ace_last_timer > 0x81f - fp.ace_frame_window) {
            gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0x00, 0xFF, 0x00, 0xFF)); // green
            gfx_printf(font, x + chWidth * 0, y + chHeight * 11, "success");
        } else {
            gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0x00, 0x00, 0xFF)); // red
            gfx_printf(font, x + chWidth * 0, y + chHeight * 11, "failure");
        }
    }
    return 1;
}

static void ace_practice_payload_proc(struct menu_item *item, void *data) {
    setACEHook();
    fp_log("practice payload placed");
}

static void ace_oot_instr_proc(struct menu_item *item, void *data) {
    // write jump to jp file names to addr 0x807C0000
    fp_log("oot instruction placed");
    __asm__("LA $t0, 0x807C0000;"
            "LA $t1, 0x0801DE67;"
            "SW $t1, 0x0000 ($t0);");
}

static int lzs_draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color, draw_params->alpha));
    struct gfx_font *font = draw_params->font;
    int chHeight = menu_get_cell_height(item->owner, 1);
    int chWidth = menu_get_cell_width(item->owner, 1);
    int x = draw_params->x;
    int y = draw_params->y;

    gfx_printf(font, x, y + chHeight * 1, "current lzs jumps: ");
    gfx_printf(font, x + chWidth * 20, y + chHeight * 1, "%d", fp.current_lzs_jumps);
    gfx_printf(font, x + chWidth * 0, y + chHeight * 2, "record lzs jumps: ");
    gfx_printf(font, x + chWidth * 20, y + chHeight * 2, "%d", fp.record_lzs_jumps);

    return 1;
}

void create_trainer_menu(struct menu *menu) {
    static struct menu bowserMenu;
    static struct menu issMenu;
    static struct menu aceMenu;
    static struct menu lzsMenu;
    static struct menu clippyMenu;

    /* initialize menu */
    menu_init(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&bowserMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&issMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&aceMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&lzsMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&clippyMenu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu->selector = menu_add_submenu(menu, 0, 0, NULL, "return");

    /*build menu*/
    int y_value = 1;
    menu_add_submenu(menu, 0, y_value++, &bowserMenu, "bowser blocks");
    menu_add_submenu(menu, 0, y_value++, &issMenu, "ice staircase skip");
#if PM64_VERSION == 'JP'
    menu_add_submenu(menu, 0, y_value++, &aceMenu, "oot ace"); // TODO: add english support for ace
#endif
    menu_add_submenu(menu, 0, y_value++, &lzsMenu, "lzs jumps");
    menu_add_submenu(menu, 0, y_value++, &clippyMenu, "clippy");

    /*build bowser menu*/
    y_value = 0;
    bowserMenu.selector = menu_add_submenu(&bowserMenu, 0, y_value++, NULL, "return");
    menu_add_static(&bowserMenu, 0, y_value, "enabled", 0xC0C0C0);
    menu_add_checkbox(&bowserMenu, 8, y_value++, checkbox_mod_proc, &fp.bowser_blocks_enabled);
    menu_add_static(&bowserMenu, 0, y_value, "attack", 0xC0C0C0);
    menu_add_option(&bowserMenu, 8, y_value++,
                    "fire\0"
                    "butt stomp\0"
                    "claw\0"
                    "wave\0"
                    "lightning\0",
                    byte_optionmod_proc, &fp.bowser_block);

    /*build iss menu*/
    issMenu.selector = menu_add_submenu(&issMenu, 0, 0, NULL, "return");
    menu_add_static_custom(&issMenu, 0, 1, iss_draw_proc, NULL, 0xFFFFFF);

    /*build ace menu*/
    aceMenu.selector = menu_add_submenu(&aceMenu, 0, 0, NULL, "return");
    menu_add_static_custom(&aceMenu, 0, 1, ace_draw_proc, NULL, 0xFFFFFF);
    menu_add_button(&aceMenu, 0, 5, "practice payload", ace_practice_payload_proc, NULL);
    menu_add_button(&aceMenu, 0, 6, "oot instruction", ace_oot_instr_proc, NULL);

    /*build lzs jump menu*/
    lzsMenu.selector = menu_add_submenu(&lzsMenu, 0, 0, NULL, "return");
    menu_add_static(&lzsMenu, 0, 1, "enabled", 0xC0C0C0);
    menu_add_checkbox(&lzsMenu, 8, 1, checkbox_mod_proc, &fp.lzs_trainer_enabled);
    menu_add_static_custom(&lzsMenu, 0, 2, lzs_draw_proc, NULL, 0xFFFFFF);

    /*build clippy menu*/
    clippyMenu.selector = menu_add_submenu(&clippyMenu, 0, 0, NULL, "return");
    menu_add_static(&clippyMenu, 0, 1, "enabled", 0xC0C0C0);
    menu_add_checkbox(&clippyMenu, 8, 1, checkbox_mod_proc, &fp.clippy_trainer_enabled);
}
