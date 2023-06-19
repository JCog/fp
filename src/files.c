#include "files.h"
#include "menu/menu.h"
#include "util/osk.h"
#include "sys/resource.h"
#include "io/sys.h"
#include <errno.h>
#include <set/set.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <vector/vector.h>

#define FILE_VIEW_ROWS 14

/* params */
static enum GetFileMode gfMode;
static char *gfSuffix;
static s32 gfSuffixLength;
static GetFileCallback gfCallbackProc;
static void *gfCallbackData;
/* data */
static struct Menu gfMenu;
static struct vector gfDirState;
static struct set gfDirEntries;
static bool gfUntitled;
static bool gfDirtyName;
/* menus */
static struct MenuItem *gfReset;
static struct MenuItem *gfLocation;
static struct MenuItem *gfName;
static struct MenuItem *gfAccept;
static struct MenuItem *gfClear;
static struct MenuItem *gfMkdir;
static struct MenuItem *gfScrollUp;
static struct MenuItem *gfScrollDown;
static struct MenuItem *gfFiles[FILE_VIEW_ROWS];

struct DirState {
    s32 scroll;
    s32 index;
};

struct DirEntry {
    char name[256];
    bool dir;
    s32 tile;
    char text[32];
    s32 animState;
};

static bool stricmp(const char *a, const char *b) {
    while (*a && *b) {
        char ca = *a++;
        char cb = *b++;
        if (ca >= 'a' && ca <= 'z') {
            ca += 'A' - 'a';
        }
        if (cb >= 'a' && cb <= 'z') {
            cb += 'A' - 'a';
        }
        if (ca != cb) {
            return FALSE;
        }
    }
    return *a == *b;
}

static bool updateList(void) {
    /* clear entries */
    vector_clear(&gfDirEntries.container);
    /* update location */
    DIR *dir = opendir(".");
    if (!dir) {
        if (errno == ENODEV) {
            strcpy(gfLocation->text, "no disk");
        } else if (errno == ENOENT) {
            strcpy(gfLocation->text, "no file system");
        } else {
            strncpy(gfLocation->text, strerror(errno), 31);
            gfLocation->text[31] = 0;
        }
        return FALSE;
    }
    getcwd(gfLocation->text, 32);
    gfLocation->text[31] = 0;
    /* enumerate and sort entries */
    struct Dirent *dirent;
    while ((dirent = readdir(dir))) {
        if ((dirent->dName[0] == '.' && strcmp(dirent->dName, "..") != 0) || !(dirent->mode & S_IRUSR)) {
            continue;
        }
        bool dir = ((dirent->mode & S_IFMT) == S_IFDIR);
        s32 nl = strlen(dirent->dName);
        s32 sl = gfSuffixLength;
        if (!dir && (nl < sl || !stricmp(&dirent->dName[nl - sl], gfSuffix))) {
            continue;
        }
        struct DirEntry entry;
        strcpy(entry.name, dirent->dName);
        entry.dir = dir;
        if (strcmp(dirent->dName, "..") == 0) {
            entry.tile = 0;
            strcpy(entry.text, "back");
        } else {
            if (entry.dir) {
                entry.tile = 1;
            } else {
                entry.tile = 2;
            }
            memcpy(entry.text, dirent->dName, 32);
            if (nl > 31) {
                strcpy(&entry.text[28], "...");
            }
        }
        entry.animState = 0;
        set_insert(&gfDirEntries, &entry);
    }
    closedir(dir);
    return TRUE;
}

static void updateView(bool enable, bool select) {
    if (enable) {
        s32 y = 3;
        if (gfMode == GETFILE_SAVE || gfMode == GETFILE_SAVE_PREFIX_INC) {
            menuItemEnable(gfName);
            menuItemEnable(gfMkdir);
            menuItemEnable(gfAccept);
            menuItemEnable(gfClear);
            gfName->y = y++;
            gfAccept->y = gfClear->y = y++;
        } else if (gfMode == GETFILE_LOAD) {
            menuItemDisable(gfName);
            menuItemDisable(gfMkdir);
            menuItemDisable(gfAccept);
            menuItemDisable(gfClear);
        }
        menuItemEnable(gfScrollUp);
        menuItemEnable(gfScrollDown);
        gfScrollUp->y = y;
        gfScrollDown->y = y + FILE_VIEW_ROWS - 1;
        struct DirState *ds = vector_at(&gfDirState, 0);
        bool selected = FALSE;
        for (s32 i = 0; i < FILE_VIEW_ROWS; ++i) {
            s32 index = ds->scroll + i;
            struct MenuItem *item = gfFiles[i];
            if (index < gfDirEntries.container.size) {
                item->y = y++;
                menuItemEnable(item);
                if (select && index == ds->index) {
                    menuSelect(&gfMenu, item);
                    selected = TRUE;
                }
            } else {
                menuItemDisable(item);
            }
        }
        if (select && !selected) {
            if (gfMode == GETFILE_LOAD) {
                menuSelect(&gfMenu, gfReset);
            } else {
                menuSelect(&gfMenu, gfName);
            }
        }
    } else {
        menuItemDisable(gfName);
        menuItemDisable(gfMkdir);
        menuItemDisable(gfAccept);
        menuItemDisable(gfClear);
        menuItemDisable(gfScrollUp);
        menuItemDisable(gfScrollDown);
        for (s32 i = 0; i < FILE_VIEW_ROWS; ++i) {
            menuItemDisable(gfFiles[i]);
        }
        if (select) {
            menuSelect(&gfMenu, gfReset);
        }
    }
}

static s32 getNextPrefixNumber(void) {
    DIR *dir = opendir(".");
    if (!dir) {
        return 0;
    }

    s32 maxNumFound = -1;
    s32 sl = strlen(gfSuffix);

    /* enumerate entries */
    struct Dirent *dirent;
    while ((dirent = readdir(dir))) {
        if (!(dirent->mode & S_IRUSR)) {
            continue;
        }
        s32 nl = strlen(dirent->dName);
        if (nl < sl || !stricmp(&dirent->dName[nl - sl], gfSuffix)) {
            continue;
        }

        s32 curNum;
        s32 ret = sscanf(dirent->dName, "%ld", &curNum);
        if (ret == EOF || ret < 1) {
            continue;
        }
        if (curNum > maxNumFound) {
            maxNumFound = curNum;
        }
    }

    closedir(dir);
    return maxNumFound + 1;
}

static void setName(const char *name, bool dirty) {
    gfDirtyName |= dirty;
    if (!name || strlen(name) == 0) {
        strcpy(gfName->text, "untitled");
        gfUntitled = TRUE;
    } else {
        if (gfDirtyName || gfMode != GETFILE_SAVE_PREFIX_INC) {
            strncpy(gfName->text, name, 31);
        } else {
            s32 ignore, prefixLength;
            sscanf(name, "%ld%ln", &ignore, &prefixLength);
            s32 prefix = getNextPrefixNumber();
            snprintf(gfName->text, 32, "%03ld%s", prefix, name + prefixLength);
        }
        gfName->text[31] = 0;
        gfUntitled = FALSE;
    }
}

static s32 overwritePromptProc(s32 optionIndex, void *data) {
    char *path = data;
    if (optionIndex != -1) {
        menuReturn(&gfMenu);
        if (optionIndex == 0 && !gfCallbackProc(path, gfCallbackData)) {
            menuReturn(&gfMenu);
        }
    }
    free(path);
    return 1;
}

static void returnPath(const char *name) {
    char *path = malloc(PATH_MAX);
    if (getcwd(path, PATH_MAX)) {
        s32 dl = strlen(path);
        s32 nl = strlen(name);
        if (dl + 1 + nl + gfSuffixLength < PATH_MAX) {
            path[dl] = '/';
            strcpy(&path[dl + 1], name);
            strcpy(&path[dl + 1 + nl], gfSuffix);
            if ((gfMode == GETFILE_SAVE || gfMode == GETFILE_SAVE_PREFIX_INC) && stat(path, NULL) == 0) {
                char prompt[48];
                sprintf(prompt, "'%.31s' exists", name);
                menuPrompt(&gfMenu, prompt,
                           "overwrite\0"
                           "cancel\0",
                           1, overwritePromptProc, path);
                return;
            }
            if (!gfCallbackProc(path, gfCallbackData)) {
                menuReturn(&gfMenu);
            }
        }
    }
    free(path);
}

static s32 fileEnterProc(struct MenuItem *item, enum MenuSwitchReason reason) {
    s32 row = (s32)item->data;
    struct DirState *ds = vector_at(&gfDirState, 0);
    s32 index = ds->scroll + row;
    if (index < gfDirEntries.container.size) {
        struct DirEntry *entry = set_at(&gfDirEntries, index);
        entry->animState = 0;
    }
    return 0;
}

static s32 fileDrawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    s32 row = (s32)item->data;
    struct DirState *ds = vector_at(&gfDirState, 0);
    struct DirEntry *entry = set_at(&gfDirEntries, ds->scroll + row);
    if (entry->animState > 0) {
        ++drawParams->x;
        ++drawParams->y;
        entry->animState = (entry->animState + 1) % 3;
    }
    s32 cw = menuGetCellWidth(item->owner, TRUE);
    struct GfxTexture *texture = resourceGet(RES_ICON_FILE);
    struct GfxSprite sprite = {
        texture,
        entry->tile,
        0,
        drawParams->x + (cw - texture->tileWidth) / 2,
        drawParams->y - (gfxFontXheight(drawParams->font) + texture->tileHeight + 1) / 2,
        1.f,
        1.f,
    };
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGBA8888(0xFF, 0xFF, 0xFF, drawParams->alpha));
    gfxSpriteDraw(&sprite);
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
    gfxPrintf(drawParams->font, drawParams->x + cw * 2, drawParams->y, "%s", entry->text);
    return 1;
}

static s32 fileActivateProc(struct MenuItem *item) {
    s32 row = (s32)item->data;
    struct DirState *ds = vector_at(&gfDirState, 0);
    s32 index = ds->scroll + row;
    struct DirEntry *entry = set_at(&gfDirEntries, index);
    entry->animState = 1;
    if (entry->dir) {
        if (chdir(entry->name)) {
            return 0;
        }
        if (strcmp(entry->name, "..") == 0) {
            vector_erase(&gfDirState, 0, 1);
        } else {
            struct DirState *ds = vector_at(&gfDirState, 0);
            ds->index = index;
            ds = vector_insert(&gfDirState, 0, 1, NULL);
            ds->scroll = 0;
            ds->index = 0;
        }

        /* Update the prefix */
        if (gfMode == GETFILE_SAVE_PREFIX_INC) {
            setName(gfName->text, FALSE);
        }

        updateView(updateList(), TRUE);
    } else {
        struct DirState *ds = vector_at(&gfDirState, 0);
        ds->index = index;
        s32 l = strlen(entry->name) - gfSuffixLength;
        char *name = malloc(l + 1);
        memcpy(name, entry->name, l);
        name[l] = 0;
        if (gfMode == GETFILE_SAVE || gfMode == GETFILE_SAVE_PREFIX_INC) {
            setName(name, TRUE);
            menuSelect(&gfMenu, gfAccept);
        } else {
            returnPath(name);
        }
        free(name);
    }
    return 1;
}

static s32 fileNavProc(struct MenuItem *item, enum MenuNavigation nav) {
    s32 row = (s32)item->data;
    s32 nEntries = gfDirEntries.container.size;
    if (row == 0 && nav == MENU_NAVIGATE_UP) {
        struct DirState *ds = vector_at(&gfDirState, 0);
        --ds->scroll;
        if (ds->scroll + FILE_VIEW_ROWS > nEntries) {
            ds->scroll = nEntries - FILE_VIEW_ROWS;
        }
        if (ds->scroll < 0) {
            if (nEntries < FILE_VIEW_ROWS) {
                ds->scroll = 0;
                menuSelect(item->owner, gfFiles[nEntries - 1]);
            } else {
                ds->scroll = nEntries - FILE_VIEW_ROWS;
                menuSelect(item->owner, gfFiles[FILE_VIEW_ROWS - 1]);
            }
        }
        return 1;
    } else if ((row == FILE_VIEW_ROWS - 1 || row == gfDirEntries.container.size - 1) && nav == MENU_NAVIGATE_DOWN) {
        struct DirState *ds = vector_at(&gfDirState, 0);
        ++ds->scroll;
        s32 index = ds->scroll + row;
        if (index == gfDirEntries.container.size) {
            ds->scroll = 0;
            menuSelect(item->owner, gfFiles[0]);
        }
        return 1;
    }
    return 0;
}

static bool dirEntryComp(void *a, void *b) {
    struct DirEntry *da = a;
    struct DirEntry *db = b;
    if (strcmp(da->name, "..") == 0) {
        return TRUE;
    }
    if (da->dir && !db->dir) {
        return TRUE;
    }
    if (!da->dir && db->dir) {
        return FALSE;
    }
    char *sa = da->name;
    char *sb = db->name;
    s32 d;
    while (*sa && *sb) {
        char ca = *sa++;
        char cb = *sb++;
        /* number comparison */
        if (ca >= '0' && ca <= '9' && cb >= '0' && cb <= '9') {
            char *na = sa - 1;
            char *nb = sb - 1;
            while (*na == '0') {
                ++na;
            }
            while (*nb == '0') {
                ++nb;
            }
            while (*sa >= '0' && *sa <= '9') {
                ++sa;
            }
            while (*sb >= '0' && *sb <= '9') {
                ++sb;
            }
            d = (sa - na) - (sb - nb);
            if (d < 0) {
                return TRUE;
            }
            if (d > 0) {
                return FALSE;
            }
            while (na != sa && nb != sb) {
                d = *na++ - *nb++;
                if (d < 0) {
                    return TRUE;
                }
                if (d > 0) {
                    return FALSE;
                }
            }
            continue;
        }
        /* case-insensitive character comparison */
        if (ca >= 'a' && ca <= 'z') {
            ca += 'A' - 'a';
        }
        if (cb >= 'a' && cb <= 'z') {
            cb += 'A' - 'a';
        }
        d = ca - cb;
        if (d < 0) {
            return TRUE;
        }
        if (d > 0) {
            return FALSE;
        }
    }
    /* length comparison */
    while (*sa) {
        ++sa;
    }
    while (*sb) {
        ++sb;
    }
    d = (sa - da->name) - (sb - db->name);
    if (d < 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static s32 oskCallbackProc(const char *str, void *data) {
    setName(str, strcmp(str, gfName->text) != FALSE);
    gfMenu.selector = gfAccept;
    return 0;
}

static s32 nameActivateProc(struct MenuItem *item) {
    menuGetOskString(item->owner, gfUntitled ? NULL : item->text, oskCallbackProc, NULL);
    return 1;
}

static void acceptProc(struct MenuItem *item, void *data) {
    returnPath(gfName->text);
}

static void clearProc(struct MenuItem *item, void *data) {
    setName(NULL, TRUE);
}

static void resetProc(struct MenuItem *item, void *data) {
    sysReset();
    vector_clear(&gfDirState);
    struct DirState *ds = vector_insert(&gfDirState, 0, 1, NULL);
    ds->scroll = 0;
    ds->index = 0;
    updateView(updateList(), TRUE);
}

static void scrollUpProc(struct MenuItem *item, void *data) {
    struct DirState *ds = vector_at(&gfDirState, 0);
    --ds->scroll;
    if (ds->scroll < 0) {
        ds->scroll = 0;
    }
}

static void scrollDownProc(struct MenuItem *item, void *data) {
    struct DirState *ds = vector_at(&gfDirState, 0);
    ++ds->scroll;
    s32 nEntries = gfDirEntries.container.size;
    if (ds->scroll + FILE_VIEW_ROWS > nEntries) {
        ds->scroll = nEntries - FILE_VIEW_ROWS;
    }
    if (ds->scroll < 0) {
        ds->scroll = 0;
    }
}

static s32 mkdirOskCallbackProc(const char *str, void *data) {
    if (*str == '\0') {
        return 0;
    } else if (mkdir(str, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) {
        menuPrompt(&gfMenu, strerror(errno), "return\0", 0, NULL, NULL);
        return 1;
    } else {
        menuReturn(&gfMenu);
        updateView(updateList(), TRUE);
        return 1;
    }
}

static void mkdirProc(struct MenuItem *item, void *data) {
    menuGetOskString(item->owner, NULL, mkdirOskCallbackProc, NULL);
}

static void gfMenuInit(void) {
    static bool ready = FALSE;
    if (!ready) {
        ready = TRUE;
        /* initialize data */
        vector_init(&gfDirState, sizeof(struct DirState));
        set_init(&gfDirEntries, sizeof(struct DirEntry), dirEntryComp);
        struct DirState *ds = vector_insert(&gfDirState, 0, 1, NULL);
        struct GfxTexture *fileIcons = resourceGet(RES_ICON_FILE);
        ds->scroll = 0;
        ds->index = 0;
        /* initialize menus */
        struct Menu *menu = &gfMenu;
        menuInit(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
        menu->selector = menuAddSubmenu(menu, 0, 0, NULL, "return");
        gfReset = menuAddButton(menu, 0, 1, "reset disk", resetProc, NULL);
        gfLocation = menuAddStatic(menu, 0, 2, NULL, 0xC0C0C0);
        gfLocation->text = malloc(32);
        gfMkdir = menuAddButtonIcon(menu, 0, 3, fileIcons, 3, 0, 0xFFFFFF, 1.0f, mkdirProc, NULL);
        gfName = menuItemAdd(menu, 2, 2, NULL, 0xFFFFFF);
        gfName->text = malloc(32);
        gfName->text[0] = 0;
        gfName->activateProc = nameActivateProc;
        gfAccept = menuAddButton(menu, 0, 4, "accept", acceptProc, NULL);
        gfClear = menuAddButton(menu, 7, 4, "clear", clearProc, NULL);
        for (s32 i = 0; i < FILE_VIEW_ROWS; ++i) {
            struct MenuItem *item = menuItemAdd(menu, 2, 5 + i, NULL, 0xFFFFFF);
            item->data = (void *)i;
            item->enterProc = fileEnterProc;
            item->drawProc = fileDrawProc;
            item->activateProc = fileActivateProc;
            item->navigateProc = fileNavProc;
            gfFiles[i] = item;
        }
        struct GfxTexture *tArrow = resourceGet(RES_ICON_ARROW);
        gfScrollUp = menuAddButtonIcon(menu, 0, 5, tArrow, 0, 0, 0xFFFFFF, 1.0f, scrollUpProc, NULL);
        gfScrollDown =
            menuAddButtonIcon(menu, 0, 5 + FILE_VIEW_ROWS - 1, tArrow, 1, 0, 0xFFFFFF, 1.0f, scrollDownProc, NULL);
    }
    updateView(updateList(), TRUE);
}

void menuGetFile(struct Menu *menu, enum GetFileMode mode, const char *defname, const char *suffix,
                 GetFileCallback callbackProc, void *callbackData) {
    gfMode = mode;
    gfDirtyName = FALSE;
    if (gfSuffix) {
        free(gfSuffix);
    }
    gfSuffix = malloc(strlen(suffix) + 1);
    strcpy(gfSuffix, suffix);
    gfSuffixLength = strlen(gfSuffix);
    gfCallbackProc = callbackProc;
    gfCallbackData = callbackData;
    gfMenuInit();
    setName(defname, FALSE);
    menuEnter(menu, &gfMenu);
}
