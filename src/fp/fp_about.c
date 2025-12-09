#include "fp.h"
#include "menu/menu.h"

struct Menu *createAboutMenu(void) {
    static struct Menu menu;
    menuInit(&menu, MENU_NOVALUE, MENU_NOVALUE, MENU_NOVALUE);

    s32 y = 0;
    menu.selector = menuAddSubmenu(&menu, 0, y++, NULL, "return");
    menuAddStatic(&menu, 0, y++, "fp version:", 0xC0C0C0);
    menuAddStatic(&menu, 0, y++, STRINGIFY(FP_VERSION), 0xFF0000);
    y++;
    menuAddStatic(&menu, 0, y++, "manual:", 0xC0C0C0);
    menuAddStatic(&menu, 0, y++, "https://fp-docs.starhaven.dev/", 0x99C3FF);
    y++;
    menuAddStatic(&menu, 0, y++, "github:", 0xC0C0C0);
    menuAddStatic(&menu, 0, y++, "https://github.com/JCog/fp", 0x99C3FF);
    s32 creditY = ++y;
    menuAddStatic(&menu, 0, y++, "contributors:", 0xC0C0C0);
    menuAddStatic(&menu, 0, y++, "JCog", 0xFFFFFF);
    menuAddStatic(&menu, 0, y++, "imaandrew", 0xFFFFFF);
    menuAddStatic(&menu, 0, y++, "fig02", 0xFFFFFF);
    menuAddStatic(&menu, 0, y++, "Rainchus", 0xFFFFFF);
    menuAddStatic(&menu, 0, y++, "rnadrich", 0xFFFFFF);
    y = creditY;
    s32 xOffset = 15;
    menuAddStatic(&menu, xOffset, y++, "special thanks:", 0xC0C0C0);
    menuAddStatic(&menu, xOffset, y++, "glankk", 0xFFFFFF);
    menuAddStatic(&menu, xOffset, y++, "krimtonz", 0xFFFFFF);

    return &menu;
}
