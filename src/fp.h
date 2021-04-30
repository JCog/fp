#ifndef _FP_H
#define _FP_H

#include <stdint.h>
#include <vector/vector.h>
#include "pm64.h"
#include "settings.h"

struct log_entry{  
    char                   *msg;
    int                     age;
};

struct timer {
    int64_t         start;
    int64_t         end;
    _Bool           show;
    _Bool           logging;
    _Bool           prev_cutscene_state;
    uint8_t         state; /* 0 = inactive, 1 = waiting to start, 2 = running, 3 = stopped */
    uint8_t         prev_state;
    uint8_t         cutscene_target;
    uint8_t         cutscene_count;
};

typedef struct{
    _Bool                   ready;
    int64_t                 cpu_counter;
    int32_t                 cpu_counter_freq;
    struct timer            timer;
    uint16_t                cheats;
    struct menu             *main_menu;
    struct menu             *global;
    _Bool                   menu_active;
    _Bool                   coord_active;
    _Bool                   version_shown;
    struct log_entry        log[SETTINGS_LOG_MAX];
    float                   saved_x;
    float                   saved_y;
    float                   saved_z;
    float                   saved_facing_angle;
    float                   saved_movement_angle;
    int8_t                  saved_trick;
    int8_t                  ace_frame_window;
    
} fp_ctxt_t;

extern fp_ctxt_t fp;

void add_log(const char *fmt, ...);

struct menu *create_warps_menu();
struct menu *create_cheats_menu();
struct menu *create_player_menu();
struct menu *create_progression_menu();
struct menu *create_file_menu();
struct menu *create_watches_menu();
struct menu *create_practice_menu();
struct menu *create_debug_menu();
struct menu *create_settings_menu();


#endif