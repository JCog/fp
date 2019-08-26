#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <startup.h>
#include <mips.h>
#include <n64.h>
#include "fp.h"
#include "gfx.h"
#include "input.h"
#include "menu.h"
#include "commands.h"
#include "resource.h"
#include "settings.h"

__attribute__((section(".data")))
fp_ctxt_t fp = { 
    .ready = 0, 
};

void fp_main(void){

    gfx_mode_init();
    input_update();

    struct gfx_font *font = menu_get_font(fp.main_menu, 1);
    uint8_t alpha = menu_get_alpha_i(fp.main_menu, 1);
    int cw = menu_get_cell_width(fp.main_menu, 1);
    int ch = menu_get_cell_height(fp.main_menu, 1);

    /*draw input display*/
    {   
        uint16_t d_pad = pm_status.raw.buttons;
        int8_t   d_x   = pm_status.control_x;
        int8_t   d_y   = pm_status.control_y;

        struct gfx_texture *texture = resource_get(RES_ICON_BUTTONS);
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xC0, 0xC0, 0xC0, alpha));
        gfx_printf(font, settings->input_display_x, settings->input_display_y,
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
            settings->input_display_x + cw * 10 + x, settings->input_display_y + y,
            1.f, 1.f,
          };
          gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(input_button_color[b],
                                                     alpha));
          gfx_sprite_draw(&sprite);
      }
    }
    
    /* handle menu input */
    {
        if (fp.menu_active) {
            if (input_bind_pressed_raw(COMMAND_MENU)){
                hide_menu();
            }
            else if (input_bind_pressed(COMMAND_RETURN)){
                menu_return(fp.main_menu);
            }
            else {
                uint16_t pad_pressed = input_pressed();
                if (pad_pressed & BUTTON_D_UP)
                    menu_navigate(fp.main_menu, MENU_NAVIGATE_UP);
                if (pad_pressed & BUTTON_D_DOWN)
                    menu_navigate(fp.main_menu, MENU_NAVIGATE_DOWN);
                if (pad_pressed & BUTTON_D_LEFT)
                    menu_navigate(fp.main_menu, MENU_NAVIGATE_LEFT);
                if (pad_pressed & BUTTON_D_RIGHT)
                    menu_navigate(fp.main_menu, MENU_NAVIGATE_RIGHT);
                if (pad_pressed & BUTTON_L)
                    menu_activate(fp.main_menu);
          }
        }
        else if (input_bind_pressed_raw(COMMAND_MENU))
            show_menu();
    }
    
    /* activate cheats */
    {
        if(settings->cheats & (1 << CHEAT_HP)){
            pm_player.stats.hp = 99;
            pm_player.stats.max_hp = 99;
            pm_player.stats.menu_max_hp = 99;
            pm_hud.hp_value = 99;
        }
        if(settings->cheats & (1 << CHEAT_FP)){
            pm_player.stats.fp = 99;
            pm_player.stats.max_fp = 99;
            pm_player.stats.menu_max_fp = 99;
            pm_hud.fp_value = 99;
        }
        if(settings->cheats & (1 << CHEAT_BP)){
            pm_player.stats.bp = 99;
        }
        if(settings->cheats & (1 << CHEAT_COINS)){
            pm_player.stats.coins = 99;
            pm_hud.coin_value = 99;
        }
        if(settings->cheats & (1 << CHEAT_STAR_POWER)){
            
        }
        if(settings->cheats & (1 << CHEAT_STAR_POINTS)){
            pm_player.stats.star_points = 99;
        }
        if(settings->cheats & (1 << CHEAT_STAR_PIECES)){
            pm_player.stats.star_pieces = 160;
        }
        if(settings->cheats & (1 << CHEAT_PERIL)){
            pm_player.stats.hp = 1;
            pm_hud.hp_value = 1;
        }
        if(settings->cheats & (1 << CHEAT_ENCOUNTER)){
            pm_overworld.partner_ability = 9;
        }
        if(settings->cheats & (1 << CHEAT_BREAK)){
            int32_t third_byte_mask = 0xFFFF00FF;
            int32_t check_mask = 0x0000FF00;

            if((pm_player.animation & check_mask) == 0x2000){
                pm_player.animation &= third_byte_mask;
            }
        }
    }

    /* handle command bindings */
    {
        for (int i = 0; i < COMMAND_MAX; ++i) {
            _Bool active = 0;
            switch (fp_commands[i].command_type) {
                case COMMAND_HOLD:       active = input_bind_held(i);        break;
                case COMMAND_PRESS:      active = input_bind_pressed(i);     break;
                case COMMAND_PRESS_ONCE: active = input_bind_pressed_raw(i); break;
            }
            if (fp_commands[i].proc && active){
                fp_commands[i].proc();
            }
        }
    }

    /* draw and animate menus */
    {
        while (fp.menu_active && menu_think(fp.main_menu))
          ;
        while (menu_think(fp.global))
          ;

        if (fp.menu_active){
          menu_draw(fp.main_menu);
        }
        menu_draw(fp.global);
    }

    /* draw log */
    {
        for (int i = SETTINGS_LOG_MAX - 1; i >= 0; --i) {
            const int fade_begin = 20;
            const int fade_duration = 20;
            struct log_entry *ent = &fp.log[i];
            uint8_t msg_alpha;
            if (!ent->msg)
                continue;
            ++ent->age;
            if (ent->age > (fade_begin + fade_duration)) {
                free(ent->msg);
                ent->msg = NULL;
                continue;
            }
            else if (!settings->bits.log){
                continue;
            }
            else if (ent->age > fade_begin){
                msg_alpha = 0xFF - (ent->age - fade_begin) * 0xFF / fade_duration;
            }
            else{
              msg_alpha = 0xFF;
            }
            msg_alpha = msg_alpha * alpha / 0xFF;
            int msg_x = settings->log_x - cw * strlen(ent->msg);
            int msg_y = settings->log_y - ch * i;
            gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(0xC0C0C0, msg_alpha));
            gfx_printf(font, msg_x, msg_y, "%s", ent->msg);
        }
    }   


    gfx_flush();
}

static void main_return_proc(struct menu_item *item, void *data){
    hide_menu();
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

    /*init fp variables*/
    fp.menu_active = 0;

    /*load default settings*/
    settings_load_default();

    /*init menus*/
    static struct menu main_menu;
    static struct menu global;
    static struct menu watches;
    fp.main_menu = &main_menu;
    fp.global = &global;

    menu_init(&main_menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&watches, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&global, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    /*populate top level menus*/
    main_menu.selector = menu_add_button(fp.main_menu, 0, 0, "return", main_return_proc, NULL);
    menu_add_submenu(fp.main_menu,0,1,create_warps_menu(),"warps");
    menu_add_submenu(fp.main_menu,0,2,create_cheats_menu(),"cheats");
    menu_add_submenu(fp.main_menu,0,3,create_inventory_menu(),"inventory");
    menu_add_submenu(fp.main_menu,0,4,create_file_menu(),"file");
    menu_add_submenu(fp.main_menu,0,5,create_watches_menu(),"watches");
    menu_add_submenu(fp.main_menu,0,6,create_trainer_menu(),"trainer");
    menu_add_submenu(fp.main_menu,0,7,create_settings_menu(),"settings");

    /* configure menu related commands */
    input_bind_set_override(COMMAND_MENU, 1);
    input_bind_set_override(COMMAND_RETURN, 1);

    /*get menu appearance*/
    apply_menu_settings();

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