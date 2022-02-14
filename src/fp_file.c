#include <stdio.h>
#include <stdlib.h>
#include "menu.h"
#include "settings.h"
#include "fp.h"
#include "files.h"
#include "sys.h"
#include "resource.h"

static void save_slot_dec_proc(struct menu_item *item, void *data) {
    pm_status.save_slot += 3;
    pm_status.save_slot %= 4;
}

static void save_slot_inc_proc(struct menu_item *item, void *data) {
    pm_status.save_slot += 1;
    pm_status.save_slot %= 4;
}

static void save_proc() {
    command_save_game_proc();
}

static void load_proc() {
    command_load_game_proc();
}

static s32 byte_mod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    s8 *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_intinput_get(item) != *p) {
            menu_intinput_set(item, *p);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        *p = menu_intinput_get(item);
    }
    return 0;
}

static s32 byte_optionmod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    u8 *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menu_option_get(item) != *p) {
            menu_option_set(item, *p);
        }
    } else if (reason == MENU_CALLBACK_DEACTIVATE) {
        *p = menu_option_get(item);
    }
    return 0;
}

static s32 checkbox_mod_proc(struct menu_item *item, enum menu_callback_reason reason, void *data) {
    u8 *p = data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        *p = 1;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        *p = 0;
    } else if (reason == MENU_CALLBACK_THINK) {
        menu_checkbox_set(item, *p);
    }
    return 0;
}

static s32 story_progress_draw_proc(struct menu_item *item, struct menu_draw_params *draw_params) {
    gfx_mode_set(GFX_MODE_COLOR, GPACK_RGB24A8(draw_params->color, draw_params->alpha));
    struct gfx_font *font = draw_params->font;
    s32 x = draw_params->x;
    s32 y = draw_params->y;

    s8 chapter_starts[] = {-128, -98, -74, -51, -12, 8, 40, 60, 90, 97};
    u8 chapter = 0;
    s8 chapter_progress = 0;
    s8 chapter_max = 0;
    for (s32 i = 1; i < 10; i++) {
        if (STORY_PROGRESS >= chapter_starts[i]) {
            chapter++;
        } else {
            chapter_progress = STORY_PROGRESS - chapter_starts[chapter];
            chapter_max = chapter_starts[chapter + 1] - chapter_starts[chapter];
            break;
        }
    }

    char buffer[24];
    if (chapter == 0) {
        sprintf(buffer, "- prologue (%d/%d)", chapter_progress, chapter_max);
    } else if (chapter > 8) {
        sprintf(buffer, "- invalid");
    } else {
        sprintf(buffer, "- chapter %d (%d/%d)", chapter, chapter_progress, chapter_max);
    }
    gfx_printf(font, x, y, buffer);
    return 1;
}

static void open_pipes_proc(struct menu_item *item, void *data) {
    fp_set_global_flag(0x1ad, 1); // 3 pipe blooper gone
    fp_set_global_flag(0x78d, 1); // 3 pipes opened
    fp_set_global_flag(0x791, 1); // goomba village pipe
    fp_set_global_flag(0x792, 1); // koopa village pipe
    fp_set_global_flag(0x793, 1); // dry dry outpost pipe

    fp_set_global_flag(0x1af, 1); // ch5 blooper gone
    fp_set_global_flag(0x78e, 1); // ch5 sewer pipe
    fp_set_global_flag(0x795, 1); // lavalava island pipe

    fp_set_global_flag(0x1b0, 1); // dark koopas gone
    fp_set_global_flag(0x78f, 1); // dark koopa pipe
    fp_set_global_flag(0x794, 1); // boo's mansion pipe
}

static void restore_enemies_proc(struct menu_item *item, void *data) {
    for (s32 i = 0; i < 600; i++) {
        pm_enemy_defeat_flags[i] = 0;
    }
}

static void restore_letters_proc(struct menu_item *item, void *data) {
    fp_set_global_flag(0x2c1, 0);
    fp_set_global_flag(0x2c2, 0);
    fp_set_global_flag(0x2c3, 0);
    fp_set_global_flag(0x2f5, 0);
    fp_set_global_flag(0x340, 0);
    fp_set_global_flag(0x341, 0);
    fp_set_global_flag(0x4c6, 0);
    fp_set_global_flag(0x4cb, 0);
    fp_set_global_flag(0x56d, 0);
    fp_set_global_flag(0x5a6, 0);
    fp_set_global_flag(0x5a9, 0);
}

static s32 do_export_file(const char *path, void *data) {
    const char *err_str = NULL;
    s32 f = creat(path, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    save_data_ctxt_t *file = data;
    if (f != -1) {
        if (write(f, file, sizeof(*file)) != sizeof(*file)) {
            err_str = strerror(errno);
        } else {
            if (close(f)) {
                err_str = strerror(errno);
            }
            f = -1;
        }
    } else {
        err_str = strerror(errno);
    }

    if (f != -1) {
        close(f);
    }
    if (file) {
        free(file);
    }
    if (err_str) {
        menu_prompt(fp.main_menu, err_str, "return\0", 0, NULL, NULL);
        return 1;
    } else {
        fp_log("exported file %d to disk", file->save_slot);
        return 0;
    }
}

s32 fp_import_file(const char *path, void *data) {
    const char *s_invalid = "invalid or corrupt file";
    const char *s_memory = "out of memory";
    const char *err_str = NULL;
    save_data_ctxt_t *file = NULL;
    s32 f = open(path, O_RDONLY);
    if (f != -1) {
        struct stat st;
        if (fstat(f, &st)) {
            err_str = strerror(errno);
        } else if (st.st_size != sizeof(*file)) {
            err_str = s_invalid;
        } else {
            file = malloc(sizeof(*file));
            if (!file) {
                err_str = s_memory;
            } else {
                errno = 0;
                if (read(f, file, sizeof(*file)) != sizeof(*file)) {
                    if (errno == 0) {
                        err_str = s_invalid;
                    } else {
                        err_str = strerror(errno);
                    }
                } else {
                    if (pm_FioValidateFileChecksum(file)) {
                        pm_save_data = *file;
                        fp_warp(file->area_id, file->map_id, file->entrance_id);
                    } else {
                        fp_log("save file corrupt");
                    }
                }
            }
        }
    } else {
        err_str = strerror(errno);
    }

    if (f != -1) {
        close(f);
    }
    if (file) {
        free(file);
    }
    if (err_str) {
        menu_prompt(fp.main_menu, err_str, "return\0", 0, NULL, NULL);
        return 1;
    } else {
        if (fp.last_imported_save_path == NULL) {
            fp.last_imported_save_path = malloc(PATH_MAX);
        }
        strcpy(fp.last_imported_save_path, path);
        fp_log("external save loaded");
        return 0;
    }
}

static void export_file_proc(struct menu_item *item, void *data) {
    save_data_ctxt_t *file = malloc(sizeof(*file));
    pm_FioFetchSavedFileInfo();
    pm_FioReadFlash(pm_save_info.logical_save_info[pm_status.save_slot][0], file, sizeof(*file));

    if (pm_FioValidateFileChecksum(file)) {
        menu_get_file(fp.main_menu, GETFILE_SAVE, "file", ".pmsave", do_export_file, file);
    } else {
        free(file);
        fp_log("no file in slot %d", pm_status.save_slot);
    }
}

static void import_file_proc(struct menu_item *item, void *data) {
    menu_get_file(fp.main_menu, GETFILE_LOAD, NULL, ".pmsave", fp_import_file, NULL);
}

struct menu *create_file_menu(void) {
    static struct menu menu;
    struct menu_item *item;

    /* initialize menu */
    menu_init(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    /*build menu*/
    s32 y = 0;
    const s32 MENU_X = 17;
    struct gfx_texture *t_save = resource_get(RES_ICON_SAVE);

    menu.selector = menu_add_submenu(&menu, 0, y++, NULL, "return");
    menu_add_static(&menu, 0, y, "save slot", 0xC0C0C0);
    menu_add_button(&menu, 11, y, "-", save_slot_dec_proc, NULL);
    menu_add_watch(&menu, 13, y, (u32)&pm_status.save_slot, WATCH_TYPE_U8);
    menu_add_button(&menu, 15, y++, "+", save_slot_inc_proc, NULL);
    y++;
    item = menu_add_button_icon(&menu, 0, y, t_save, 3, 0xFFFFFF, save_proc, NULL);
    item->tooltip = "save";
    item = menu_add_button_icon(&menu, 3, y, t_save, 2, 0xFFFFFF, load_proc, NULL);
    item->tooltip = "load";
    item = menu_add_button_icon(&menu, 6, y, t_save, 1, 0xFFFFFF, export_file_proc, NULL);
    item->tooltip = "export";
    item = menu_add_button_icon(&menu, 9, y, t_save, 0, 0xFFFFFF, import_file_proc, NULL);
    item->tooltip = "import";
    menu_add_tooltip(&menu, 12, y++, fp.main_menu, 0xC0C0C0);
    y++;
    menu_add_static(&menu, 0, y, "story progress", 0xC0C0C0);
    menu_add_intinput(&menu, MENU_X, y++, 16, 2, byte_mod_proc, &STORY_PROGRESS);
    menu_add_static_custom(&menu, 1, y++, story_progress_draw_proc, NULL, 0xC0C0C0);
    y++;
    menu_add_static(&menu, 0, y, "music", 0xC0C0C0);
    menu_add_checkbox(&menu, MENU_X, y++, checkbox_mod_proc, &pm_status.music_enabled);
    menu_add_static(&menu, 0, y, "quizzes answered", 0xC0C0C0);
    menu_add_intinput(&menu, MENU_X, y++, 10, 2, byte_mod_proc, &pm_save_data.global_bytes[0x161]);
    menu_add_static(&menu, 0, y, "peach item 1", 0xC0C0C0);
    menu_add_option(&menu, MENU_X, y++,
                    "goomba\0"
                    "clubba\0"
                    "mushroom\0",
                    byte_optionmod_proc, &pm_save_data.global_bytes[0xD8]);
    menu_add_static(&menu, 0, y, "peach item 2", 0xC0C0C0);
    menu_add_option(&menu, MENU_X, y++,
                    "fuzzy\0"
                    "hammer bros.\0"
                    "thunder rage\0",
                    byte_optionmod_proc, &pm_save_data.global_bytes[0xD9]);
    menu_add_static(&menu, 0, y, "peach item 3", 0xC0C0C0);
    menu_add_option(&menu, MENU_X, y++,
                    "pokey\0"
                    "koopatrol\0"
                    "super soda\0",
                    byte_optionmod_proc, &pm_save_data.global_bytes[0xDA]);
    menu_add_button(&menu, 0, y++, "open shortcut pipes", open_pipes_proc, NULL);
    menu_add_button(&menu, 0, y++, "restore enemies", restore_enemies_proc, NULL);
    menu_add_button(&menu, 0, y++, "restore letters", restore_letters_proc, NULL);

    return &menu;
}
