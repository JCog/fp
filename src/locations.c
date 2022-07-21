#include "common.h"
#include "fp.h"
#include "menu.h"
#include "warp_info.h"

static u16 area;
static u16 map;
static u16 entrance;

static void areaPrevProc(struct MenuItem *item, void *data) {
    if (area == 0) {
        area = AREA_COUNT - 1;
    } else {
        area--;
    }

    return;
}

static void areaNextProc(struct MenuItem *item, void *data) {
    if (area == AREA_COUNT - 1) {
        area = 0;
    } else {
        area++;
    }

    return;
}

static void mapPrevProc(struct MenuItem *item, void *data) {
    if (map == 0) {
        map = areaInfoList[area]->mapCount - 1;
    } else {
        map--;
    }

    return;
}

static void mapNextProc(struct MenuItem *item, void *data) {
    if (map == areaInfoList[area]->mapCount - 1) {
        map = 0;
    } else {
        map++;
    }

    return;
}

static void entrancePrevProc(struct MenuItem *item, void *data) {
    if (entrance == 0) {
        entrance = areaInfoList[area]->maps[map].entranceCount - 1;
    } else {
        entrance--;
    }

    return;
}

static void entranceNextProc(struct MenuItem *item, void *data) {
    if (entrance == areaInfoList[area]->maps[map].entranceCount - 1) {
        entrance = 0;
    } else {
        entrance++;
    }

    return;
}

static s32 warpInfoDrawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
    struct GfxFont *font = drawParams->font;
    s32 ch = menuGetCellHeight(item->owner, TRUE);
    s32 x = drawParams->x;
    s32 y = drawParams->y;

    if (map >= areaInfoList[area]->mapCount) {
        map = 0;
    }
    if (entrance >= areaInfoList[area]->maps[map].entranceCount) {
        entrance = 0;
    }

    gfxPrintf(font, x, y + ch * 0, "%x %s", area, areaInfoList[area]->areaName);
    gfxPrintf(font, x, y + ch * 3, "%x %s", map, areaInfoList[area]->maps[map].mapName);
    gfxPrintf(font, x, y + ch * 6, "%d/%d", entrance, areaInfoList[area]->maps[map].entranceCount);

    return 1;
}

static s32 currentMapDrawProc(struct MenuItem *item, struct MenuDrawParams *drawParams) {
    gfxModeSet(GFX_MODE_COLOR, GPACK_RGB24A8(drawParams->color, drawParams->alpha));
    struct GfxFont *font = drawParams->font;
    s32 ch = menuGetCellHeight(item->owner, TRUE);
    s32 x = drawParams->x;
    s32 y = drawParams->y;
    u16 areaID = pm_gGameStatus.areaID;
    u16 mapID = pm_gGameStatus.mapID;
    char **mapName = &pm_gAreas[areaID].maps[mapID].id;
    gfxPrintf(font, x, y + ch * 0, "current map");
    gfxPrintf(font, x, y + ch * 1, "a: %x %s", areaID, areaInfoList[areaID]->areaName);
    gfxPrintf(font, x, y + ch * 2, "m: %x %s", mapID, areaInfoList[areaID]->maps[mapID].mapName);
    gfxPrintf(font, x, y + ch * 3, "e: %x", pm_gGameStatus.entryID);
    gfxPrintf(font, x, y + ch * 5, "%s", *mapName);

    return 1;
}

static void warpProc(struct MenuItem *item, void *data) {
    if (fpWarp(area, map, entrance)) {
        fp.savedArea = area;
        fp.savedMap = map;
        fp.savedEntrance = entrance;
    }
}

void createLocationsMenu(struct Menu *menu) {

    /* initialize menu */
    menuInit(menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);
    menu->selector = menuAddSubmenu(menu, 0, 0, NULL, "return");

    menuAddStatic(menu, 0, 1, "area:", 0xC0C0C0);
    menuAddButton(menu, 0, 2, "<", areaPrevProc, NULL);
    menuAddButton(menu, 1, 2, ">", areaNextProc, NULL);

    menuAddStatic(menu, 0, 4, "map:", 0xC0C0C0);
    menuAddButton(menu, 0, 5, "<", mapPrevProc, NULL);
    menuAddButton(menu, 1, 5, ">", mapNextProc, NULL);

    menuAddStatic(menu, 0, 7, "entrance:", 0xC0C0C0);
    menuAddButton(menu, 0, 8, "<", entrancePrevProc, NULL);
    menuAddButton(menu, 1, 8, ">", entranceNextProc, NULL);

    menuAddStaticCustom(menu, 3, 2, warpInfoDrawProc, NULL, 0xC0C0C0);

    menuAddButton(menu, 0, 9, "warp", warpProc, NULL);

    menuAddStaticCustom(menu, 0, 12, currentMapDrawProc, NULL, 0xC0C0C0);
}
