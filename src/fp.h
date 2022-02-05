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
    uint16_t saved_area;
    uint16_t saved_map;
    uint16_t saved_entrance;
    int8_t ace_frame_window;
    uint16_t ace_last_timer;
    _Bool ace_last_flag_status;
    _Bool ace_last_jump_status;
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
    char *last_imported_save_path;
    _Bool free_cam;
    _Bool lock_cam;
    enum cam_bhv cam_bhv;
    int16_t cam_dist_min;
    int16_t cam_dist_max;
    float cam_pitch;
    float cam_yaw;
    vec3f_t cam_pos;
    controller_t input_mask;
    _Bool cam_enabled_before;
} fp_ctxt_t;

extern fp_ctxt_t fp;

void fp_log(const char *fmt, ...);
_Bool fp_warp(Area area, uint16_t map, uint16_t entrance);
void fp_set_global_flag(int flag_index, _Bool value);
void fp_set_area_flag(int flag_index, _Bool value);
void fp_set_enemy_defeat_flag(int flag_index, _Bool value);
void fp_set_global_byte(int byte_index, int8_t value);
int fp_import_file(const char *path, void *data);
void fp_set_input_mask(uint16_t pad, uint8_t x, uint8_t y);
void fp_update_cam(void);

struct menu *create_warps_menu();
struct menu *create_cheats_menu();
struct menu *create_player_menu();
struct menu *create_file_menu();
struct menu *create_practice_menu();
struct menu *create_debug_menu();
struct menu *create_settings_menu();
struct menu *create_camera_menu();

#ifdef NDEBUG
#define PRINTF(...) ((void)0)
#else
#define PRINTF(...) (osSyncPrintf(__VA_ARGS__))
#endif

#define CHEAT_ACTIVE(cheat) (settings->cheats & (1 << cheat))

#endif
