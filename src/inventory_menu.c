#include <stdlib.h>
#include "menu.h"
#include "fp.h"

struct menu *create_inventory_menu(){
    static struct menu inventory;
    menu_init(&inventory,fp.main_menu.x,fp.main_menu.y);

    inventory.selected_item = menu_add_button(&inventory,0,0,"return",menu_return,NULL);

    return &inventory;
}