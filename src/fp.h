#ifndef FP_H
#define FP_H
#include <stdint.h>
#include <vector/vector.h>
#include "pm64.h"
#include "settings.h"

struct log_entry {
    char *msg;
    int age;
};

struct timer {
    int64_t start;
    int64_t end;
    uint32_t lag_start;
    uint32_t lag_end;
    uint16_t frame_start;
    uint16_t frame_end;
    _Bool prev_cutscene_state;
    uint8_t mode;  /* 0 = auto, 1 = manual; */
    uint8_t state; /* 0 = inactive, 1 = waiting to start, 2 = running, 3 = stopped */
    uint8_t cutscene_target;
    uint8_t cutscene_count;
    _Bool moving;
};

typedef struct {
    _Bool ready;
    struct menu *main_menu;
    struct menu *global;
    struct menu *menu_mem;
    struct menu *menu_watches;
    struct menu_item *menu_watchlist;
    uint8_t profile;
    _Bool settings_loaded;
    _Bool version_shown;
    int64_t cpu_counter;
    int32_t cpu_counter_freq;
    struct timer timer;
    s64 timer_count;
    s32 lag_frames;
    _Bool menu_active;
    struct log_entry log[SETTINGS_LOG_MAX];
    float saved_x;
    float saved_y;
    float saved_z;
    float saved_facing_angle;
    float saved_movement_angle;
    int8_t saved_trick;
    uint16_t saved_group;
    uint16_t saved_room;
    uint16_t saved_entrance;
    int8_t ace_frame_window;
    _Bool turbo;
    _Bool bowser_blocks_enabled;
    int8_t bowser_block;
    _Bool lzs_trainer_enabled;
    int8_t prev_pressed_y;
    uint8_t prev_prev_action_state;
    _Bool lz_stored;
    _Bool player_landed;
    uint16_t frames_since_land;
    _Bool warp;
    uint8_t warp_delay;
    uint16_t current_lzs_jumps;
    uint16_t record_lzs_jumps;
    uint16_t frames_since_battle;
    uint8_t clippy_status;
    _Bool clippy_trainer_enabled;
} fp_ctxt_t;

extern fp_ctxt_t fp;

void fp_log(const char *fmt, ...);
_Bool fp_warp(Group group, uint16_t room, uint16_t entrance);
void fp_set_global_flag(int flag_index, _Bool value);
void fp_set_area_flag(int flag_index, _Bool value);
void fp_set_enemy_defeat_flag(int flag_index, _Bool value);
void fp_set_global_byte(int byte_index, int8_t value);

struct menu *create_warps_menu();
struct menu *create_cheats_menu();
struct menu *create_player_menu();
struct menu *create_file_menu();
struct menu *create_practice_menu();
struct menu *create_debug_menu();
struct menu *create_settings_menu();

#ifdef NDEBUG
#define PRINTF(...) ((void)0)
#else
#define PRINTF(...) (osSyncPrintf(__VA_ARGS__))
#endif

#define CHEAT_ACTIVE(cheat) (settings->cheats & (1 << cheat))

#endif
