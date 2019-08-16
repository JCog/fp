    
#include <stdio.h>
#include <stdlib.h>
#include <startup.h>
#include <inttypes.h>
#include "fp.h"
#include "gfx.h"
#include "input.h"
#include "commands.h"
#include "watches.h"


__attribute__((section(".data")))
fp_ctxt_t fp = { 
    .ready = 0, 
};

void fp_main(void){
    gfx_begin();
    input_update();


    pm_player.stats.boots_upgrade = 2;
    pm_player.stats.hammer_upgrade = 2;
    pm_player.stats.fp = 99;
    pm_player.stats.max_fp = 99;
    pm_player.stats.menu_max_fp = 99;
    pm_player.stats.bp = 99;
    pm_player.stats.level = 27;
    pm_player.stats.has_action_command = 1;
    pm_player.stats.coins = 999;
    pm_player.stats.star_pieces = 160;

    /* draw input display */
    {
        controller_t pad    = pm_status.raw;
        int16_t      pad_x  = pm_status.control_x;
        int16_t      pad_y  = pm_status.control_y;

        gfx_printf(16,240-16,"%4i %4i",pad_x,pad_y);
        gfx_printf_color(106,240-16,GPACK_RGBA8888(0x00,0x00,0xFF,0xFF),"%s", pad.a?"A":" ");
        gfx_printf_color(116,240-16,GPACK_RGBA8888(0x00,0xFF,0x00,0xFF),"%s", pad.b?"B":" ");
        gfx_printf_color(126,240-16,GPACK_RGBA8888(0xFF,0x00,0x00,0xFF),"%s", pad.s?"S":" ");
        gfx_printf_color(136,240-16,GPACK_RGBA8888(0xFF,0xFF,0xFF,0xFF),"%s%s%s", pad.z?"Z":" ",pad.l?"L":" ",pad.r?"R":" ");
        gfx_printf_color(166,240-16,GPACK_RGBA8888(0xFF,0xFF,0x00,0xFF),"%s%s%s%s", pad.cl?"<":" ",pad.cu?"^":" ", pad.cr?">":" ",pad.cd?"v":" ");
        gfx_printf_color(206,240-16,GPACK_RGBA8888(0xFF,0xFF,0xFF,0xFF),"%s%s%s%s", pad.dl?"<":" ",pad.du?"^":" ", pad.dr?">":" ",pad.dd?"v":" ");
    }

     /* draw floating watches */
    {
        for(int i=0;i<fp.watch_cnt;i++){
            watch_t *watch = vector_at(&fp.watches,i);
            if(watch->floating){
                watch_printf(watch);
            }
        }
    }

    /* handle menu */
    {
        static _Bool skip_menu = 0;
        uint16_t button_pressed = pm_status.pressed.buttons;

        if(input_bind_pressed(0)){
            skip_menu = 1;
            fp.menu_active = !fp.menu_active;
            if(fp.menu_active){
                reserve_buttons(BUTTON_L | BUTTON_D_DOWN | BUTTON_D_LEFT | BUTTON_D_RIGHT | BUTTON_D_UP);
            }else{
                free_buttons(BUTTON_L | BUTTON_D_DOWN | BUTTON_D_LEFT | BUTTON_D_RIGHT | BUTTON_D_UP);
            }
        }

        if(fp.menu_active && !skip_menu){
            struct menu *fp_menu = &fp.main_menu;
            enum menu_nav navdir = MENU_NAV_NONE;
            enum menu_callback callback = MENU_CALLBACK_NONE;
            if(button_pressed & BUTTON_D_DOWN){
                navdir=MENU_NAV_DOWN;
            }else if(button_pressed & BUTTON_D_UP){
                navdir=MENU_NAV_UP;
            }else if(button_pressed & BUTTON_D_LEFT){
                navdir=MENU_NAV_LEFT;
            }else if(button_pressed & BUTTON_D_RIGHT){
                navdir=MENU_NAV_RIGHT;
            }else if(button_pressed & BUTTON_L){
                callback = MENU_CALLBACK_ACTIVATE;
            }

            menu_navigate(fp_menu,navdir);
            menu_callback(fp_menu,callback);
            menu_draw(fp_menu);
        }

        skip_menu = 0;
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

    gfx_finish();    /*output gfx display lists*/
}

void gamestate_main(){
    pm_GameUpdate(); /* displaced function call - advances 1 game frame*/

}

void init(){
    clear_bss();
    do_global_ctors();
    gfx_init();

    vector_init(&fp.watches, sizeof(watch_t));
    vector_reserve(&fp.watches,WATCHES_MAX);
    fp.watch_cnt = 0;

    /*hard coded button bindings*/
    fp_commands[0].bind = make_bind(2, BUTTON_R, BUTTON_D_UP);
    fp_commands[1].bind = make_bind(2, BUTTON_R, BUTTON_D_RIGHT);
    fp_commands[2].bind = make_bind(1, BUTTON_D_UP);
    fp_commands[3].bind = make_bind(1, BUTTON_D_DOWN);
    fp_commands[4].bind = make_bind(1, BUTTON_D_LEFT);
    fp_commands[5].bind = make_bind(1, BUTTON_D_RIGHT);

    /*init menu and default it to inactive*/
    fp.menu_active = 0;
    menu_init(&fp.main_menu, 15, 55);

    /*add menus*/
    fp.main_menu.selected_item = menu_add_button(&fp.main_menu,0,0,"return",menu_return,NULL);
    menu_add_submenu(&fp.main_menu,0,1,create_watches_menu(),"watches");

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