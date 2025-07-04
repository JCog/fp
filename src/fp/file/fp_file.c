#include "files.h"
#include "fp.h"
#include "io/sys.h"
#include "menu/menu.h"
#include "sys/resource.h"
#include "sys/settings.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>

static void saveSlotDecProc(struct MenuItem *item, void *data) {
    pm_gGameStatus.saveSlot += 3;
    pm_gGameStatus.saveSlot %= 4;
}

static void saveSlotIncProc(struct MenuItem *item, void *data) {
    pm_gGameStatus.saveSlot += 1;
    pm_gGameStatus.saveSlot %= 4;
}

static s32 storyProgressDrawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
    struct GfxFont *font = drawParams->font;
    s32 x = drawParams->x;
    s32 y = drawParams->y;

    s8 chapterStarts[] = {-128, -98, -74, -51, -12, 8, 40, 60, 90, 97};
    u8 chapter = 0;
    s8 chapterProgress = 0;
    s8 chapterMax = 0;
    for (s32 i = 1; i < 10; i++) {
        if (STORY_PROGRESS >= chapterStarts[i]) {
            chapter++;
        } else {
            chapterProgress = STORY_PROGRESS - chapterStarts[chapter];
            chapterMax = chapterStarts[chapter + 1] - chapterStarts[chapter];
            break;
        }
    }

    char buffer[24];
    if (chapter == 0) {
        snprintf(buffer, sizeof(buffer), "pro. (%d/%d)", chapterProgress, chapterMax);
    } else if (chapter > 8) {
        snprintf(buffer, sizeof(buffer), "invalid");
    } else {
        snprintf(buffer, sizeof(buffer), "ch%d (%d/%d)", chapter, chapterProgress, chapterMax);
    }
    gfxPrintf(font, x, y, buffer);
    return 1;
}

static void restoreEnemiesProc(struct MenuItem *item, void *data) {
    for (s32 i = 0; i < 60; i++) {
        for (s32 j = 0; j < 12; j++) {
            pm_gCurrentEncounter.defeatFlags[i][j] = 0;
        }
    }
}

static s32 doExportFile(const char *path, void *data) {
    const char *errStr = NULL;
    s32 f = creat(path, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    pm_SaveData *file = data;
    if (f != -1) {
        if (write(f, file, sizeof(*file)) != sizeof(*file)) {
            errStr = strerror(errno);
        } else {
            if (close(f)) {
                errStr = strerror(errno);
            }
            f = -1;
        }
    } else {
        errStr = strerror(errno);
    }

    if (f != -1) {
        close(f);
    }
    if (file) {
        free(file);
    }
    if (errStr) {
        menuPrompt(fp.mainMenu, errStr, "return\0", 0, NULL, NULL);
        return 1;
    } else {
        fpLog("exported file %d to disk", file->saveSlot);
        return 0;
    }
}

s32 fpImportFile(const char *path, void *data) {
    const char *sInvalid = "invalid or corrupt file";
    const char *sMemory = "out of memory";
    const char *errStr = NULL;
    pm_SaveData *file = NULL;
    s32 f = open(path, O_RDONLY);
    if (f != -1) {
        struct stat st;
        if (fstat(f, &st)) {
            errStr = strerror(errno);
        } else if (st.st_size != sizeof(*file)) {
            errStr = sInvalid;
        } else {
            file = malloc(sizeof(*file));
            if (!file) {
                errStr = sMemory;
            } else {
                errno = 0;
                if (read(f, file, sizeof(*file)) != sizeof(*file)) {
                    if (errno == 0) {
                        errStr = sInvalid;
                    } else {
                        errStr = strerror(errno);
                    }
                } else {
                    if (pm_fio_validate_globals_checksums(file)) {
                        pm_gCurrentSaveFile = *file;
                        pm_fio_deserialize_state();
                        fpWarp(file->areaID, file->mapID, file->entryID);
                    } else {
                        fpLog("save file corrupt");
                    }
                }
            }
        }
    } else {
        errStr = strerror(errno);
    }

    if (f != -1) {
        close(f);
    }
    if (file) {
        free(file);
    }
    if (errStr) {
        menuPrompt(fp.mainMenu, errStr, "return\0", 0, NULL, NULL);
        return 1;
    } else {
        if (fp.lastImportedSavePath == NULL) {
            fp.lastImportedSavePath = malloc(PATH_MAX);
        }
        strcpy(fp.lastImportedSavePath, path);
        fpLog("external save loaded");
        return 0;
    }
}

static void exportFileProc(struct MenuItem *item, void *data) {
    pm_SaveData *file = malloc(sizeof(*file));
    pm_fio_fetch_saved_file_info();
    pm_fio_read_flash(pm_LogicalSaveInfo[pm_gGameStatus.saveSlot][0], file, sizeof(*file));

    if (pm_fio_validate_globals_checksums(file)) {
        menuGetFile(fp.mainMenu, GETFILE_SAVE, "file", ".pmsave", doExportFile, file);
    } else {
        free(file);
        fpLog("no file in slot %d", pm_gGameStatus.saveSlot);
    }
}

static void importFileProc(struct MenuItem *item, void *data) {
    menuGetFile(fp.mainMenu, GETFILE_LOAD, NULL, ".pmsave", fpImportFile, NULL);
}

static s32 audioModeProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    u32 *useMonoSound = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menuOptionGet(item) != *useMonoSound) {
            menuOptionSet(item, *useMonoSound);
        }
    } else if (reason == MENU_CALLBACK_DEACTIVATE) {
        *useMonoSound = menuOptionGet(item);
        pm_fio_save_globals();
        pm_gGameStatus.soundOutputMode = !*useMonoSound;
        if (*useMonoSound) {
            pm_audio_set_mono();
        } else {
            pm_audio_set_stereo();
        }
    }
    return 0;
}

struct Menu *createFileMenu(void) {
    static struct Menu menu;

    /* initialize menu */
    menuInit(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    /*build menu*/
    s32 y = 0;
    const s32 menuX = 11;
    struct GfxTexture *tSave = resourceGet(RES_ICON_SAVE);

    menu.selector = menuAddSubmenu(&menu, 0, y++, NULL, "return");
    y++;
    struct MenuItem *saveButton =
        menuAddButtonIcon(&menu, 0, y, tSave, 3, 0, 0xFFFFFF, 1.0f, menuFuncProc, commandSaveGameProc);
    saveButton->tooltip = "save";
    struct MenuItem *loadButton =
        menuAddButtonIcon(&menu, 3, y, tSave, 2, 0, 0xFFFFFF, 1.0f, menuFuncProc, commandLoadGameProc);
    loadButton->tooltip = "load";
    struct MenuItem *exportButton = menuAddButtonIcon(&menu, 6, y, tSave, 1, 0, 0xFFFFFF, 1.0f, exportFileProc, NULL);
    exportButton->tooltip = "export";
    struct MenuItem *importButton = menuAddButtonIcon(&menu, 9, y, tSave, 0, 0, 0xFFFFFF, 1.0f, importFileProc, NULL);
    importButton->tooltip = "import";
    menuAddTooltip(&menu, 12, y++, fp.mainMenu, 0xC0C0C0);
    struct MenuItem *fileButtons[4] = {saveButton, loadButton, exportButton, importButton};
    menuItemCreateChain(fileButtons, 4, MENU_NAVIGATE_RIGHT, TRUE, FALSE);
    menuItemCreateChain(fileButtons, 4, MENU_NAVIGATE_LEFT, TRUE, TRUE);
    y++;

    menuAddStatic(&menu, 0, y, "slot", 0xC0C0C0);
    struct MenuItem *minusButton = menuAddButton(&menu, 6, y, "-", saveSlotDecProc, NULL);
    menuAddWatch(&menu, 8, y, (u32)&pm_gGameStatus.saveSlot, WATCH_TYPE_U8);
    struct MenuItem *plusButton = menuAddButton(&menu, 10, y++, "+", saveSlotIncProc, NULL);
    struct MenuItem *slotButtons[2] = {minusButton, plusButton};
    menuItemCreateChain(slotButtons, 2, MENU_NAVIGATE_RIGHT, TRUE, FALSE);
    menuItemCreateChain(slotButtons, 2, MENU_NAVIGATE_LEFT, TRUE, TRUE);
    menuItemAddChainLink(saveButton, minusButton, MENU_NAVIGATE_DOWN);
    menuItemAddChainLink(loadButton, minusButton, MENU_NAVIGATE_DOWN);
    menuItemAddChainLink(exportButton, minusButton, MENU_NAVIGATE_DOWN);
    menuItemAddChainLink(importButton, minusButton, MENU_NAVIGATE_DOWN);
    y++;

    menuAddStatic(&menu, 0, y, "story progress", 0xC0C0C0);
    struct MenuItem *progressInput = menuAddIntinput(&menu, 15, y++, 16, 2, menuByteModProc, &STORY_PROGRESS);
    menuAddStaticCustom(&menu, 0, y++, storyProgressDrawProc, NULL, 0xC0C0C0);
    menuItemAddChainLink(minusButton, progressInput, MENU_NAVIGATE_DOWN);
    menuItemAddChainLink(plusButton, progressInput, MENU_NAVIGATE_DOWN);
    y++;

    menuAddStatic(&menu, 0, y, "quizmo", 0xC0C0C0);
    struct MenuItem *quizmoInput =
        menuAddIntinput(&menu, menuX, y++, 10, 2, menuByteModProc, &pm_gCurrentSaveFile.globalBytes[0x161]);
    menuItemAddChainLink(quizmoInput, progressInput, MENU_NAVIGATE_UP);
    menuAddStatic(&menu, 0, y, "toy box 1", 0xC0C0C0);
    menuAddOption(&menu, menuX, y++,
                  "goomba\0"
                  "clubba\0"
                  "mushroom\0",
                  menuByteOptionmodProc, &pm_gCurrentSaveFile.globalBytes[0xD8]);
    menuAddStatic(&menu, 0, y, "toy box 2", 0xC0C0C0);
    menuAddOption(&menu, menuX, y++,
                  "fuzzy\0"
                  "hammer bros.\0"
                  "thunder rage\0",
                  menuByteOptionmodProc, &pm_gCurrentSaveFile.globalBytes[0xD9]);
    menuAddStatic(&menu, 0, y, "toy box 3", 0xC0C0C0);
    menuAddOption(&menu, menuX, y++,
                  "pokey\0"
                  "koopatrol\0"
                  "super soda\0",
                  menuByteOptionmodProc, &pm_gCurrentSaveFile.globalBytes[0xDA]);
    y++;
    menuAddStatic(&menu, 0, y, "audio mode", 0xC0C0C0);
    struct MenuItem *audioOption = menuAddOption(&menu, menuX, y++,
                                                 "stereo\0"
                                                 "mono\0",
                                                 audioModeProc, &pm_gSaveGlobals.useMonoSound);
    y++;

    struct MenuItem *enemiesButton = menuAddButton(&menu, 0, y++, "restore enemies", restoreEnemiesProc, NULL);
    menuItemAddChainLink(enemiesButton, audioOption, MENU_NAVIGATE_UP);

    return &menu;
}
