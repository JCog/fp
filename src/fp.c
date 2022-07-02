#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <startup.h>
#include <n64.h>
#include "commands.h"
#include "fp.h"
#include "geometry.h"
#include "gu.h"
#include "io.h"
#include "resource.h"
#include "watchlist.h"
#include "crash_screen.h"
#include "sys.h"
#include "util.h"

__attribute__((section(".data"))) fp_ctxt_t fp = {
    .ready = 0,
};

// Initializes and uses new stack instead of using games main thread stack.
static void init_stack(void (*func)(void)) {
    static _Alignas(8) __attribute__((section(".stack"))) char stack[0x2000];
    __asm__ volatile("la     $t0, %1;"
                     "sw     $sp, -0x04($t0);"
                     "sw     $ra, -0x08($t0);"
                     "addiu  $sp, $t0, -0x08;"
                     "jalr   %0;"
                     "nop;"
                     "lw     $ra, 0($sp);"
                     "lw     $sp, 4($sp);" ::"r"(func),
                     "i"(&stack[sizeof(stack)]));
}

static void main_return_proc(struct menu_item *item, void *data) {
    hide_menu();
}

void fp_init() {
    clear_bss();
    do_global_ctors();

    gfx_start();
    gfx_mode_configure(GFX_MODE_FILTER, G_TF_POINT);
    gfx_mode_configure(GFX_MODE_COMBINE, G_CC_MODE(G_CC_MODULATEIA_PRIM, G_CC_MODULATEIA_PRIM));

    icons_init();

    fp.profile = 0;
    fp.settings_loaded = 0;
    fp.version_shown = 0;
    fp.cpu_counter = 0;
    fp.cpu_counter_freq = 0;
    fp.timer.start = 0;
    fp.timer.end = 0;
    fp.timer.lag_start = 0;
    fp.timer.lag_end = 0;
    fp.timer.frame_start = 0;
    fp.timer.frame_end = 0;
    fp.timer.prev_cutscene_state = 0;
    fp.timer.mode = 0;
    fp.timer.state = 0;
    fp.timer.cutscene_target = 1;
    fp.timer.cutscene_count = 0;
    fp.timer.moving = 0;
    fp.menu_active = 0;

    for (s32 i = 0; i < SETTINGS_LOG_MAX; i++) {
        fp.log[i].msg = NULL;
    }

    fp.saved_x = 0;
    fp.saved_y = 0;
    fp.saved_z = 0;
    fp.saved_facing_angle = 0;
    fp.saved_area = 0x1c;
    fp.saved_map = 0;
    fp.saved_entrance = 0;
    fp.turbo = 0;
    fp.ace_last_timer = 0;
    fp.ace_last_flag_status = 0;
    fp.ace_last_jump_status = 0;
    fp.bowser_blocks_enabled = 0;
    fp.bowser_block = 0;
    fp.prev_prev_action_state = 0;
    fp.lz_stored = 0;
    fp.record_lzs_jumps = 0;
    fp.current_lzs_jumps = 0;
    fp.player_landed = 0;
    fp.frames_since_land = 0;
    fp.warp = 0;
    fp.warp_delay = 0;
    fp.frames_since_battle = 0;
    fp.clippy_status = 0;
    fp.last_imported_save_path = NULL;
    fp.free_cam = 0;
    fp.lock_cam = 0;
    fp.cam_bhv = CAMBHV_MANUAL;
    fp.cam_dist_min = 100;
    fp.cam_dist_max = 400;
    fp.cam_yaw = 0;
    fp.cam_pitch = 0;
    fp.cam_pos.x = 0;
    fp.cam_pos.y = 0;
    fp.cam_pos.z = 0;
    fp.cam_enabled_before = 0;
    fp.action_command_trainer_enabled = 0;

    io_init();

    settings_load_default();

    // init menus
    static struct menu main_menu;
    static struct menu watches;
    static struct menu global;

    menu_init(&main_menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&watches, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu_init(&global, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    fp.main_menu = &main_menu;
    fp.global = &global;
    fp.menu_watches = &watches;

    // populate top level menus
    s32 menu_index = 0;
    main_menu.selector = menu_add_button(fp.main_menu, 0, menu_index++, "return", main_return_proc, NULL);
    menu_add_submenu(fp.main_menu, 0, menu_index++, create_warps_menu(), "warps");
    menu_add_submenu(fp.main_menu, 0, menu_index++, create_cheats_menu(), "cheats");
    menu_add_submenu(fp.main_menu, 0, menu_index++, create_player_menu(), "player");
    menu_add_submenu(fp.main_menu, 0, menu_index++, create_file_menu(), "file");
    menu_add_submenu(fp.main_menu, 0, menu_index++, create_practice_menu(), "practice");
    menu_add_submenu(fp.main_menu, 0, menu_index++, create_camera_menu(), "camera");
    menu_add_submenu(fp.main_menu, 0, menu_index++, &watches, "watches");
    menu_add_submenu(fp.main_menu, 0, menu_index++, create_debug_menu(), "debug");
    menu_add_submenu(fp.main_menu, 0, menu_index++, create_settings_menu(), "settings");

    // populate watches menu
    watches.selector = menu_add_submenu(&watches, 0, 0, NULL, "return");
    fp.menu_watchlist = watchlist_create(&watches, &global, 0, 1);

    // configure menu related commands
    input_bind_set_override(COMMAND_MENU, 1);
    input_bind_set_override(COMMAND_RETURN, 1);

    // get menu appearance
    apply_menu_settings();

    // skip intro on boot
    pm_status.skip_intro = 1;

    // calculate frame window for OoT ACE
    *(u16 *)0x807D0000 = 0;
    s32 memory_value = 0;
    s32 *pointer = (s32 *)0x807BFFF8;

    while (memory_value == 0) {
        pointer--;
        memory_value = *pointer;
    }

    s32 frame_window = (s32)pointer;
    frame_window -= 0x80400000;

    if (frame_window % 0x4000 == 0) {
        frame_window /= 0x40000;
        frame_window -= 0x10;
    } else {
        frame_window /= 0x40000;
        frame_window -= 0xf;
    }

    frame_window *= -1;
    fp.ace_frame_window = frame_window;

#if PM64_VERSION == JP
    crash_screen_init();
#endif

    fp.ready = 1;
}

void fp_update_menu(void) {
    if (input_bind_pressed_raw(COMMAND_MENU)) {
        hide_menu();
    } else if (input_bind_pressed(COMMAND_RETURN)) {
        menu_return(fp.main_menu);
    } else {
        u16 pad_pressed = input_pressed();
        if (pad_pressed & BUTTON_D_UP) {
            menu_navigate(fp.main_menu, MENU_NAVIGATE_UP);
        }
        if (pad_pressed & BUTTON_D_DOWN) {
            menu_navigate(fp.main_menu, MENU_NAVIGATE_DOWN);
        }
        if (pad_pressed & BUTTON_D_LEFT) {
            menu_navigate(fp.main_menu, MENU_NAVIGATE_LEFT);
        }
        if (pad_pressed & BUTTON_D_RIGHT) {
            menu_navigate(fp.main_menu, MENU_NAVIGATE_RIGHT);
        }
        if (pad_pressed & BUTTON_L) {
            menu_activate(fp.main_menu);
        }
    }
}

static void fp_update_cpu_counter(void) {
    static u32 count = 0;
    u32 new_count;
    __asm__("mfc0    %0, $9;" : "=r"(new_count));
    fp.cpu_counter_freq = OS_CPU_COUNTER;
    fp.cpu_counter += new_count - count;
    count = new_count;
}

void fp_emergency_settings_reset(u16 pad_pressed) {
    if (pad_pressed) {
        static const u16 input_list[] = {
            BUTTON_D_UP,    BUTTON_D_UP,   BUTTON_D_DOWN,  BUTTON_D_DOWN, BUTTON_D_LEFT,
            BUTTON_D_RIGHT, BUTTON_D_LEFT, BUTTON_D_RIGHT, BUTTON_B,      BUTTON_A,
        };
        static s32 input_pos = 0;
        size_t input_list_length = sizeof(input_list) / sizeof(*input_list);
        if (pad_pressed == input_list[input_pos]) {
            ++input_pos;
            if (input_pos == input_list_length) {
                input_pos = 0;
                settings_load_default();
                apply_menu_settings();
                fp_log("default settings restored");
            }
        } else {
            input_pos = 0;
        }
    }
}

#define STRINGIFY(S)  STRINGIFY_(S)
#define STRINGIFY_(S) #S
void fp_draw_version(struct gfx_font *font, s32 cell_width, s32 cell_height, u8 menu_alpha) {
    if (fp.fp_icon == NULL) {
        fp.fp_icon = icons_create_item(ITEM_FP_PLUS_A, 31, SCREEN_HEIGHT - 49, 255, 1.0f, 0);
    }
    icons_draw(fp.fp_icon);
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0, 0, 0xFF));
    gfx_printf(font, 16, SCREEN_HEIGHT - 35 + cell_height * 1, STRINGIFY(FP_VERSION));
    gfx_printf(font, SCREEN_WIDTH - cell_width * 21, SCREEN_HEIGHT - 35 + cell_height * 1, STRINGIFY(URL));

    if (pm_status.load_menu_state == 5) {
        fp.version_shown = 1;
        icons_delete(fp.fp_icon);
    }
}

void fp_draw_input_display(struct gfx_font *font, s32 cell_width, s32 cell_height, u8 menu_alpha) {
    u16 d_pad = pm_status.raw.buttons;
    s8 d_x = pm_status.control_x;
    s8 d_y = pm_status.control_y;

    struct gfx_texture *texture = resource_get(RES_ICON_BUTTONS);
    struct gfx_texture *control_stick = resource_get(RES_TEXTURE_CONTROL_STICK);
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xC0, 0xC0, 0xC0, menu_alpha));
    if (settings->control_stick == 1 || settings->control_stick == 2) {
        s32 image_range = control_stick->tile_width / 2;
        s32 image_dx, image_dy;
        if (d_x > settings->control_stick_range) {
            image_dx = image_range;
        } else if (d_x < -settings->control_stick_range) {
            image_dx = -image_range;
        } else {
            image_dx = d_x * image_range / settings->control_stick_range;
        }
        if (d_y > settings->control_stick_range) {
            image_dy = -image_range;
        } else if (d_y < -settings->control_stick_range) {
            image_dy = image_range;
        } else {
            image_dy = -d_y * image_range / settings->control_stick_range;
        }
        struct gfx_sprite in_background = {
            control_stick, 0,   settings->input_display_x, settings->input_display_y - control_stick->tile_height,
            1.f,           1.f,
        };
        struct gfx_sprite in_stick = {
            control_stick,
            1,
            settings->input_display_x + image_dx,
            settings->input_display_y - control_stick->tile_height + image_dy,
            1.f,
            1.f,
        };
        gfx_sprite_draw(&in_background);
        gfx_sprite_draw(&in_stick);
    }
    if (settings->control_stick == 0) {
        gfx_printf(font, settings->input_display_x, settings->input_display_y, "%4i %4i", d_x, d_y);
    } else if (settings->control_stick == 2) {
        gfx_printf(font, settings->input_display_x + control_stick->tile_width + cell_width,
                   settings->input_display_y - cell_height * 2, "%4i %4i", d_x, d_y);
    }

    // clang-format off
    static const s32 buttons[] = {
        15, 14, 12, 3, 2, 1, 0, 13, 5, 4, 11, 10, 9, 8,
    };
    // clang-format on

    for (s32 i = 0; i < sizeof(buttons) / sizeof(*buttons); ++i) {
        s32 b = buttons[i];
        if (!(d_pad & (1 << b))) {
            continue;
        }
        s32 x = (cell_width - texture->tile_width) / 2 + i * 10;
        s32 y = -(gfx_font_xheight(font) + texture->tile_height + 1) / 2;
        s32 button_dx;
        if (settings->control_stick == 0) {
            button_dx = cell_width * 10;
        } else {
            button_dx = control_stick->tile_width + cell_width;
        }
        struct gfx_sprite sprite = {
            texture, b, settings->input_display_x + button_dx + x, settings->input_display_y + y, 1.f, 1.f,
        };
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(input_button_color[b], menu_alpha));
        gfx_sprite_draw(&sprite);
    }
}

void fp_update_timer(s64 *timer_count, s32 *lag_frames) {
    _Bool in_cutscene = pm_player.flags & 0x00002000;

    switch (fp.timer.state) {
        case 1:
            if (fp.timer.mode == 1 || (fp.timer.prev_cutscene_state && !in_cutscene)) {
                fp.timer.state = 2;
                fp.timer.start = fp.cpu_counter;
                fp.timer.lag_start = pm_ViFrames;
                fp.timer.frame_start = pm_status.frame_counter;
                if (settings->bits.timer_logging) {
                    fp_log("timer started");
                }
            }
            break;
        case 2:
            if (fp.timer.mode == 0 && !fp.timer.prev_cutscene_state && in_cutscene) {
                fp.timer.cutscene_count++;
                if (settings->bits.timer_logging && fp.timer.cutscene_count != fp.timer.cutscene_target) {
                    fp_log("cutscene started");
                }
            }
            if (fp.timer.cutscene_count == fp.timer.cutscene_target) {
                fp.timer.state = 3;
                fp.timer.end = fp.cpu_counter;
                fp.timer.lag_end = pm_ViFrames;
                fp.timer.frame_end = pm_status.frame_counter;
                fp_log("timer stopped");
            }
            *timer_count = fp.cpu_counter - fp.timer.start;
            *lag_frames = (pm_ViFrames - fp.timer.lag_start) / 2 - (pm_status.frame_counter - fp.timer.frame_start);
            break;
        case 3:
            *timer_count = fp.timer.end - fp.timer.start;
            *lag_frames = (fp.timer.lag_end - fp.timer.lag_start) / 2 - (fp.timer.frame_end - fp.timer.frame_start);
            break;
    }
}

void fp_draw_timer(s64 timer_count, s32 lag_frames, struct gfx_font *font, s32 cell_width, s32 cell_height,
                   u8 menu_alpha) {
    s32 hundredths = timer_count * 100 / fp.cpu_counter_freq;
    s32 seconds = hundredths / 100;
    s32 minutes = seconds / 60;
    s32 hours = minutes / 60;

    hundredths %= 100;
    seconds %= 60;
    minutes %= 60;

    s32 x = settings->timer_x;
    s32 y = settings->timer_y;
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xC0, 0xC0, 0xC0, menu_alpha));

    if (hours > 0) {
        gfx_printf(font, x, y, "%d:%02d:%02d.%02d", hours, minutes, seconds, hundredths);
    } else if (minutes > 0) {
        gfx_printf(font, x, y, "%d:%02d.%02d", minutes, seconds, hundredths);
    } else {
        gfx_printf(font, x, y, "%d.%02d", seconds, hundredths);
    }

    gfx_printf(font, x, y + cell_height, "%d", lag_frames);
}

// this whole thing should be redone once battles are better understood - freezing rng isn't very reliable
void fp_bowser_block_trainer(void) {
    if (pm_status.is_battle && pm_status.area_id == 0x4 && (pm_status.map_id == 0x7 || pm_status.map_id == 0x13) &&
        STORY_PROGRESS != STORY_INTRO && !(pm_status.peach_flags & (1 << 0))) {

        actor_t *bowser = pm_battle_status.enemy_actors[0];

        if (bowser != NULL) {
            s32 *turn = &bowser->var_table[0];
            s32 *turns_since_wave = &bowser->var_table[2];
            s32 *turns_since_beam = &bowser->var_table[3];
            s32 *turns_since_claw = &bowser->var_table[4];
            s32 *turns_since_stomp = &bowser->var_table[5];
            s32 *turns_since_heal = &bowser->var_table[6];
            *turns_since_heal = 0;
            *turns_since_beam = 0;
            switch (fp.bowser_block) {
                case 0: // fire
                    *turn = 3;
                    *turns_since_claw = 0;
                    *turns_since_stomp = 0;
                    *turns_since_wave = 0;
                    break;
                case 1: // butt stomp
                    *turn = 3;
                    *turns_since_claw = 0;
                    *turns_since_stomp = 1;
                    *turns_since_wave = 0;
                    pm_RandSeed = 0x03D49DFF;
                    break;
                case 2: // claw
                    *turn = 3;
                    *turns_since_stomp = 0;
                    *turns_since_claw = 1;
                    *turns_since_wave = 0;
                    pm_RandSeed = 0x9CB89EDA;
                    break;
                case 3: // wave
                    *turn = 4;
                    *turns_since_wave = 6;
                    pm_RandSeed = 0x77090261;
                    break;
                case 4: // lightning, still gives wave for hallway bowser
                    *turn = 4;
                    *turns_since_wave = 6;
                    pm_RandSeed = 0x72A5DCE5;
                    break;
            }
        }

        if (pm_battle_status.partner_actor != NULL) {
            // if partner is KO'd by wave, never let it last more than one turn so you can keep practicing the block
            if (pm_battle_status.partner_actor->ko_duration > 1) {
                pm_battle_status.partner_actor->ko_duration = 1;
                pm_battle_status.partner_actor->ptr_defuff_icon->ptr_property_list[15] = 1;
            }
        }
    }
}

void fp_lzs_trainer(void) {
    // detect if loading zone is stored
    // TODO: Yes this function looks awful.
    // If a cleaner way to detect stored loading zones is found or decomped, please submit a PR or let an fp dev know
    u32 *event_spc_ptr = &pm_curr_script_lst.script_list_ptr;
    u32 event_space = *event_spc_ptr;
    u32 event_count = pm_curr_script_lst.script_list_count;

    for (s32 event_priority = 0; event_priority < event_count; event_priority++) {
        u32 *event_id_ptr = (event_spc_ptr + 2 + event_priority);
        u32 event_id = *event_id_ptr;
        u32 *event_ptr_ptr = (u32 *)(event_space + (4 * event_id));
        u32 *event_ptr = (u32 *)*event_ptr_ptr;

        if (event_ptr) {
            u32 *callback_ptr = (u32 *)*(event_ptr + 2);
            if (callback_ptr) {
                u32 callback_function = *(callback_ptr + 0x5);
                if (callback_function == 0x802CA400) {
                    fp.lz_stored = 1;
                }
            }
        }
    }

    // Count frames since mario landed
    if (pm_player.action_state == ACTION_STATE_LAND || pm_player.action_state == ACTION_STATE_WALK ||
        pm_player.action_state == ACTION_STATE_RUN) {
        fp.player_landed = 1;
    }
    if (fp.player_landed) {
        fp.frames_since_land++;
    } else {
        fp.frames_since_land = 0;
    }
    if (pm_player.action_state == ACTION_STATE_JUMP) {
        fp.player_landed = 0;
    }

    // log lzs status
    if (fp.lz_stored && pm_status.pressed.a) {
        if (fp.prev_prev_action_state == ACTION_STATE_FALLING && pm_player.action_state == ACTION_STATE_JUMP &&
            pm_MapChangeState == 0) {
            fp_log("control early");
        } else if (pm_player.prev_action_state == ACTION_STATE_JUMP ||
                   pm_player.action_state == ACTION_STATE_SPIN_JUMP ||
                   pm_player.action_state == ACTION_STATE_ULTRA_JUMP) {
            fp_log("jump >= 2 frames early");
            if (pm_status.pressed.y_cardinal || fp.prev_pressed_y) {
                fp_log("control early");
            }
        } else if (pm_player.prev_action_state == ACTION_STATE_FALLING) {
            fp_log("jump 1 frame early");
            if (pm_player.action_state == ACTION_STATE_RUN || pm_player.action_state == ACTION_STATE_WALK) {
                fp_log("control early");
            }
        } else if (fp.prev_prev_action_state == ACTION_STATE_FALLING && pm_MapChangeState == 0) {
            fp_log("jump 1 frame late");
            fp_log("control early");
        } else if (fp.frames_since_land == 3) {
            fp_log("jump 1 frame late");
            if (pm_status.pressed.y_cardinal) {
                fp_log("control late");
            }
        } else if (fp.frames_since_land == 4) {
            fp_log("jump 2 frames late");
            if (pm_status.pressed.y_cardinal || fp.prev_pressed_y) {
                fp_log("control late");
            }
        } else if (fp.frames_since_land == 0 &&
                   (fp.prev_prev_action_state == ACTION_STATE_RUN || fp.prev_prev_action_state == ACTION_STATE_WALK)) {
            fp_log("jump >= 2 frames late");
            fp_log("control early");
        } else if (fp.frames_since_land >= 5 && pm_MapChangeState == 0) {
            fp_log("jump > 2 frames late");
            if (pm_status.pressed.y_cardinal || fp.prev_pressed_y) {
                fp_log("control late");
            }
        } else if (fp.frames_since_land == 2) {
            fp.current_lzs_jumps++;
        }
    }

    if (fp.current_lzs_jumps > fp.record_lzs_jumps) {
        fp.record_lzs_jumps = fp.current_lzs_jumps;
    }

    fp.prev_pressed_y = pm_status.pressed.y_cardinal;
    fp.prev_prev_action_state = pm_player.prev_action_state;

    if (pm_MapChangeState == 1) {
        fp.lz_stored = 0;
        fp.player_landed = 0;
        fp.frames_since_land = 0;
        fp.current_lzs_jumps = 0;
    }
}

void fp_clippy_trainer(void) {
    if (pm_status.pressed.cr && pm_encounter_status.first_strike != 2) {
        if (pm_GameState == 2 && pm_overworld.enable_partner_ability == 1) {
            fp.clippy_status = 1;
        } else if (fp.frames_since_battle > 0) {
            fp.clippy_status = 3;
        } else if (pm_GameState == 3 && fp.frames_since_battle == 0) {
            fp.clippy_status = 2;
        }
    }

    if (pm_GameState == 3) {
        fp.frames_since_battle++;
        switch (fp.clippy_status) {
            case 1: fp_log("early"); break;
            case 2: break; // Got clippy
            case 3: fp_log("late"); break;
        }
        fp.clippy_status = 0;
    } else if (pm_GameState != 3) {
        fp.frames_since_battle = 0;
    }
}

void fp_action_command_trainer(void) {
    // Either goombario or mario attacking
    if ((pm_battle_state_2 == 3 && pm_player.player_data.current_partner == PARTNER_GOOMBARIO) ||
        pm_battle_state_2 == 4) {
        if (pm_ActionCommandStatus.state == 10 && pm_status.pressed.a) {
            fp.last_a_press = pm_status.frame_counter;
        } else if (pm_ActionCommandStatus.state == 11) {
            if (fp.last_a_press) {
                u16 frames_early = pm_status.frame_counter - fp.last_a_press;
                fp_log("pressed A %d frame%s early", frames_early, frames_early > 1 ? "s" : "");
                fp.last_a_press = 0;
            }
            if (pm_status.pressed.a) {
                fp_log("pressed A frame %d out of %d",
                       pm_battle_status.unk_434[pm_ActionCommandStatus.unk_50] - pm_ActionCommandStatus.unk_54,
                       pm_battle_status.unk_434[pm_ActionCommandStatus.unk_50]);
            }
            fp.last_valid_frame = pm_status.frame_counter;
            // check for a press up to 10 frames late
        } else if (pm_ActionCommandStatus.state == 12 && pm_status.pressed.a &&
                   pm_status.frame_counter - fp.last_valid_frame <= 10) {
            u16 frames_late = pm_status.frame_counter - fp.last_valid_frame;
            fp_log("pressed A %d frame%s late", frames_late, frames_late > 1 ? "s" : "");
        }
    }
}

void fp_update_cheats(void) {
    if (CHEAT_ACTIVE(CHEAT_HP)) {
        pm_player.player_data.hp = pm_player.player_data.max_hp;
    }
    if (CHEAT_ACTIVE(CHEAT_FP)) {
        pm_player.player_data.fp = pm_player.player_data.max_fp;
    }
    if (CHEAT_ACTIVE(CHEAT_COINS)) {
        pm_player.player_data.coins = 999;
    }
    if (CHEAT_ACTIVE(CHEAT_STAR_POWER)) {
        pm_player.player_data.star_power.full_bars_filled = pm_player.player_data.star_power.star_spirits_saved;
        pm_player.player_data.star_power.partial_bars_filled = 0;
    }
    if (CHEAT_ACTIVE(CHEAT_STAR_PIECES)) {
        pm_player.player_data.star_pieces = 160;
    }
    if (CHEAT_ACTIVE(CHEAT_PERIL)) {
        pm_player.player_data.hp = 1;
    }
    if (CHEAT_ACTIVE(CHEAT_AUTO_MASH)) {
        if (pm_status.is_battle) {
            pm_ActionCommandStatus.barFillLevel = 10000;
        }
    }
    if (CHEAT_ACTIVE(CHEAT_BRIGHTEN_ROOM)) {
        set_screen_overlay_alpha(1, 0);
    }
    if (CHEAT_ACTIVE(CHEAT_AUTO_ACTION_CMD)) {
        pm_ActionCommandStatus.autoSucceed = 1;
    }
}

void fp_update_warps(void) {
    if (fp.warp_delay > 0) {
        PRINTF("fp.warp_delay: %d\n", fp.warp_delay);
        fp.warp_delay--;
    }

    if (fp.warp && fp.warp_delay == 0) {
        // if a popup menu is currently hidden, destroy it
        if (pm_popup_menu_var == 10) {
            PRINTF("destroying popup menu\n");
            pm_DestroyPopupMenu();
        }

        pm_SetMapTransitionEffect(0); // normal black fade
        PRINTF("changing game mode\n");
        pm_SetGameMode(5); // start the "change map" game mode
        fp.warp = 0;
    }
}

void fp_draw_log(struct gfx_font *font, s32 cell_width, s32 cell_height, u8 menu_alpha) {
    for (s32 i = SETTINGS_LOG_MAX - 1; i >= 0; --i) {
        const s32 fade_begin = 20;
        const s32 fade_duration = 20;
        struct log_entry *ent = &fp.log[i];
        u8 msg_alpha;
        if (!ent->msg) {
            continue;
        }
        ++ent->age;
        if (ent->age > (fade_begin + fade_duration)) {
            free(ent->msg);
            ent->msg = NULL;
            continue;
        } else if (!settings->bits.log) {
            continue;
        } else if (ent->age > fade_begin) {
            msg_alpha = 0xFF - (ent->age - fade_begin) * 0xFF / fade_duration;
        } else {
            msg_alpha = 0xFF;
        }
        msg_alpha = msg_alpha * menu_alpha / 0xFF;
        s32 msg_x = settings->log_x - cell_width * strlen(ent->msg);
        s32 msg_y = settings->log_y - cell_height * i;
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(0xC0C0C0, msg_alpha));
        gfx_printf(font, msg_x, msg_y, "%s", ent->msg);
    }
}

void fp_cam_update() {
    if (fp.free_cam) {
        if (!fp.cam_enabled_before) {
            fp.cam_pos.x = pm_gCameras->lookAt_eye.x;
            fp.cam_pos.y = pm_gCameras->lookAt_eye.y;
            fp.cam_pos.z = pm_gCameras->lookAt_eye.z;
            fp.cam_enabled_before = 1;
        } else {
            fp_update_cam();
        }
        Vec3f *camera_at = &pm_gCameras->lookAt_obj;
        Vec3f *camera_eye = &pm_gCameras->lookAt_eye;

        *camera_eye = fp.cam_pos;

        Vec3f vf;
        vec3f_py(&vf, fp.cam_pitch, fp.cam_yaw);
        vec3f_add(camera_at, camera_eye, &vf);
    }
}

/**
 * fp's main update function
 * This runs after the base games full update loop every frame
 */
void fp_update(void) {
    fp_update_cpu_counter();
    input_update();

    u16 pad_pressed = input_pressed();

    if (!fp.version_shown) {
        pm_status.skip_intro = 1;
    }

    if (!fp.settings_loaded) {
        if (!(input_pressed() & BUTTON_START) && settings_load(fp.profile)) {
            apply_menu_settings();
        }
        fp.settings_loaded = 1;
    }

    fp_emergency_settings_reset(pad_pressed);

    if (fp.menu_active) {
        fp_update_menu();
    } else if (input_bind_pressed_raw(COMMAND_MENU)) {
        show_menu();
    }

    fp.timer_count = 0;
    fp.lag_frames = 0;

    if (fp.timer.state != 0) {
        fp_update_timer(&fp.timer_count, &fp.lag_frames);
    }

    fp.timer.prev_cutscene_state = pm_player.flags & 0x00002000;

    if (fp.bowser_blocks_enabled) {
        fp_bowser_block_trainer();
    }

    if (fp.lzs_trainer_enabled) {
        fp_lzs_trainer();
    }

    if (fp.clippy_trainer_enabled) {
        fp_clippy_trainer();
    }

    if (fp.action_command_trainer_enabled) {
        fp_action_command_trainer();
    }

    fp_update_cheats();

    if (fp.turbo) {
        pm_player.run_speed = 24.0f;
    } else {
        pm_player.run_speed = 4.0f;
    }

    for (s32 i = 0; i < COMMAND_MAX; ++i) {
        _Bool active = 0;

        switch (fp_commands[i].command_type) {
            case COMMAND_HOLD: active = input_bind_held(i); break;
            case COMMAND_PRESS: active = input_bind_pressed(i); break;
            case COMMAND_PRESS_ONCE: active = input_bind_pressed_raw(i); break;
        }

        if (fp_commands[i].proc && active) {
            fp_commands[i].proc();
        }
    }

    fp_update_warps();

    // Override updateMode so update_cameras switch always defaults
    if (fp.free_cam) {
        pm_gCameras[pm_gCurrentCameraID].updateMode = 7;
    }
    fp_cam_update();

    while (fp.menu_active && menu_think(fp.main_menu)) {
        // wait
    }

    while (menu_think(fp.global)) {
        // wait
    }
}

/**
 * fp's main draw function
 * This runs after the game draws the front UI every frame
 */
void fp_draw(void) {
    gfx_mode_init();

    struct gfx_font *font = menu_get_font(fp.main_menu, 1);
    u8 menu_alpha = menu_get_alpha_i(fp.main_menu, 1);
    s32 cell_width = menu_get_cell_width(fp.main_menu, 1);
    s32 cell_height = menu_get_cell_height(fp.main_menu, 1);

    if (!fp.version_shown) {
        fp_draw_version(font, cell_width, cell_height, menu_alpha);
    }
    
    //TODO: remove before PR
    if (fp.test_icons[0] == NULL) {
        fp.test_icons[0] = icons_create_global(Icon_MashAButton, 32, 32, 255, 1.0f);
    } else {
        for (u32 i = 0; i < 10; i++) {
            if (fp.test_icons[i] != NULL) {
                icons_draw(fp.test_icons[i]);
            }
        }
    }

    if (settings->bits.input_display) {
        fp_draw_input_display(font, cell_width, cell_height, menu_alpha);
    }

    if (fp.timer.moving || (fp.timer.state == 3 && !fp.menu_active) ||
        (settings->bits.timer_show && !fp.menu_active && fp.timer.state > 0)) {
        fp_draw_timer(fp.timer_count, fp.lag_frames, font, cell_width, cell_height, menu_alpha);
    }

    if (fp.menu_active) {
        menu_draw(fp.main_menu);
    }

    menu_draw(fp.global);
    fp_draw_log(font, cell_width, cell_height, menu_alpha);
    gfx_flush();
}

/* ========================== HOOK ENTRY POINTS ========================== */

ENTRY void fp_update_entry(void) {
    init_gp();

    if (!fp.ready) {
        init_stack(fp_init);
        PRINTF("\n**** fp initialized ****\n\n");
    }

    step_game_loop();
    init_stack(fp_update);
}

ENTRY void fp_draw_entry(void) {
    init_gp();
    state_render_frontUI();
    init_stack(fp_draw);
}

ENTRY void fp_after_draw_entry(void) {
    crash_screen_set_draw_info_custom(nuGfxCfb_ptr, SCREEN_WIDTH, SCREEN_HEIGHT);
}

HOOK void fp_update_camera_mode_6(Camera *cam) {
    if (!fp.free_cam) {
        update_camera_mode_6(cam);
    }
}

HOOK void fp_update_input(void) {
    update_player_input();
    controller_t *mask = &fp.input_mask;

    pm_player.raw.buttons &= ~mask->buttons;
    pm_player.previous.buttons &= ~mask->buttons;
    pm_player.pad_held.buttons &= ~mask->buttons;

    pm_player.pad_x &= ~mask->x_cardinal;
    pm_player.pad_y &= ~mask->y_cardinal;
}

HOOK s32 fp_check_block_input(s32 buttonMask) {
    s32 mashWindow;
    s32 blockWindow;
    s32 block;
    s32 mash = 0;
    s32 bufferPos;
    s32 i;

    pm_battle_status.block_result = 0; // Fail

    if (pm_battle_status.unk_83 == -1 && (pm_battle_status.flags1 & 0x2000000)) {
        pm_battle_status.block_result = 1;
        return 1;
    }

    if (!pm_battle_status.unk_83 || (pm_status.demo_flag & 1)) {
        return 0;
    }

    if (pm_player.player_data.hits_taken < 9999) {
        pm_player.player_data.hits_taken += 1;
        pm_ActionCommandStatus.hitsTakenIsMax = 0;
    } else {
        pm_ActionCommandStatus.hitsTakenIsMax = 1;
    }

    block = 0;
    blockWindow = 3;
    mashWindow = 10;

    if (!(pm_battle_status.flags1 & 0x80000) && pm_is_ability_active(0)) {
        blockWindow = 5;
    }

    // Pre-window mashing check
    bufferPos = pm_battle_status.input_buffer_pos;
    bufferPos -= mashWindow + blockWindow;

    if (bufferPos < 0) {
        bufferPos += ARRAY_LENGTH(pm_battle_status.push_input_buffer);
    }
    for (i = 0; i < mashWindow; i++) {
        if (bufferPos >= ARRAY_LENGTH(pm_battle_status.push_input_buffer)) {
            bufferPos -= ARRAY_LENGTH(pm_battle_status.push_input_buffer);
        }

        if (pm_battle_status.push_input_buffer[bufferPos] & buttonMask) {
            if (fp.action_command_trainer_enabled) {
                s32 frames_early = mashWindow - i;
                fp_log("blocked %d frame%s early", frames_early, frames_early > 1 ? "s" : "");
            }
            mash = 1;
            break;
        }
        bufferPos++;
    }

    // Block check
    bufferPos = pm_battle_status.input_buffer_pos;
    bufferPos -= blockWindow;
    if (bufferPos < 0) {
        bufferPos += ARRAY_LENGTH(pm_battle_status.push_input_buffer);
    }
    for (i = 0; i < blockWindow; i++) {
        if (bufferPos >= ARRAY_LENGTH(pm_battle_status.push_input_buffer)) {
            bufferPos -= ARRAY_LENGTH(pm_battle_status.push_input_buffer);
        }

        if (pm_battle_status.push_input_buffer[bufferPos] & buttonMask) {
            if (!mash && fp.action_command_trainer_enabled) {
                fp_log("blocked frame %d out of %d", i + 1, blockWindow);
            }
            pm_battle_status.block_result = 1; // Block
            block = 1;
            break;
        }
        bufferPos++;
    }

    if (mash) {
        pm_battle_status.block_result = -1; // Mash
        block = 0;
    }

    // Ignore inputs until another mash window has passed, so check_block_input() can be called in quick succession
    if (block) {
        bufferPos = pm_battle_status.input_buffer_pos;
        bufferPos -= mashWindow + blockWindow + 20;
        if (bufferPos < 0) {
            bufferPos += ARRAY_LENGTH(pm_battle_status.push_input_buffer);
        }

        for (i = 0; i < mashWindow + blockWindow + 20; i++) {
            if (bufferPos >= ARRAY_LENGTH(pm_battle_status.push_input_buffer)) {
                bufferPos -= ARRAY_LENGTH(pm_battle_status.push_input_buffer);
            }
            pm_battle_status.push_input_buffer[bufferPos] = 0;
            bufferPos++;
        }
    }
    if (block && !pm_ActionCommandStatus.hitsTakenIsMax) {
        pm_player.player_data.hits_blocked += 1;
    }

    return block;
}

#include <startup.c>
#include <set/set.c>
#include <vector/vector.c>
#include <list/list.c>
#include <grc.c>
