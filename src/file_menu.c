#include <stdlib.h>
#include "menu.h"
#include "fp.h"

struct menu *create_file_menu(){
    static struct menu file;
    menu_init(&file,fp.main_menu.x,fp.main_menu.y);

    file.selected_item = menu_add_button(&file,0,0,"return",menu_return,NULL);

    return &file;
}