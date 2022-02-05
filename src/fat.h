#ifndef FAT_H
#define FAT_H
#include <stdint.h>
#include <time.h>
#include <list/list.h>
#include "pm64.h"

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

enum fat_type {
    FAT12,
    FAT16,
    FAT32,
};

enum fat_rw {
    FAT_READ,
    FAT_WRITE,
};

/* block cache */
struct fat_cache {
    _Bool valid;
    _Bool dirty;
    u32 max_lba;
    u32 load_lba;
    u32 prep_lba;
    int n_sect;
    _Alignas(0x10) char data[0x200 * FAT_MAX_CACHE_SECT];
};

typedef int (*fat_rd_proc)(size_t lba, size_t n_block, void *buf);
typedef int (*fat_wr_proc)(size_t lba, size_t n_block, const void *buf);

/* fat context */
struct fat {
    /* block io interface */
    fat_rd_proc read;
    fat_wr_proc write;
    /* file system info */
    enum fat_type type;
    u32 part_lba;
    u32 n_part_sect;
    u16 n_sect_byte;
    u8 n_clust_sect;
    u16 n_resv_sect;
    u8 n_fat;
    u16 n_entry;
    u32 n_fs_sect;
    u32 n_fat_sect;
    u32 root_clust;
    u16 fsis_lba;
    u32 fat_lba;
    u32 root_lba;
    u32 data_lba;
    u32 n_clust_byte;
    u32 max_clust;
    u32 free_lb;
    /* cache */
    struct fat_cache cache[FAT_CACHE_MAX];
};

/* file pointer */
struct fat_file {
    struct fat *fat;
    /* file info */
    u32 clust;
    u32 size;
    _Bool is_dir;
    /* file offset */
    u32 p_off;
    /* file system geometry pointers */
    u32 p_clust;
    u32 p_clust_seq;
    u32 p_clust_sect;
    u32 p_sect_off;
};

/* canonical path */
struct fat_path {
    struct list ent_list;
};

/* directory entry */
struct fat_entry {
    struct fat *fat;
    /* pointer to first physical entry (sfn entry or start of lfn chain) */
    struct fat_file first;
    /* pointer to last physical entry (sfn entry) */
    struct fat_file last;
    /* sfn */
    char short_name[13];
    /* lfn or case-adjusted sfn */
    char name[256];
    /* metadata */
    time_t ctime;
    int cms;
    time_t atime;
    time_t mtime;
    u8 attrib;
    u32 clust;
    u32 size;
};

void fat_root(struct fat *fat, struct fat_file *file);
void fat_begin(struct fat_entry *entry, struct fat_file *file);
void fat_rewind(struct fat_file *file);
u32 fat_advance(struct fat_file *file, u32 n_byte, _Bool *eof);
u32 fat_rw(struct fat_file *file, enum fat_rw rw, void *buf, u32 n_byte, struct fat_file *new_file, _Bool *eof);
int fat_dir(struct fat_file *dir, struct fat_entry *entry);
int fat_find(struct fat *fat, struct fat_entry *dir, const char *path, struct fat_entry *entry);
struct fat_path *fat_path(struct fat *fat, struct fat_path *dir_fp, const char *path, const char **tail);
struct fat_entry *fat_path_target(struct fat_path *fp);
struct fat_entry *fat_path_dir(struct fat_path *fp);
void fat_free(struct fat_path *ptr);
int fat_create(struct fat *fat, struct fat_entry *dir, const char *path, u8 attrib, struct fat_entry *entry);
struct fat_path *fat_create_path(struct fat *fat, struct fat_path *dir_fp, const char *path, u8 attrib);
int fat_resize(struct fat_entry *entry, u32 size, struct fat_file *file);
int fat_empty(struct fat *fat, struct fat_entry *dir);
int fat_rename(struct fat *fat, struct fat_path *entry_fp, struct fat_path *dir_fp, const char *path,
               struct fat_entry *new_entry);
int fat_remove(struct fat_entry *entry);
int fat_attrib(struct fat_entry *entry, u8 attrib);
int fat_atime(struct fat_entry *entry, time_t timeval);
int fat_mtime(struct fat_entry *entry, time_t timeval);
int fat_init(struct fat *fat, fat_rd_proc read, fat_wr_proc write, u32 rec_lba, int part);
int fat_flush(struct fat *fat);

#endif
