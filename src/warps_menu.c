#include <stdlib.h>
#include <stdio.h>
#include "gfx.h"
#include "menu.h"
#include "fp.h"

static int activate_warp_proc(struct menu_item *item, void *data){

    return 1;
}

struct menu *create_warps_menu(){
    static struct menu warps;
    menu_init(&warps,fp.main_menu.x,fp.main_menu.y);

    int x = 0;
    int y = 0;
    char group[16] = {0};



    warps.selected_item = menu_add_button(&warps,x,y++,"return",menu_return,NULL);
    menu_add_gray(&warps, x, y++, "current group: ");
    menu_add_gray(&warps, 10, y, group);

    return &warps;
}