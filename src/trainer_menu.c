#include <stdlib.h>
#include "menu.h"
#include "fp.h"

struct menu *create_trainer_menu(){
    static struct menu trainer;
    menu_init(&trainer,fp.main_menu.x,fp.main_menu.y);

    trainer.selected_item = menu_add_button(&trainer,0,0,"return",menu_return,NULL);

    return &trainer;
}