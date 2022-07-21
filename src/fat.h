#ifndef FAT_H
#define FAT_H
#include "common.h"
#include <list/list.h>
#include <sys/_timeval.h>

#define FAT_MAX_CACHE_SECT   4

#define FAT_CACHE_FAT        0
#define FAT_CACHE_DATA       1
#define FAT_CACHE_MAX        2

#define FAT_ATTRIB_DEFAULT   0x00
#define FAT_ATTRIB_READONLY  0x01
#define FAT_ATTRIB_HIDDEN    0x02
#define FAT_ATTRIB_SYSTEM    0x04
#define FAT_ATTRIB_LABEL     0x08
#define FAT_ATTRIB_DIRECTORY 0x10
#define FAT_ATTRIB_ARCHIVE   0x20
#define FAT_ATTRIB_DEVICE    0x40

enum FatType {
    FAT12,
    FAT16,
    FAT32,
};

enum FatRw {
    FAT_READ,
    FAT_WRITE,
};

/* block cache */
struct FatCache {
    bool valid;
    bool dirty;
    u32 maxLba;
    u32 loadLba;
    u32 prepLba;
    s32 nSect;
    _Alignas(0x10) char data[0x200 * FAT_MAX_CACHE_SECT];
};

typedef s32 (*FatRdProc)(size_t lba, size_t nBlock, void *buf);
typedef s32 (*FatWrProc)(size_t lba, size_t nBlock, const void *buf);

/* fat context */
struct Fat {
    /* block io interface */
    FatRdProc read;
    FatWrProc write;
    /* file system info */
    enum FatType type;
    u32 partLba;
    u32 nPartSect;
    u16 nSectByte;
    u8 nClustSect;
    u16 nResvSect;
    u8 nFat;
    u16 nEntry;
    u32 nFsSect;
    u32 nFatSect;
    u32 rootClust;
    u16 fsisLba;
    u32 fatLba;
    u32 rootLba;
    u32 dataLba;
    u32 nClustByte;
    u32 maxClust;
    u32 freeLb;
    /* cache */
    struct FatCache cache[FAT_CACHE_MAX];
};

/* file pointer */
struct FatFile {
    struct Fat *fat;
    /* file info */
    u32 clust;
    u32 size;
    bool isDir;
    /* file offset */
    u32 pOff;
    /* file system geometry pointers */
    u32 pClust;
    u32 pClustSeq;
    u32 pClustSect;
    u32 pSectOff;
};

/* canonical path */
struct FatPath {
    struct list entList;
};

/* directory entry */
struct FatEntry {
    struct Fat *fat;
    /* pointer to first physical entry (sfn entry or start of lfn chain) */
    struct FatFile first;
    /* pointer to last physical entry (sfn entry) */
    struct FatFile last;
    /* sfn */
    char shortName[13];
    /* lfn or case-adjusted sfn */
    char name[256];
    /* metadata */
    time_t ctime;
    s32 cms;
    time_t atime;
    time_t mtime;
    u8 attrib;
    u32 clust;
    u32 size;
};

void fatRoot(struct Fat *fat, struct FatFile *file);
void fatBegin(struct FatEntry *entry, struct FatFile *file);
void fatRewind(struct FatFile *file);
u32 fatAdvance(struct FatFile *file, u32 nByte, bool *eof);
u32 fatRw(struct FatFile *file, enum FatRw rw, void *buf, u32 nByte, struct FatFile *newFile, bool *eof);
s32 fatDir(struct FatFile *dir, struct FatEntry *entry);
s32 fatFind(struct Fat *fat, struct FatEntry *dir, const char *path, struct FatEntry *entry);
struct FatPath *fatPath(struct Fat *fat, struct FatPath *dirFp, const char *path, const char **tail);
struct FatEntry *fatPathTarget(struct FatPath *fp);
struct FatEntry *fatPathDir(struct FatPath *fp);
void fatFree(struct FatPath *ptr);
s32 fatCreate(struct Fat *fat, struct FatEntry *dir, const char *path, u8 attrib, struct FatEntry *entry);
struct FatPath *fatCreatePath(struct Fat *fat, struct FatPath *dirFp, const char *path, u8 attrib);
s32 fatResize(struct FatEntry *entry, u32 size, struct FatFile *file);
s32 fatEmpty(struct Fat *fat, struct FatEntry *dir);
s32 fatRename(struct Fat *fat, struct FatPath *entryFp, struct FatPath *dirFp, const char *path,
              struct FatEntry *newEntry);
s32 fatRemove(struct FatEntry *entry);
s32 fatAttrib(struct FatEntry *entry, u8 attrib);
s32 fatAtime(struct FatEntry *entry, time_t timeval);
s32 fatMtime(struct FatEntry *entry, time_t timeval);
s32 fatInit(struct Fat *fat, FatRdProc read, FatWrProc write, u32 recLba, s32 part);
s32 fatFlush(struct Fat *fat);

#endif
