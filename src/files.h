#ifndef FILE_H
#define FILE_H
#include "common.h"
#include "menu/menu.h"

enum GetFileMode {
    GETFILE_LOAD,
    GETFILE_SAVE,
    GETFILE_SAVE_PREFIX_INC,
};

typedef s32 (*GetFileCallback)(const char *path, void *data);

void menuGetFile(struct Menu *menu, enum GetFileMode mode, const char *defname, const char *suffix,
                 GetFileCallback callbackProc, void *callbackData);

#endif
