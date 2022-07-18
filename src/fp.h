#ifndef FP_H
#define FP_H
#include <stdint.h>
#include <vector/vector.h>
#include "pm64.h"
#include "settings.h"

enum cam_mode {
    CAMMODE_CAMERA,
    CAMMODE_VIEW,
};

enum cam_bhv {
    CAMBHV_MANUAL,
    CAMBHV_BIRDSEYE,
    CAMBHV_RADIAL,
};

struct log_entry {
    char *msg;
    s32 age;
};

struct timer {
    s64 start;
    s64 end;
    u32 lag_start;
    u32 lag_end;
    u16 frame_start;
    u16 frame_end;
    _Bool prev_cutscene_state;
    u8 mode;  /* 0 = auto, 1 = manual; */
    u8 state; /* 0 = inactive, 1 = waiting to start, 2 = running, 3 = stopped */
    u8 cutscene_target;
    u8 cutscene_count;
    _Bool moving;
};

typedef struct {
    _Bool ready;
    struct menu *main_menu;
    struct menu *global;
    struct menu *menu_mem;
    struct menu *menu_watches;
    struct menu_item *menu_watchlist;
    u8 profile;
    _Bool settings_loaded;
    _Bool version_shown;
    s64 cpu_counter;
    s32 cpu_counter_freq;
    struct timer timer;
    s64 timer_count;
    s32 lag_frames;
    _Bool menu_active;
    struct log_entry log[SETTINGS_LOG_MAX];
    f32 saved_x;
    f32 saved_y;
    f32 saved_z;
    f32 saved_facing_angle;
    f32 saved_movement_angle;
    u16 saved_area;
    u16 saved_map;
    u16 saved_entrance;
    s8 ace_frame_window;
    u16 ace_last_timer;
    _Bool ace_last_flag_status;
    _Bool ace_last_jump_status;
    _Bool turbo;
    _Bool bowser_blocks_enabled;
    s8 bowser_block;
    _Bool lzs_trainer_enabled;
    s8 prev_pressed_y;
    u8 prev_prev_action_state;
    _Bool lz_stored;
    _Bool player_landed;
    u16 frames_since_land;
    _Bool warp;
    u8 warp_delay;
    u16 current_lzs_jumps;
    u16 record_lzs_jumps;
    u16 frames_since_battle;
    u8 clippy_status;
    _Bool clippy_trainer_enabled;
    char *last_imported_save_path;
    _Bool free_cam;
    _Bool lock_cam;
    enum cam_bhv cam_bhv;
    s16 cam_dist_min;
    s16 cam_dist_max;
    f32 cam_pitch;
    f32 cam_yaw;
    vec3f_t cam_pos;
    controller_t input_mask;
    _Bool cam_enabled_before;
    _Bool action_command_trainer_enabled;
    u16 last_a_press;
    u16 last_valid_frame;
} fp_ctxt_t;

extern fp_ctxt_t fp;

void fp_log(const char *fmt, ...);
_Bool fp_warp(u16 area, u16 map, u16 entrance);
void fp_set_global_flag(s32 flag_index, _Bool value);
void fp_set_area_flag(s32 flag_index, _Bool value);
void fp_set_enemy_defeat_flag(s32 flag_index, _Bool value);
void fp_set_global_byte(s32 byte_index, s8 value);
s32 fp_import_file(const char *path, void *data);
void fp_set_input_mask(u16 pad, u8 x, u8 y);
void fp_update_cam(void);

struct menu *create_warps_menu();
struct menu *create_cheats_menu();
struct menu *create_player_menu();
struct menu *create_file_menu();
struct menu *create_practice_menu();
struct menu *create_debug_menu();
struct menu *create_settings_menu();
struct menu *create_camera_menu();

#define CHEAT_ACTIVE(cheat) (settings->cheats & (1 << cheat))

#endif
