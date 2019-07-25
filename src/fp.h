#ifndef _FP_H
#define _FP_H

typedef struct  {
    _Bool                   ready;    
    /*struct vector           watches;*/
    size_t                  watch_cnt;
    uint16_t                cheats;
    /*struct menu             main_menu;*/
    /*struct settings        *settings;*/
    _Bool                   menu_active;
    size_t test;
    
} fp_ctxt_t;

#endif