#include "fp.h"
#include "commands.h"
#include "common.h"
#include "crash_screen.h"
#include "geometry.h"
#include "input.h"
#include "io.h"
#include "resource.h"
#include "timer.h"
#include "watchlist.h"
#include <n64.h>
#include <startup.h>
#include <stdlib.h>
#include <string.h>

__attribute__((section(".data"))) fp_ctxt_t fp = {
    .ready = FALSE,
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

    fp.profile = 0;
    fp.settings_loaded = FALSE;
    fp.version_shown = FALSE;
    fp.cpu_counter = 0;
    fp.cpu_counter_freq = 0;
    fp.menu_active = FALSE;

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
    fp.turbo = FALSE;
    fp.ace_last_timer = 0;
    fp.ace_last_flag_status = FALSE;
    fp.ace_last_jump_status = FALSE;
    fp.bowser_blocks_enabled = FALSE;
    fp.bowser_block = 0;
    fp.lzs_trainer_enabled = FALSE;
    fp.prev_prev_action_state = 0;
    fp.lz_stored = FALSE;
    fp.record_lzs_jumps = 0;
    fp.current_lzs_jumps = 0;
    fp.player_landed = FALSE;
    fp.frames_since_land = 0;
    fp.warp = FALSE;
    fp.warp_delay = 0;
    fp.frames_since_battle = 0;
    fp.clippy_status = 0;
    fp.clippy_trainer_enabled = FALSE;
    fp.last_imported_save_path = NULL;
    fp.free_cam = FALSE;
    fp.lock_cam = FALSE;
    fp.cam_bhv = CAMBHV_MANUAL;
    fp.cam_dist_min = 100;
    fp.cam_dist_max = 400;
    fp.cam_yaw = 0;
    fp.cam_pitch = 0;
    fp.cam_pos.x = 0;
    fp.cam_pos.y = 0;
    fp.cam_pos.z = 0;
    fp.cam_enabled_before = FALSE;
    fp.action_command_trainer_enabled = FALSE;

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
    input_bind_set_override(COMMAND_MENU, TRUE);
    input_bind_set_override(COMMAND_RETURN, TRUE);

    // get menu appearance
    apply_menu_settings();

    // skip intro on boot
    pm_gGameStatus.bSkipIntro = 1;

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

    fp.ready = TRUE;
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
        } else if (pad_pressed & BUTTON_D_DOWN) {
            menu_navigate(fp.main_menu, MENU_NAVIGATE_DOWN);
        } else if (pad_pressed & BUTTON_D_LEFT) {
            menu_navigate(fp.main_menu, MENU_NAVIGATE_LEFT);
        } else if (pad_pressed & BUTTON_D_RIGHT) {
            menu_navigate(fp.main_menu, MENU_NAVIGATE_RIGHT);
        } else if (pad_pressed & BUTTON_L) {
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
    static struct gfx_texture *fp_icon_tex;
    if (pm_gGameStatus.introState == 5) {
        fp.version_shown = TRUE;
    } else {
        if (fp_icon_tex == NULL) {
            fp_icon_tex = resource_load_pmicon_item(ITEM_FP_PLUS_A, FALSE);
        }
        struct gfx_sprite fp_icon_sprite = {
            fp_icon_tex, 0, 0, 15, SCREEN_HEIGHT - 65, 1.f, 1.f,
        };
        gfx_mode_replace(GFX_MODE_DROPSHADOW, 0);
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0xFF, 0xFF, 0xFF));
        gfx_sprite_draw(&fp_icon_sprite);
        gfx_mode_pop(GFX_MODE_DROPSHADOW);

        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0, 0, menu_alpha));
        gfx_printf(font, 16, SCREEN_HEIGHT - 35 + cell_height * 1, STRINGIFY(FP_VERSION));
        gfx_printf(font, SCREEN_WIDTH - cell_width * 21, SCREEN_HEIGHT - 35 + cell_height * 1, STRINGIFY(URL));
    }
}

void fp_draw_input_display(struct gfx_font *font, s32 cell_width, s32 cell_height, u8 menu_alpha) {
    u16 d_pad = pm_gGameStatus.currentButtons[0].buttons;
    s8 d_x = pm_gGameStatus.stickX[0];
    s8 d_y = pm_gGameStatus.stickY[0];

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
            control_stick, 0,   0, settings->input_display_x, settings->input_display_y - control_stick->tile_height,
            1.f,           1.f,
        };
        struct gfx_sprite in_stick = {
            control_stick,
            1,
            0,
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
            texture, b, 0, settings->input_display_x + button_dx + x, settings->input_display_y + y, 1.f, 1.f,
        };
        gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(input_button_color[b], menu_alpha));
        gfx_sprite_draw(&sprite);
    }
}

void fp_draw_timer(struct gfx_font *font, s32 cell_width, s32 cell_height, u8 menu_alpha) {
    s32 hundredths = timer_get_timer_count() * 100 / fp.cpu_counter_freq;
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

    gfx_printf(font, x, y + cell_height, "%d", timer_get_lag_frames());
}

// this whole thing should be redone once battles are better understood - freezing rng isn't very reliable
void fp_bowser_block_trainer(void) {
    if (pm_gGameStatus.isBattle && pm_gGameStatus.areaID == 0x4 &&
        (pm_gGameStatus.mapID == 0x7 || pm_gGameStatus.mapID == 0x13) && STORY_PROGRESS != STORY_INTRO &&
        !(pm_gGameStatus.peachFlags & (1 << 0))) {

        pm_Actor_t *bowser = pm_gBattleStatus.enemyActors[0];

        if (bowser != NULL) {
            s32 *turn = &bowser->state.varTable[0];
            s32 *turns_since_wave = &bowser->state.varTable[2];
            s32 *turns_since_beam = &bowser->state.varTable[3];
            s32 *turns_since_claw = &bowser->state.varTable[4];
            s32 *turns_since_stomp = &bowser->state.varTable[5];
            s32 *turns_since_heal = &bowser->state.varTable[6];
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

        if (pm_gBattleStatus.partnerActor != NULL) {
            // if partner is KO'd by wave, never let it last more than one turn so you can keep practicing the block
            if (pm_gBattleStatus.partnerActor->koDuration > 1) {
                pm_gBattleStatus.partnerActor->koDuration = 1;
            }
        }
    }
}

void fp_lzs_trainer(void) {
    // detect if loading zone is stored
    for (s32 evt_idx = 0; evt_idx < pm_gNumScripts; evt_idx++) {
        pm_Evt_t *script = (*pm_gCurrentScriptListPtr)[pm_gScriptIndexList[evt_idx]];
        if (script) {
            if (script->ptrNextLine) {
                u32 callback_function = script->ptrNextLine[5];
                if (callback_function == (uintptr_t)pm_GotoMap) {
                    fp.lz_stored = TRUE;
                }
            }
        }
    }

    // Count frames since mario landed
    if (pm_player.actionState == ACTION_STATE_LAND || pm_player.actionState == ACTION_STATE_WALK ||
        pm_player.actionState == ACTION_STATE_RUN) {
        fp.player_landed = TRUE;
    }
    if (fp.player_landed) {
        fp.frames_since_land++;
    } else {
        fp.frames_since_land = 0;
    }
    if (pm_player.actionState == ACTION_STATE_JUMP) {
        fp.player_landed = FALSE;
    }

    // log lzs status
    if (fp.lz_stored && pm_gGameStatus.pressedButtons[0].a) {
        if (fp.prev_prev_action_state == ACTION_STATE_FALLING && pm_player.actionState == ACTION_STATE_JUMP &&
            pm_MapChangeState == 0) {
            fp_log("control early");
        } else if (pm_player.prevActionState == ACTION_STATE_JUMP || pm_player.actionState == ACTION_STATE_SPIN_JUMP ||
                   pm_player.actionState == ACTION_STATE_ULTRA_JUMP) {
            fp_log("jump >= 2 frames early");
            if (pm_gGameStatus.pressedButtons[0].yCardinal || fp.prev_pressed_y) {
                fp_log("control early");
            }
        } else if (pm_player.prevActionState == ACTION_STATE_FALLING) {
            fp_log("jump 1 frame early");
            if (pm_player.actionState == ACTION_STATE_RUN || pm_player.actionState == ACTION_STATE_WALK) {
                fp_log("control early");
            }
        } else if (fp.prev_prev_action_state == ACTION_STATE_FALLING && pm_MapChangeState == 0) {
            fp_log("jump 1 frame late");
            fp_log("control early");
        } else if (fp.frames_since_land == 3) {
            fp_log("jump 1 frame late");
            if (pm_gGameStatus.pressedButtons[0].yCardinal) {
                fp_log("control late");
            }
        } else if (fp.frames_since_land == 4) {
            fp_log("jump 2 frames late");
            if (pm_gGameStatus.pressedButtons[0].yCardinal || fp.prev_pressed_y) {
                fp_log("control late");
            }
        } else if (fp.frames_since_land == 0 &&
                   (fp.prev_prev_action_state == ACTION_STATE_RUN || fp.prev_prev_action_state == ACTION_STATE_WALK)) {
            fp_log("jump >= 2 frames late");
            fp_log("control early");
        } else if (fp.frames_since_land >= 5 && pm_MapChangeState == 0) {
            fp_log("jump > 2 frames late");
            if (pm_gGameStatus.pressedButtons[0].yCardinal || fp.prev_pressed_y) {
                fp_log("control late");
            }
        } else if (fp.frames_since_land == 2) {
            fp.current_lzs_jumps++;
        }
    }

    if (fp.current_lzs_jumps > fp.record_lzs_jumps) {
        fp.record_lzs_jumps = fp.current_lzs_jumps;
    }

    fp.prev_pressed_y = pm_gGameStatus.pressedButtons[0].yCardinal;
    fp.prev_prev_action_state = pm_player.prevActionState;

    if (pm_MapChangeState == 1) {
        fp.lz_stored = FALSE;
        fp.player_landed = FALSE;
        fp.frames_since_land = 0;
        fp.current_lzs_jumps = 0;
    }
}

void fp_clippy_trainer(void) {
    if (pm_gGameStatus.pressedButtons[0].cr && pm_gCurrentEncounter.eFirstStrike != 2) {
        if (pm_GameState == 2 && pm_gPartnerActionStatus.partnerActionState == 1) {
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
    if ((pm_battle_state_2 == 3 && pm_player.playerData.currentPartner == PARTNER_GOOMBARIO) ||
        pm_battle_state_2 == 4) {
        if (pm_gActionCommandStatus.state == 10 && pm_gGameStatus.pressedButtons[0].a) {
            fp.last_a_press = pm_gGameStatus.frameCounter;
        } else if (pm_gActionCommandStatus.state == 11) {
            if (fp.last_a_press) {
                u16 frames_early = pm_gGameStatus.frameCounter - fp.last_a_press;
                fp_log("pressed A %d frame%s early", frames_early, frames_early > 1 ? "s" : "");
                fp.last_a_press = 0;
            }
            if (pm_gGameStatus.pressedButtons[0].a) {
                fp_log("pressed A frame %d out of %d",
                       pm_gBattleStatus.unk_434[pm_gActionCommandStatus.unk_50] - pm_gActionCommandStatus.unk_54,
                       pm_gBattleStatus.unk_434[pm_gActionCommandStatus.unk_50]);
            }
            fp.last_valid_frame = pm_gGameStatus.frameCounter;
            // check for a press up to 10 frames late
        } else if (pm_gActionCommandStatus.state == 12 && pm_gGameStatus.pressedButtons[0].a &&
                   pm_gGameStatus.frameCounter - fp.last_valid_frame <= 10) {
            u16 frames_late = pm_gGameStatus.frameCounter - fp.last_valid_frame;
            fp_log("pressed A %d frame%s late", frames_late, frames_late > 1 ? "s" : "");
        }
    }
}

void fp_update_cheats(void) {
    if (CHEAT_ACTIVE(CHEAT_HP)) {
        pm_player.playerData.curHP = pm_player.playerData.maxHP;
    }
    if (CHEAT_ACTIVE(CHEAT_FP)) {
        pm_player.playerData.curFP = pm_player.playerData.curMaxFP;
    }
    if (CHEAT_ACTIVE(CHEAT_COINS)) {
        pm_player.playerData.coins = 999;
    }
    if (CHEAT_ACTIVE(CHEAT_STAR_POWER)) {
        pm_player.playerData.starSpiritsFullBarsFilled = pm_player.playerData.starSpiritsSaved;
        pm_player.playerData.starSpiritsPartialBarFilled = 0;
    }
    if (CHEAT_ACTIVE(CHEAT_STAR_PIECES)) {
        pm_player.playerData.starPieces = 160;
    }
    if (CHEAT_ACTIVE(CHEAT_PERIL)) {
        pm_player.playerData.curHP = 1;
    }
    if (CHEAT_ACTIVE(CHEAT_AUTO_MASH)) {
        if (pm_gGameStatus.isBattle) {
            pm_gActionCommandStatus.barFillLevel = 10000;
        }
    }
    if (CHEAT_ACTIVE(CHEAT_BRIGHTEN_ROOM)) {
        pm_set_screen_overlay_alpha(1, 0);
    }
    if (CHEAT_ACTIVE(CHEAT_AUTO_ACTION_CMD)) {
        pm_gActionCommandStatus.autoSucceed = 1;
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
        PRINTF("changing game timer_mode\n");
        pm_SetGameMode(5); // start the "change map" game timer_mode
        fp.warp = FALSE;
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
            fp.cam_enabled_before = TRUE;
        } else {
            fp_update_cam();
        }
        vec3f_t *camera_at = &pm_gCameras->lookAt_obj;
        vec3f_t *camera_eye = &pm_gCameras->lookAt_eye;

        *camera_eye = fp.cam_pos;

        vec3f_t vf;
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
        pm_gGameStatus.bSkipIntro = 1;
    }

    if (!fp.settings_loaded) {
        if (!(input_pressed() & BUTTON_START) && settings_load(fp.profile)) {
            apply_menu_settings();
        }
        fp.settings_loaded = TRUE;
    }

    fp_emergency_settings_reset(pad_pressed);

    if (fp.menu_active) {
        fp_update_menu();
    } else if (input_bind_pressed_raw(COMMAND_MENU)) {
        show_menu();
    }

    timer_update();

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
        pm_player.runSpeed = 24.0f;
    } else {
        pm_player.runSpeed = 4.0f;
    }

    for (s32 i = 0; i < COMMAND_MAX; ++i) {
        bool active = FALSE;

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

    struct gfx_font *font = menu_get_font(fp.main_menu, TRUE);
    u8 menu_alpha = menu_get_alpha_i(fp.main_menu, TRUE);
    s32 cell_width = menu_get_cell_width(fp.main_menu, TRUE);
    s32 cell_height = menu_get_cell_height(fp.main_menu, TRUE);

    if (!fp.version_shown) {
        fp_draw_version(font, cell_width, cell_height, menu_alpha);
    }

    if (settings->bits.input_display) {
        fp_draw_input_display(font, cell_width, cell_height, menu_alpha);
    }

    enum timer_state timer_state = timer_get_state();
    if (fp.timer_moving || (timer_state == TIMER_STOPPED && !fp.menu_active) ||
        (settings->bits.timer_show && !fp.menu_active && timer_state != TIMER_INACTIVE)) {
        fp_draw_timer(font, cell_width, cell_height, menu_alpha);
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

    pm_step_game_loop();
    init_stack(fp_update);
}

ENTRY void fp_draw_entry(void) {
    init_gp();
    pm_state_render_frontUI();
    init_stack(fp_draw);
}

ENTRY void fp_after_draw_entry(void) {
    crash_screen_set_draw_info_custom(nuGfxCfb_ptr, SCREEN_WIDTH, SCREEN_HEIGHT);
}

HOOK void fp_update_camera_mode_6(pm_Camera_t *cam) {
    if (!fp.free_cam) {
        pm_update_camera_mode_6(cam);
    }
}

HOOK void fp_update_input(void) {
    pm_update_player_input();
    pm_Controller_t *mask = &fp.input_mask;

    pm_player.currentButtons.buttons &= ~mask->buttons;
    pm_player.previousButtons.buttons &= ~mask->buttons;
    pm_player.heldButtons.buttons &= ~mask->buttons;

    pm_player.stickAxisX &= ~mask->xCardinal;
    pm_player.stickAxisY &= ~mask->yCardinal;
}

HOOK s32 fp_check_block_input(s32 buttonMask) {
    s32 mashWindow;
    s32 blockWindow;
    s32 block;
    s32 mash = 0;
    s32 bufferPos;
    s32 i;

    pm_gBattleStatus.blockResult = 0; // Fail

    if (pm_gBattleStatus.unk_83 == -1 && (pm_gBattleStatus.flags1 & 0x2000000)) {
        pm_gBattleStatus.blockResult = 1;
        return 1;
    }

    if (!pm_gBattleStatus.unk_83 || (pm_gGameStatus.demoState & 1)) {
        return 0;
    }

    if (pm_player.playerData.hitsTaken < 9999) {
        pm_player.playerData.hitsTaken += 1;
        pm_gActionCommandStatus.hitsTakenIsMax = 0;
    } else {
        pm_gActionCommandStatus.hitsTakenIsMax = 1;
    }

    block = 0;
    blockWindow = 3;
    mashWindow = 10;

    if (!(pm_gBattleStatus.flags1 & 0x80000) && pm_is_ability_active(0)) {
        blockWindow = 5;
    }

    // Pre-window mashing check
    bufferPos = pm_gBattleStatus.inputBufferPos;
    bufferPos -= mashWindow + blockWindow;

    if (bufferPos < 0) {
        bufferPos += ARRAY_LENGTH(pm_gBattleStatus.pushInputBuffer);
    }
    for (i = 0; i < mashWindow; i++) {
        if (bufferPos >= ARRAY_LENGTH(pm_gBattleStatus.pushInputBuffer)) {
            bufferPos -= ARRAY_LENGTH(pm_gBattleStatus.pushInputBuffer);
        }

        if (pm_gBattleStatus.pushInputBuffer[bufferPos] & buttonMask) {
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
    bufferPos = pm_gBattleStatus.inputBufferPos;
    bufferPos -= blockWindow;
    if (bufferPos < 0) {
        bufferPos += ARRAY_LENGTH(pm_gBattleStatus.pushInputBuffer);
    }
    for (i = 0; i < blockWindow; i++) {
        if (bufferPos >= ARRAY_LENGTH(pm_gBattleStatus.pushInputBuffer)) {
            bufferPos -= ARRAY_LENGTH(pm_gBattleStatus.pushInputBuffer);
        }

        if (pm_gBattleStatus.pushInputBuffer[bufferPos] & buttonMask) {
            if (!mash && fp.action_command_trainer_enabled) {
                fp_log("blocked frame %d out of %d", i + 1, blockWindow);
            }
            pm_gBattleStatus.blockResult = 1; // Block
            block = 1;
            break;
        }
        bufferPos++;
    }

    if (mash) {
        pm_gBattleStatus.blockResult = -1; // Mash
        block = 0;
    }

    // Ignore inputs until another mash window has passed, so check_block_input() can be called in quick succession
    if (block) {
        bufferPos = pm_gBattleStatus.inputBufferPos;
        bufferPos -= mashWindow + blockWindow + 20;
        if (bufferPos < 0) {
            bufferPos += ARRAY_LENGTH(pm_gBattleStatus.pushInputBuffer);
        }

        for (i = 0; i < mashWindow + blockWindow + 20; i++) {
            if (bufferPos >= ARRAY_LENGTH(pm_gBattleStatus.pushInputBuffer)) {
                bufferPos -= ARRAY_LENGTH(pm_gBattleStatus.pushInputBuffer);
            }
            pm_gBattleStatus.pushInputBuffer[bufferPos] = 0;
            bufferPos++;
        }
    }
    if (block && !pm_gActionCommandStatus.hitsTakenIsMax) {
        pm_player.playerData.hitsBlocked += 1;
    }

    return block;
}

#include <grc.c>
#include <list/list.c>
#include <set/set.c>
#include <startup.c>
#include <vector/vector.c>
