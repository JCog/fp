#include "menu.h"
#include "fp.h"

struct cheat_item{
    enum cheats mask;
    const char *name;
};

static struct cheat_item cheat_table[] = {
    { CHEAT_HP, "hp" },
    { CHEAT_FP, "fp" },
    { CHEAT_BP, "bp" },
    { CHEAT_COINS, "coins" },
    { CHEAT_STAR_POWER, "star power" },
    { CHEAT_STAR_POINTS, "star points" },
    { CHEAT_STAR_PIECES, "star pieces" },
    { CHEAT_PERIL, "peril" },
    { CHEAT_ENCOUNTER, "never encounter" },
};

struct menu *create_cheats_menu(){
    static struct menu cheats;
    menu_init(&cheats,0,0);
    cheats.selected_item = menu_add_button(&cheats,0,0,"return",menu_return,NULL);
    for(int i=0;i<sizeof(cheat_table)/sizeof(*cheat_table);i++){
        menu_add_switch(&cheats,0,i+1,&fp.cheats,2,(1 << cheat_table[i].mask),NULL,cheat_table[i].name);
    }
    return &cheats;
}
