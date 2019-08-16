#ifndef _FP_H
#define _FP_H

#include <stdint.h>
#include <vector/vector.h>
#include "pm64.h"
#include "menu.h"

enum cheats {
    CHEAT_HP,
    CHEAT_FP,
    CHEAT_BP,
    CHEAT_COINS,
    CHEAT_STAR_POWER,
    CHEAT_STAR_POINTS,
    CHEAT_STAR_PIECES,
    CHEAT_PERIL,
    CHEAT_ENCOUNTER
};

typedef struct  {
    _Bool                   ready;    
    struct vector           watches;
    size_t                  watch_cnt;
    uint16_t                cheats;
    struct menu             main_menu;
    /*struct settings        *settings;*/
    _Bool                   menu_active;
    float                   saved_x;
    float                   saved_y;
    float                   saved_z;
    float                   saved_facing_angle;
    float                   saved_movement_angle;
    
} fp_ctxt_t;


extern fp_ctxt_t fp;

struct menu *create_warps_menu();
struct menu *create_inventory_menu();
struct menu *create_scene_menu();
struct menu *create_watches_menu();
struct menu *create_cheats_menu();
struct menu *create_file_menu();

#endif