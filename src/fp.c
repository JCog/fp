#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <startup.h>
#include <mips.h>
#include <n64.h>
#include "fp.h"
#include "gfx.h"
#include "input.h"
#include "commands.h"
#include "resource.h"

__attribute__((section(".data")))
fp_ctxt_t fp = { 
    .ready = 0, 
};

void fp_main(void){
    gfx_mode_init();

    /*hard code font for now. add to settings later*/
    int alpha = 0xFF;
    gfx_mode_configure(GFX_MODE_TEXT, GFX_TEXT_FAST);
    struct gfx_font *font = resource_get(0);
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0xFF, 0xFF, alpha));

    /*draw input display*/
    /*
    {   
        int input_display_x = 16;
        int input_display_y = 210;

        uint16_t d_pad = pm_status.raw.buttons;
        int8_t   d_x   = pm_status.control_x;
        int8_t   d_y   = pm_status.control_y;

        struct gfx_texture *texture = resource_get(RES_ICON_BUTTONS);
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xC0, 0xC0, 0xC0, alpha));
        gfx_printf(font, input_display_x, input_display_y,
                   "%4i %4i", d_x, d_y);
        static const int buttons[] =
        {
          15, 14, 12, 3, 2, 1, 0, 13, 5, 4, 11, 10, 9, 8,
        };
        for (int i = 0; i < sizeof(buttons) / sizeof(*buttons); ++i) {
          int b = buttons[i];
          if (!(d_pad & (1 << b)))
            continue;
          int x = (cw - texture->tile_width) / 2 + i * 10;
          int y = -(gfx_font_xheight(font) + texture->tile_height + 1) / 2;
          struct gfx_sprite sprite =
          {
            texture, b,
            input_display_x + cw * 10 + x, input_display_y + y,
            1.f, 1.f,
          };
          gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(input_button_color[b],
                                                     alpha));
          gfx_sprite_draw(&sprite);
      }
    */
    /* activate cheats */
    {
        if(fp.cheats & (1 << CHEAT_HP)){
            pm_player.stats.hp = 99;
            pm_player.stats.max_hp = 99;
            pm_player.stats.menu_max_hp = 99;
            pm_hud.hp_value = 99;
        }
        if(fp.cheats & (1 << CHEAT_FP)){
            pm_player.stats.fp = 99;
            pm_player.stats.max_fp = 99;
            pm_player.stats.menu_max_fp = 99;
            pm_hud.fp_value = 99;
        }
        if(fp.cheats & (1 << CHEAT_BP)){
            pm_player.stats.bp = 99;
        }
        if(fp.cheats & (1 << CHEAT_COINS)){
            pm_player.stats.coins = 99;
            pm_hud.coin_value = 99;
        }
        if(fp.cheats & (1 << CHEAT_STAR_POWER)){
            
        }
        if(fp.cheats & (1 << CHEAT_STAR_POINTS)){
            pm_player.stats.star_points = 99;
        }
        if(fp.cheats & (1 << CHEAT_STAR_PIECES)){
            pm_player.stats.star_pieces = 160;
        }
        if(fp.cheats & (1 << CHEAT_PERIL)){
            pm_player.stats.hp = 1;
            pm_hud.hp_value = 1;
        }
            
    }


    /* handle command bindings */
    {
        for(int i=0;i<7;i++){
            _Bool activate = 0;
            switch(fp_commands[i].type){
                case COMMAND_HOLD:
                    activate = input_bind_held(i);
                    break;
                case COMMAND_PRESS:
                    activate = input_bind_pressed(i);
                    break;
            }
            if(activate && fp_commands[i].proc){
                fp_commands[i].proc();
            }
        }
    }

gfx_flush();
}

void gamestate_main(){
    pm_GameUpdate(); /* displaced function call - advances 1 game frame*/

}

void init(){
    clear_bss();
    do_global_ctors();

    gfx_start();
    gfx_mode_configure(GFX_MODE_FILTER, G_TF_POINT);
    gfx_mode_configure(GFX_MODE_COMBINE, G_CC_MODE(G_CC_MODULATEIA_PRIM,G_CC_MODULATEIA_PRIM));

    /*hard coded button bindings*/
    fp_commands[0].bind = make_bind(2, BUTTON_R, BUTTON_D_UP);
    fp_commands[1].bind = make_bind(2, BUTTON_R, BUTTON_D_RIGHT);
    fp_commands[2].bind = make_bind(1, BUTTON_D_UP);
    fp_commands[3].bind = make_bind(1, BUTTON_D_DOWN);
    fp_commands[4].bind = make_bind(1, BUTTON_D_LEFT);
    fp_commands[5].bind = make_bind(1, BUTTON_D_RIGHT);
    fp_commands[6].bind = make_bind(2, BUTTON_R, BUTTON_D_DOWN);
    

    /*init menu and default it to inactive*/
    /*
    fp.menu_active = 0;
    menu_init(&fp.main_menu, 15, 55);
    fp.main_menu.selected_item = menu_add_button(&fp.main_menu,0,0,"return",menu_return,NULL);
    menu_add_submenu(&fp.main_menu,0,1,create_warps_menu(),"warps");
    menu_add_submenu(&fp.main_menu,0,2,create_cheats_menu(),"cheats");
    menu_add_submenu(&fp.main_menu,0,3,create_inventory_menu(),"inventory");
    menu_add_submenu(&fp.main_menu,0,4,create_file_menu(),"file");
    menu_add_submenu(&fp.main_menu,0,5,create_watches_menu(),"watches");
    menu_add_submenu(&fp.main_menu,0,6,create_trainer_menu(),"trainer");
    menu_add_submenu(&fp.main_menu,0,7,create_settings_menu(),"settings");
    */

    
    
    
    /*
    menu_set_font(fp.menu_main, font);
    menu_set_cell_width(gz.menu_main, font->char_width + font->letter_spacing);
    menu_set_cell_height(gz.menu_main, font->char_height + font->line_spacing);
    */



    /*ready*/
    fp.ready = 1;

}

// Initilizes and uses new stack instead of using graph threads stack. 
static void init_stack(void (*func)(void)) {
    static _Alignas(8) __attribute__((section(".stack"))) 
    char stack[0x2000];
    __asm__ volatile(   "la     $t0, %1;"
                        "sw     $sp, -0x04($t0);"
                        "sw     $ra, -0x08($t0);"
                        "addiu  $sp, $t0, -0x08;"
                        "jalr   %0;"
                        "nop;"
                        "lw     $ra, 0($sp);"
                        "lw     $sp, 4($sp);"
                        ::
                        "r"(func),
                        "i"(&stack[sizeof(stack)]));
}


/* fp entry point - init stack, update game, and call main function */
ENTRY void _start(void){

    init_gp();
    if(!fp.ready){
        init_stack(init);
    }
    gamestate_main();
    init_stack(fp_main);
}

#include <startup.c>
#include <vector/vector.c>
#include <list/list.c>
#include <grc.c>