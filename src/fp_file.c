#include "files.h"
#include "fp.h"
#include "menu.h"
#include "resource.h"
#include "settings.h"
#include "sys.h"
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

static void saveProc(struct MenuItem *item, void *data) {
    commandSaveGameProc();
}

static void loadProc(struct MenuItem *item, void *data) {
    commandLoadGameProc();
}

static s32 byteModProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    s8 *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menuIntinputGet(item) != *p) {
            menuIntinputSet(item, *p);
        }
    } else if (reason == MENU_CALLBACK_CHANGED) {
        *p = menuIntinputGet(item);
    }
    return 0;
}

static s32 byteOptionmodProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    u8 *p = data;
    if (reason == MENU_CALLBACK_THINK_INACTIVE) {
        if (menuOptionGet(item) != *p) {
            menuOptionSet(item, *p);
        }
    } else if (reason == MENU_CALLBACK_DEACTIVATE) {
        *p = menuOptionGet(item);
    }
    return 0;
}

static s32 checkboxModProc(struct MenuItem *item, enum MenuCallbackReason reason, void *data) {
    u8 *p = data;
    if (reason == MENU_CALLBACK_SWITCH_ON) {
        *p = 1;
    } else if (reason == MENU_CALLBACK_SWITCH_OFF) {
        *p = 0;
    } else if (reason == MENU_CALLBACK_THINK) {
        menuCheckboxSet(item, *p);
    }
    return 0;
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
        sprintf(buffer, "- prologue (%d/%d)", chapterProgress, chapterMax);
    } else if (chapter > 8) {
        sprintf(buffer, "- invalid");
    } else {
        sprintf(buffer, "- chapter %d (%d/%d)", chapter, chapterProgress, chapterMax);
    }
    gfxPrintf(font, x, y, buffer);
    return 1;
}

static void openPipesProc(struct MenuItem *item, void *data) {
    fpSetGlobalFlag(0x1ad, TRUE); // 3 pipe blooper gone
    fpSetGlobalFlag(0x78d, TRUE); // 3 pipes opened
    fpSetGlobalFlag(0x791, TRUE); // goomba village pipe
    fpSetGlobalFlag(0x792, TRUE); // koopa village pipe
    fpSetGlobalFlag(0x793, TRUE); // dry dry outpost pipe

    fpSetGlobalFlag(0x1af, TRUE); // ch5 blooper gone
    fpSetGlobalFlag(0x78e, TRUE); // ch5 sewer pipe
    fpSetGlobalFlag(0x795, TRUE); // lavalava island pipe

    fpSetGlobalFlag(0x1b0, TRUE); // dark koopas gone
    fpSetGlobalFlag(0x78f, TRUE); // dark koopa pipe
    fpSetGlobalFlag(0x794, TRUE); // boo's mansion pipe
}

static void restoreEnemiesProc(struct MenuItem *item, void *data) {
    for (s32 i = 0; i < 60; i++) {
        for (s32 j = 0; j < 12; j++) {
            pm_gCurrentEncounter.defeatFlags[i][j] = 0;
        }
    }
}

static void restoreLettersProc(struct MenuItem *item, void *data) {
    fpSetGlobalFlag(0x2c1, FALSE);
    fpSetGlobalFlag(0x2c2, FALSE);
    fpSetGlobalFlag(0x2c3, FALSE);
    fpSetGlobalFlag(0x2f5, FALSE);
    fpSetGlobalFlag(0x340, FALSE);
    fpSetGlobalFlag(0x341, FALSE);
    fpSetGlobalFlag(0x4c6, FALSE);
    fpSetGlobalFlag(0x4cb, FALSE);
    fpSetGlobalFlag(0x56d, FALSE);
    fpSetGlobalFlag(0x5a6, FALSE);
    fpSetGlobalFlag(0x5a9, FALSE);
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
                    if (pm_fioValidateFileChecksum(file)) {
                        pm_gCurrentSaveFile = *file;
                        pm_fioDeserializeState();
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
    pm_fioFetchSavedFileInfo();
    pm_fioReadFlash(pm_logicalSaveInfo[pm_gGameStatus.saveSlot][0], file, sizeof(*file));

    if (pm_fioValidateFileChecksum(file)) {
        menuGetFile(fp.mainMenu, GETFILE_SAVE, "file", ".pmsave", doExportFile, file);
    } else {
        free(file);
        fpLog("no file in slot %d", pm_gGameStatus.saveSlot);
    }
}

static void importFileProc(struct MenuItem *item, void *data) {
    menuGetFile(fp.mainMenu, GETFILE_LOAD, NULL, ".pmsave", fpImportFile, NULL);
}

struct Menu *createFileMenu(void) {
    static struct Menu menu;
    struct MenuItem *item;

    /* initialize menu */
    menuInit(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    /*build menu*/
    s32 y = 0;
    const s32 menuX = 17;
    struct GfxTexture *tSave = resourceGet(RES_ICON_SAVE);

    menu.selector = menuAddSubmenu(&menu, 0, y++, NULL, "return");
    menuAddStatic(&menu, 0, y, "save slot", 0xC0C0C0);
    menuAddButton(&menu, 11, y, "-", saveSlotDecProc, NULL);
    menuAddWatch(&menu, 13, y, (u32)&pm_gGameStatus.saveSlot, WATCH_TYPE_U8);
    menuAddButton(&menu, 15, y++, "+", saveSlotIncProc, NULL);
    y++;
    item = menuAddButtonIcon(&menu, 0, y, tSave, 3, 0, 0xFFFFFF, 1.0f, saveProc, NULL);
    item->tooltip = "save";
    item = menuAddButtonIcon(&menu, 3, y, tSave, 2, 0, 0xFFFFFF, 1.0f, loadProc, NULL);
    item->tooltip = "load";
    item = menuAddButtonIcon(&menu, 6, y, tSave, 1, 0, 0xFFFFFF, 1.0f, exportFileProc, NULL);
    item->tooltip = "export";
    item = menuAddButtonIcon(&menu, 9, y, tSave, 0, 0, 0xFFFFFF, 1.0f, importFileProc, NULL);
    item->tooltip = "import";
    menuAddTooltip(&menu, 12, y++, fp.mainMenu, 0xC0C0C0);
    y++;
    menuAddStatic(&menu, 0, y, "story progress", 0xC0C0C0);
    menuAddIntinput(&menu, menuX, y++, 16, 2, byteModProc, &STORY_PROGRESS);
    menuAddStaticCustom(&menu, 1, y++, storyProgressDrawProc, NULL, 0xC0C0C0);
    y++;
    menuAddStatic(&menu, 0, y, "music", 0xC0C0C0);
    menuAddCheckbox(&menu, menuX, y++, checkboxModProc, &pm_gGameStatus.musicEnabled);
    menuAddStatic(&menu, 0, y, "quizzes answered", 0xC0C0C0);
    menuAddIntinput(&menu, menuX, y++, 10, 2, byteModProc, &pm_gCurrentSaveFile.globalBytes[0x161]);
    menuAddStatic(&menu, 0, y, "peach item 1", 0xC0C0C0);
    menuAddOption(&menu, menuX, y++,
                  "goomba\0"
                  "clubba\0"
                  "mushroom\0",
                  byteOptionmodProc, &pm_gCurrentSaveFile.globalBytes[0xD8]);
    menuAddStatic(&menu, 0, y, "peach item 2", 0xC0C0C0);
    menuAddOption(&menu, menuX, y++,
                  "fuzzy\0"
                  "hammer bros.\0"
                  "thunder rage\0",
                  byteOptionmodProc, &pm_gCurrentSaveFile.globalBytes[0xD9]);
    menuAddStatic(&menu, 0, y, "peach item 3", 0xC0C0C0);
    menuAddOption(&menu, menuX, y++,
                  "pokey\0"
                  "koopatrol\0"
                  "super soda\0",
                  byteOptionmodProc, &pm_gCurrentSaveFile.globalBytes[0xDA]);
    menuAddButton(&menu, 0, y++, "open shortcut pipes", openPipesProc, NULL);
    menuAddButton(&menu, 0, y++, "restore enemies", restoreEnemiesProc, NULL);
    menuAddButton(&menu, 0, y++, "restore letters", restoreLettersProc, NULL);

    return &menu;
}
