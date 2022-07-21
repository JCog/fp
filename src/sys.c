#include "sys.h"
#include "fat.h"
#include "io.h"
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

struct Desc {
    int fildes;
    struct FatPath *fp;
    struct FatFile file;
    int flags;
};

struct FileDesc {
    struct Desc desc;
    u32 pos;
};

struct DirDesc {
    struct Desc desc;
    long pos;
    struct Dirent dirent;
};

// NOLINTBEGIN
__attribute__((used)) __attribute__((noinline)) void __assert_func(const char *file, int line, const char *func,
                                                                   const char *failedexpr) {}
// NOLINTEND

static bool fatReady = FALSE;
static struct Fat fat;
static void *descList[OPEN_MAX] = {NULL};
static struct FatPath *wd = NULL;

static int initFat(void) {
    if (fatReady) {
        return 0;
    }
    if (diskInit()) {
        return -1;
    }
    if (fatInit(&fat, diskRead, diskWrite, 0, 0)) {
        return -1;
    }
    wd = fatPath(&fat, NULL, "", NULL);
    if (!wd) {
        return -1;
    }
    fatReady = TRUE;
    return 0;
}

static struct FatPath *getOrigin(const char *path, const char **tail) {
    if (path[0] == '/' || path[0] == '\\') {
        if (tail) {
            *tail = &path[1];
        }
        return NULL;
    } else {
        if (tail) {
            *tail = path;
        }
        return wd;
    }
}

static int wdFind(const char *path, struct FatEntry *entry) {
    const char *tail;
    struct FatPath *origin = getOrigin(path, &tail);
    return fatFind(&fat, fatPathTarget(origin), tail, entry);
}

static struct FatPath *wdPath(const char *path) {
    const char *tail;
    struct FatPath *origin = getOrigin(path, &tail);
    return fatPath(&fat, origin, tail, NULL);
}

static ino_t makeSn(struct FatEntry *entry) {
    if ((entry->attrib & FAT_ATTRIB_DIRECTORY) && entry->clust < 2) {
        return fat.partLba;
    }
    return fat.partLba + 1 + entry->last.clust * (fat.nClustByte / 0x20) + entry->last.pOff / 0x20;
}

static int checkPath(ino_t sn, struct FatPath *fp) {
    for (struct FatEntry *pEnt = fp->entList.first; pEnt; pEnt = list_next(pEnt)) {
        if (sn == makeSn(pEnt)) {
            errno = EACCES;
            return -1;
        }
    }
    return 0;
}

static int entAccess(struct FatEntry *entry, bool write) {
    ino_t sn = makeSn(entry);
    if (write && checkPath(sn, wd)) {
        return -1;
    }
    for (int i = 0; i < FOPEN_MAX; ++i) {
        struct Desc *desc = descList[i];
        if (!desc) {
            continue;
        }
        if (write && checkPath(sn, desc->fp)) {
            return -1;
        }
        if ((desc->flags & _FWRITE) && sn == makeSn(fatPathTarget(desc->fp))) {
            errno = EACCES;
            return -1;
        }
    }
    return 0;
}

static void *newDesc(size_t size, struct FatPath *fp, int flags) {
    for (int i = 0; i < OPEN_MAX; ++i) {
        if (!descList[i]) {
            struct Desc *desc = malloc(size);
            if (!desc) {
                errno = ENOMEM;
                return NULL;
            }
            desc->fildes = i;
            desc->fp = fp;
            fatBegin(fatPathTarget(fp), &desc->file);
            desc->flags = flags;
            descList[i] = desc;
            return desc;
        }
    }
    errno = EMFILE;
    return NULL;
}

static void deleteDesc(int fildes) {
    struct Desc *desc = descList[fildes];
    fatFree(desc->fp);
    free(desc);
    descList[fildes] = NULL;
}

static void *getDesc(int fildes) {
    if (fildes < 0 || fildes >= OPEN_MAX) {
        errno = EBADF;
        return NULL;
    }
    void *desc = descList[fildes];
    if (!desc) {
        errno = EBADF;
        return NULL;
    }
    return desc;
}

static int seekFile(struct FileDesc *fdesc) {
    struct FatFile *file = &fdesc->desc.file;
    if (fdesc->pos == file->pOff) {
        return 0;
    }
    if (fdesc->pos < file->pOff) {
        fatRewind(file);
    }
    int e = errno;
    errno = 0;
    fatAdvance(file, fdesc->pos - file->pOff, NULL);
    if (errno != 0) {
        return -1;
    }
    errno = e;
    return 0;
}

static mode_t entMode(struct FatEntry *entry) {
    mode_t mode;
    if (entry->attrib & FAT_ATTRIB_DIRECTORY) {
        mode = S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
    } else {
        mode = S_IFREG;
    }
    if (!(entry->attrib & FAT_ATTRIB_HIDDEN)) {
        mode |= S_IRUSR | S_IRGRP | S_IROTH;
    }
    if (!(entry->attrib & FAT_ATTRIB_READONLY)) {
        mode |= S_IWUSR;
    }
    return mode;
}

static void entStat(struct FatEntry *entry, struct stat *buf) {
    buf->st_dev = 0;
    buf->st_ino = makeSn(entry);
    buf->st_mode = entMode(entry);
    buf->st_nlink = 1;
    buf->st_uid = 0;
    buf->st_gid = 0;
    buf->st_size = entry->size;
    buf->st_atime = entry->atime;
    buf->st_mtime = entry->mtime;
    buf->st_ctime = entry->ctime;
    buf->st_blksize = fat.nClustByte;
    buf->st_blocks = (entry->size + fat.nClustByte - 1) / fat.nClustByte;
}

int open(const char *path, int oflags, ...) {
    if (initFat()) {
        return -1;
    }
    /* find/create file */
    int flags = oflags + 1;
    const char *tail;
    struct FatPath *origin = getOrigin(path, &tail);
    int e = errno;
    errno = 0;
    struct FatPath *fp = fatPath(&fat, origin, tail, NULL);
    struct FatEntry *entry;
    if (errno == 0) {
        entry = fatPathTarget(fp);
        if ((oflags & O_CREAT) && (oflags & O_EXCL)) {
            errno = EEXIST;
            goto error;
        }
        if ((entry->attrib & FAT_ATTRIB_DIRECTORY) && (flags & _FWRITE)) {
            errno = EISDIR;
            goto error;
        }
        if (entAccess(entry, flags & _FWRITE)) {
            goto error;
        }
        errno = e;
    } else {
        if (errno == ENOENT && (oflags & O_CREAT)) {
            va_list va;
            va_start(va, oflags);
            mode_t mode = va_arg(va, mode_t);
            va_end(va);
            u8 attrib = FAT_ATTRIB_ARCHIVE;
            if (!(mode & S_IRUSR)) {
                attrib |= FAT_ATTRIB_HIDDEN;
            }
            if (!(mode & S_IWUSR)) {
                attrib |= FAT_ATTRIB_READONLY;
            }
            if (fp) {
                fatFree(fp);
            }
            fp = fatCreatePath(&fat, origin, tail, attrib);
            if (!fp) {
                goto error;
            }
            entry = fatPathTarget(fp);
            errno = e;
        } else {
            goto error;
        }
    }
    /* clear file */
    if ((oflags & O_TRUNC) && entry->size > 0) {
        if (fatResize(entry, 0, NULL)) {
            goto error;
        }
    }
    /* allocate and initialize file desc */
    struct FileDesc *fdesc = newDesc(sizeof(*fdesc), fp, flags);
    if (!fdesc) {
        goto error;
    }
    fdesc->pos = 0;
    return fdesc->desc.fildes;
error:
    if (fp) {
        fatFree(fp);
    }
    return -1;
}

int creat(const char *path, mode_t mode) {
    return open(path, O_WRONLY | O_CREAT | O_TRUNC, mode);
}

int fstat(int fildes, struct stat *buf) {
    struct Desc *desc = getDesc(fildes);
    if (!desc) {
        return -1;
    }
    entStat(fatPathTarget(desc->fp), buf);
    return 0;
}

int fstatat(int fildes, const char *path, struct stat *buf, int flag) {
    if (initFat()) {
        return -1;
    }
    struct FatEntry *dir;
    if (fildes == AT_FDCWD) {
        dir = fatPathTarget(wd);
    } else {
        struct Desc *desc = getDesc(fildes);
        if (!desc) {
            return -1;
        }
        dir = fatPathTarget(desc->fp);
    }
    struct FatEntry entry;
    if (fatFind(&fat, dir, path, &entry)) {
        return -1;
    }
    entStat(&entry, buf);
    return 0;
}

int isatty(int fildes) {
    if (getDesc(fildes)) {
        errno = ENOTTY;
    }
    return 0;
}

off_t lseek(int fildes, off_t offset, int whence) {
    struct FileDesc *fdesc = getDesc(fildes);
    if (!fdesc) {
        return -1;
    }
    if (whence == SEEK_SET) {
        fdesc->pos = offset;
    } else if (whence == SEEK_CUR) {
        fdesc->pos += offset;
    } else if (whence == SEEK_END) {
        fdesc->pos = fdesc->desc.file.size + offset;
    } else {
        errno = EINVAL;
        return -1;
    }
    return fdesc->pos;
}

int close(int fildes) {
    struct FileDesc *fdesc = getDesc(fildes);
    if (!fdesc) {
        return -1;
    }
    deleteDesc(fdesc->desc.fildes);
    /* flush data to disk */
    return fatFlush(&fat);
}

int read(int fildes, void *buf, unsigned int nbyte) {
    struct FileDesc *fdesc = getDesc(fildes);
    if (!fdesc) {
        return -1;
    }
    /* validate file desc */
    if (!(fdesc->desc.flags & _FREAD)) {
        errno = EBADF;
        return -1;
    }
    if (nbyte == 0) {
        return 0;
    }
    /* seek */
    if (seekFile(fdesc)) {
        return -1;
    }
    /* read data and advance pointer */
    u32 n = fatRw(&fdesc->desc.file, FAT_READ, buf, nbyte, &fdesc->desc.file, NULL);
    fdesc->pos += n;
    return n;
}

int write(int fildes, void *buf, unsigned int nbyte) {
    struct FileDesc *fdesc = getDesc(fildes);
    if (!fdesc) {
        return -1;
    }
    struct Desc *desc = &fdesc->desc;
    struct FatEntry *entry = fatPathTarget(desc->fp);
    /* validate file desc */
    if (!(desc->flags & _FWRITE)) {
        errno = EBADF;
        return -1;
    }
    if (nbyte == 0) {
        return 0;
    }
    /* seek to end if FAPPEND is set */
    if (desc->flags & _FAPPEND) {
        fdesc->pos = desc->file.size;
    }
    /* write zero padding if needed */
    if (fdesc->pos > desc->file.size) {
        u32 size = desc->file.size;
        /* resize file */
        if (fatResize(entry, fdesc->pos + nbyte, &desc->file)) {
            return -1;
        }
        /* seek and clear */
        u32 adv = size - desc->file.pOff;
        if (fatAdvance(&desc->file, adv, NULL) != adv) {
            return -1;
        }
        u32 n = fdesc->pos - size;
        if (fatRw(&desc->file, FAT_WRITE, NULL, n, &desc->file, NULL) != n) {
            return -1;
        }
    } else {
        /* resize file if needed */
        u32 newOff = fdesc->pos + nbyte;
        if (newOff > desc->file.size) {
            if (fatResize(entry, newOff, &desc->file)) {
                return -1;
            }
        }
        /* seek */
        if (seekFile(fdesc)) {
            return -1;
        }
    }
    /* write data and advance pointer */
    u32 n = fatRw(&desc->file, FAT_WRITE, buf, nbyte, &desc->file, NULL);
    fdesc->pos += n;
    return n;
}

int truncate(const char *path, off_t length) {
    if (initFat()) {
        return -1;
    }
    if (length < 0) {
        errno = EINVAL;
        return -1;
    }
    struct FatEntry entry;
    if (wdFind(path, &entry)) {
        return -1;
    }
    if (entry.attrib & FAT_ATTRIB_DIRECTORY) {
        errno = EISDIR;
        return -1;
    }
    if (entAccess(&entry, TRUE)) {
        return -1;
    }
    u32 size = entry.size;
    if (fatResize(&entry, length, NULL)) {
        return -1;
    }
    if (length > size) {
        struct FatFile file;
        fatBegin(&entry, &file);
        int e = errno;
        errno = 0;
        fatAdvance(&file, size, NULL);
        if (errno != 0) {
            return -1;
        }
        errno = e;
        u32 n = length - size;
        if (fatRw(&file, FAT_WRITE, NULL, n, NULL, NULL) != n) {
            return -1;
        }
    }
    return fatFlush(&fat);
}

int rename(const char *oldPath, const char *newPath) {
    if (initFat()) {
        return -1;
    }
    int e = errno;
    errno = 0;
    struct FatPath *fp = wdPath(oldPath);
    if (errno == 0) {
        entAccess(fatPathTarget(fp), TRUE);
    }
    int r = -1;
    if (fp) {
        if (errno == 0) {
            errno = e;
            const char *tail;
            struct FatPath *origin = getOrigin(newPath, &tail);
            r = fatRename(&fat, fp, origin, tail, NULL);
        }
        fatFree(fp);
    }
    if (r == 0) {
        return fatFlush(&fat);
    } else {
        return r;
    }
}

int chmod(const char *path, mode_t mode) {
    if (initFat()) {
        return -1;
    }
    struct FatEntry entry;
    if (wdFind(path, &entry)) {
        return -1;
    }
    if (entAccess(&entry, TRUE)) {
        return -1;
    }
    u8 attrib = entry.attrib;
    if (mode & S_IRUSR) {
        attrib &= ~FAT_ATTRIB_HIDDEN;
    } else {
        attrib |= FAT_ATTRIB_HIDDEN;
    }
    if (mode & S_IWUSR) {
        attrib &= ~FAT_ATTRIB_READONLY;
    } else {
        attrib |= FAT_ATTRIB_READONLY;
    }
    if (fatAttrib(&entry, attrib)) {
        return -1;
    }
    return fatFlush(&fat);
}

int unlink(const char *path) {
    if (initFat()) {
        return -1;
    }
    struct FatEntry entry;
    if (wdFind(path, &entry)) {
        return -1;
    }
    if (entry.attrib & FAT_ATTRIB_DIRECTORY) {
        errno = EISDIR;
        return -1;
    }
    if (entAccess(&entry, TRUE)) {
        return -1;
    }
    if (fatRemove(&entry)) {
        return -1;
    }
    return fatFlush(&fat);
}

DIR *opendir(const char *dirname) {
    if (initFat()) {
        return NULL;
    }
    /* find directory */
    int e = errno;
    errno = 0;
    struct FatPath *fp = wdPath(dirname);
    if (errno == 0) {
        errno = e;
    } else {
        goto error;
    }
    struct FatEntry *entry = fatPathTarget(fp);
    if (!(entry->attrib & FAT_ATTRIB_DIRECTORY)) {
        errno = ENOTDIR;
        goto error;
    }
    if (entAccess(entry, FALSE)) {
        goto error;
    }
    /* allocate and initialize dir desc */
    struct DirDesc *ddesc = newDesc(sizeof(*ddesc), fp, _FREAD);
    if (!ddesc) {
        goto error;
    }
    ddesc->pos = 0;
    return (void *)ddesc;
error:
    if (fp) {
        fatFree(fp);
    }
    return NULL;
}

int closedir(DIR *dirp) {
    struct DirDesc *ddesc = (void *)dirp;
    deleteDesc(ddesc->desc.fildes);
    return 0;
}

struct Dirent *readdir(DIR *dirp) {
    struct DirDesc *ddesc = (void *)dirp;
    struct FatEntry entry;
    do {
        if (fatDir(&ddesc->desc.file, &entry)) {
            return NULL;
        }
    } while (entry.attrib & FAT_ATTRIB_LABEL);
    ++ddesc->pos;
    struct Dirent *dirent = &ddesc->dirent;
    dirent->dIno = makeSn(&entry);
    strcpy(dirent->dName, entry.name);
    /* extensions */
    dirent->mode = entMode(&entry);
    dirent->ctime = entry.ctime;
    dirent->mtime = entry.mtime;
    dirent->size = entry.size;
    return &ddesc->dirent;
}

void seekdir(DIR *dirp, long loc) {
    rewinddir(dirp);
    struct DirDesc *ddesc = (void *)dirp;
    for (long i = 0; i < loc; ++i) {
        struct FatEntry entry;
        do {
            if (fatDir(&ddesc->desc.file, &entry)) {
                return;
            }
        } while (entry.attrib & FAT_ATTRIB_LABEL);
        ++ddesc->pos;
    }
}

long telldir(DIR *dirp) {
    struct DirDesc *ddesc = (void *)dirp;
    return ddesc->pos;
}

void rewinddir(DIR *dirp) {
    struct DirDesc *ddesc = (void *)dirp;
    fatRewind(&ddesc->desc.file);
    ddesc->pos = 0;
}

int mkdir(const char *path, mode_t mode) {
    if (initFat()) {
        return -1;
    }
    u8 attrib = FAT_ATTRIB_DIRECTORY;
    if (!(mode & S_IRUSR)) {
        attrib |= FAT_ATTRIB_HIDDEN;
    }
    if (!(mode & S_IWUSR)) {
        attrib |= FAT_ATTRIB_READONLY;
    }
    const char *tail;
    struct FatPath *fp = getOrigin(path, &tail);
    if (fatCreate(&fat, fatPathTarget(fp), tail, attrib, NULL)) {
        return -1;
    }
    return fatFlush(&fat);
}

int rmdir(const char *path) {
    if (initFat()) {
        return -1;
    }
    struct FatEntry entry;
    if (wdFind(path, &entry)) {
        return -1;
    }
    if (!(entry.attrib & FAT_ATTRIB_DIRECTORY)) {
        errno = ENOTDIR;
        return -1;
    }
    if (entAccess(&entry, TRUE)) {
        return -1;
    }
    if (fatRemove(&entry)) {
        return -1;
    }
    return fatFlush(&fat);
}

int stat(const char *path, struct stat *buf) {
    if (initFat()) {
        return -1;
    }
    struct FatEntry entry;
    if (wdFind(path, &entry)) {
        return -1;
    }
    if (buf) {
        entStat(&entry, buf);
    }
    return 0;
}

int lstat(const char *path, struct stat *buf) {
    return stat(path, buf);
}

int chdir(const char *path) {
    if (initFat()) {
        return -1;
    }
    int e = errno;
    errno = 0;
    struct FatPath *fp = wdPath(path);
    if (errno == 0) {
        if (!(fatPathTarget(fp)->attrib & FAT_ATTRIB_DIRECTORY)) {
            errno = ENOTDIR;
            goto error;
        } else {
            errno = e;
        }
    } else {
        goto error;
    }
    fatFree(wd);
    wd = fp;
    return 0;
error:
    if (fp) {
        fatFree(fp);
    }
    return -1;
}

char *getcwd(char *buf, size_t size) {
    if (initFat()) {
        return NULL;
    }
    if (size == 0) {
        errno = EINVAL;
        return NULL;
    }
    char *p = buf;
    char *e = buf + size;
    for (struct FatEntry *pEnt = wd->entList.first; pEnt; pEnt = list_next(pEnt)) {
        char *n = pEnt->name;
        while (*n && p != e) {
            *p++ = *n++;
        }
        if ((pEnt == wd->entList.first || pEnt != wd->entList.last) && p != e) {
            *p++ = '/';
        }
        if (p == e) {
            errno = ERANGE;
            return NULL;
        }
    }
    *p = 0;
    return buf;
}

time_t time(time_t *tloc) {
    if (tloc) {
        *tloc = 0;
    }
    return 0;
}

void sysReset(void) {
    fatReady = FALSE;
    for (int i = 0; i < OPEN_MAX; ++i) {
        if (descList[i]) {
            deleteDesc(i);
        }
    }
    if (wd) {
        fatFree(wd);
        wd = 0;
    }
}
