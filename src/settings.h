#ifndef SETTINGS_H
#define SETTINGS_H
#include "commands.h"
#include "common.h"

#define SETTINGS_SAVE_FILE_SIZE   0x1380
#define SETTINGS_PROFILE_MAX      4
#define SETTINGS_VERSION          0x0004

#define SETTINGS_WATCHES_MAX      18
#define SETTINGS_TELEPORT_MAX     9
#define SETTINGS_MEMFILE_MAX      10
#define SETTINGS_BIND_MAX         COMMAND_MAX
#define SETTINGS_LOG_MAX          4

#define SETTINGS_BREAK_NORMAL     0
#define SETTINGS_BREAK_AGGRESSIVE 1

enum cheats {
    CHEAT_HP,
    CHEAT_FP,
    CHEAT_COINS,
    CHEAT_STAR_POWER,
    CHEAT_STAR_PIECES,
    CHEAT_PERIL,
    CHEAT_AUTO_MASH,
    CHEAT_AUTO_ACTION_CMD,
    CHEAT_BRIGHTEN_ROOM,
    CHEAT_MAX
};

struct watch_info {
    u8 type         : 4;
    u8 anchored     : 1;
    u8 position_set : 1;
};

struct settings_bits {
    u32 font_resource   : 4;
    u32 drop_shadow     : 1;
    u32 input_display   : 1;
    u32 log             : 1;
    u32 timer_show      : 1;
    u32 timer_logging   : 1;
    u32 battle_debug    : 3;
    u32 quizmo_debug    : 1;
    u32 watches_visible : 1;
};

struct settings_data {
    /* order elements by size for space-efficient packing */
    u32 watch_address[SETTINGS_WATCHES_MAX];
    u32 cheats;
    struct settings_bits bits;
    s16 menu_x;
    s16 menu_y;
    s16 input_display_x;
    s16 input_display_y;
    s16 log_x;
    s16 log_y;
    s16 timer_x;
    s16 timer_y;
    s16 watch_x[SETTINGS_WATCHES_MAX];
    s16 watch_y[SETTINGS_WATCHES_MAX];
    u16 binds[SETTINGS_BIND_MAX];
    s8 control_stick_range;
    u8 control_stick;
    struct watch_info watch_info[SETTINGS_WATCHES_MAX];
    u8 n_watches;
};

struct settings_header {
    u16 version;
    u16 data_size;
    u16 data_checksum;
};

struct settings {
    struct settings_header header;
    struct settings_data data;
};

void settings_load_default();
void apply_menu_settings();
void settings_save(s32 profile);
_Bool settings_load(s32 profile);

extern struct settings_data *settings;

#endif
