#include <stdlib.h>
#include "menu.h"
#include "fp.h"

struct menu *create_settings_menu(){
    static struct menu settings;
    menu_init(&settings,fp.main_menu.x,fp.main_menu.y);

    settings.selected_item = menu_add_button(&settings,0,0,"return",menu_return,NULL);

    return &settings;
}