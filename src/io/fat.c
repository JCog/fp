#include "fat.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
   string operations
*/

/* transform string to lower case */
static void cvtLower(char *s, s32 length) {
    while (length-- > 0) {
        if (*s >= 'A' && *s <= 'Z') {
            *s += 'a' - 'A';
        }
        ++s;
    }
}

/* transform string to upper case */
static void cvtUpper(char *s, s32 length) {
    while (length-- > 0) {
        if (*s >= 'a' && *s <= 'z') {
            *s += 'A' - 'a';
        }
        ++s;
    }
}

/* case-insensitive compare */
static bool nameComp(const char *a, const char *b) {
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
            return 0;
        }
    }
    return !*a && !*b;
}

/* compute the length of a string without trailing spaces and dots */
static size_t nameTrim(const char *s, size_t length) {
    while (length > 0 && (s[length - 1] == ' ' || s[length - 1] == '.')) {
        --length;
    }
    return length;
}

/* split a string into its name and extension components */
static void nameSplit(const char *s, const char **name, const char **ext, s32 *nameLength, s32 *extLength) {
    const char *end = s + nameTrim(s, strlen(s));
    const char *dot = NULL;
    for (const char *c = end - 1; c >= s; --c) {
        if (*c == '.') {
            dot = c;
            break;
        }
    }
    *name = s;
    if (dot) {
        *nameLength = dot - *name;
        *ext = dot + 1;
        *extLength = end - *ext;
    } else {
        *nameLength = end - *name;
        *ext = NULL;
        *extLength = 0;
    }
}

enum SfnCase {
    SFN_CASE_ANY,
    SFN_CASE_LOWER,
    SFN_CASE_UPPER,
};

/* check if a character is a valid sfn character in the given case */
static bool charIsSfn(char c, enum SfnCase *cse) {
    if (c >= 'A' && c <= 'Z') {
        if (cse) {
            if (*cse == SFN_CASE_LOWER) {
                return 0;
            } else if (*cse == SFN_CASE_ANY) {
                *cse = SFN_CASE_UPPER;
            }
        }
    }
    if (c >= 'a' && c <= 'z') {
        if (cse) {
            if (*cse == SFN_CASE_UPPER) {
                return 0;
            } else if (*cse == SFN_CASE_ANY) {
                *cse = SFN_CASE_LOWER;
            }
        }
        c += 'A' - 'a';
    }
    return (c >= '@' && c <= 'Z') || (c >= '0' && c <= '9') || (c >= '#' && c <= ')') || (c >= '^' && c <= '`') ||
           (c >= '\x80' && c <= '\xFF') || c == '!' || c == '-' || c == '{' || c == '}' || c == '~';
}

/* convert an lfn name component to sfn characters */
static s32 cvtSfn(const char *s, s32 sLength, char *buf, s32 bufSize) {
    s32 p = 0;
    for (s32 i = 0; i < sLength && p < bufSize; ++i) {
        char c = s[i];
        if (c >= 'a' && c <= 'z') {
            c += 'A' - 'a';
        } else if (c == '.' || c == ' ') {
            continue;
        } else if (!charIsSfn(c, NULL)) {
            c = '_';
        }
        buf[p++] = c;
    }
    s32 length = p;
    while (p < bufSize) {
        buf[p++] = ' ';
    }
    return length;
}

/* check if a name is a valid sfn,
   possibly with one or two lower case components */
static bool nameIsSfn(const char *s, bool *lowerName, bool *lowerExt) {
    if (strcmp(s, ".") == 0 || strcmp(s, "..") == 0) {
        if (lowerName) {
            *lowerName = FALSE;
        }
        if (lowerExt) {
            *lowerExt = FALSE;
        }
        return TRUE;
    }
    const char *name;
    const char *ext;
    s32 nameL;
    s32 extL;
    nameSplit(s, &name, &ext, &nameL, &extL);
    if (nameL == 0 || nameL > 8 || (ext && extL > 3)) {
        return FALSE;
    }
    enum SfnCase nameCse = SFN_CASE_ANY;
    enum SfnCase extCse = SFN_CASE_ANY;
    for (s32 i = 0; i < nameL; ++i) {
        if (!charIsSfn(name[i], &nameCse)) {
            return FALSE;
        }
    }
    for (s32 i = 0; i < extL; ++i) {
        if (!charIsSfn(ext[i], &extCse)) {
            return FALSE;
        }
    }
    if (lowerName) {
        *lowerName = (nameCse == SFN_CASE_LOWER);
    }
    if (lowerExt) {
        *lowerExt = (extCse == SFN_CASE_LOWER);
    }
    return TRUE;
}

/* translate an sfn from a directory entry to its normal form */
static s32 getSfn(const char *sfn, char *buf, s32 *nameL, s32 *extL) {
    s32 nl = 8;
    s32 el = 3;
    while (nl > 0 && sfn[nl - 1] == ' ') {
        --nl;
    }
    while (el > 0 && sfn[8 + el - 1] == ' ') {
        --el;
    }
    memcpy(&buf[0], &sfn[0], nl);
    s32 l = nl;
    if (el > 0) {
        buf[nl] = '.';
        memcpy(&buf[nl + 1], &sfn[8], el);
        l += 1 + el;
    }
    buf[l] = 0;
    if (nameL) {
        *nameL = nl;
    }
    if (extL) {
        *extL = el;
    }
    return l;
}

/* validate the name in a directory entry */
static bool validateSfn(const char *sfn) {
    /* check for dot entry */
    if (sfn[0] == '.') {
        for (s32 i = 2; i < 11; ++i) {
            if (sfn[i] != ' ') {
                return FALSE;
            }
        }
        return sfn[1] == '.' || sfn[1] == ' ';
    }
    /* validate characters */
    enum SfnCase cse = SFN_CASE_UPPER;
    bool spc = FALSE;
    for (s32 i = 0; i < 11; ++i) {
        if (i == 8) {
            spc = 0;
        }
        if (sfn[i] == ' ') {
            spc = TRUE;
        } else if (spc || !charIsSfn(sfn[i], &cse)) {
            return FALSE;
        }
    }
    return TRUE;
}

/* convert a name to 8.3 */
static void cvt83(const char *s, char *sfn) {
    memset(sfn, ' ', 11);
    if (strcmp(s, ".") == 0) {
        sfn[0] = '.';
    } else if (strcmp(s, "..") == 0) {
        sfn[0] = '.';
        sfn[1] = '.';
    } else {
        const char *name;
        const char *ext;
        s32 nameLength;
        s32 extLength;
        nameSplit(s, &name, &ext, &nameLength, &extLength);
        memcpy(&sfn[0], name, nameLength);
        memcpy(&sfn[8], ext, extLength);
        cvtUpper(sfn, 11);
        if (sfn[0] == '\xE5') {
            sfn[0] = '\x05';
        }
    }
}

/* compute the vfat checksum of an 8.3 name */
static u8 computeLfnChecksum(const char *sfn) {
    u8 *p = (void *)sfn;
    u8 checksum = 0;
    for (s32 i = 0; i < 11; ++i) {
        checksum = ((checksum & 1) << 7) + (checksum >> 1) + p[i];
    }
    return checksum;
}

/*
   time operations
*/

/* convert unix time to dos date and time */
static void unix2dos(time_t time, u16 *dosDate, u16 *dosTime) {
    s32 sec = time % 60;
    time /= 60;
    s32 min = time % 60;
    time /= 60;
    s32 hr = time % 24;
    time /= 24;
    time += 719468;
    s32 era = (time >= 0 ? time : time - 146096) / 146097;
    s32 doe = time - era * 146097;
    s32 yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    s32 y = yoe + era * 400;
    s32 doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    s32 mp = (5 * doy + 2) / 153;
    s32 d = doy - (153 * mp + 2) / 5 + 1;
    s32 m = mp + (mp < 10 ? 3 : -9);
    y += (m <= 2);
    if (y < 1980 || y > 2107) {
        if (dosDate) {
            *dosDate = 0;
        }
        if (dosTime) {
            *dosTime = 0;
        }
    } else {
        if (dosDate) {
            *dosDate = ((u16)(y - 1980) << 9) | ((u16)m << 5) | ((u16)d << 0);
        }
        if (dosTime) {
            *dosTime = ((u16)hr << 11) | ((u16)min << 5) | ((u16)(sec / 2) << 0);
        }
    }
}

/* convert dos date and time to unix time */
static time_t dos2unix(u16 dosDate, u16 dosTime) {
    s32 y = 1980 + ((dosDate >> 9) & 0x7F);
    s32 m = (s32)((dosDate >> 5) & 0xF);
    s32 d = (s32)((dosDate >> 0) & 0x1F);
    s32 hr = (dosTime >> 11) & 0x1F;
    s32 min = (dosTime >> 5) & 0x3F;
    s32 sec = ((dosTime >> 0) & 0x1F) * 2;
    if (m < 1 || m > 12 || d < 1 || d > 31 || hr > 23 || min > 59 || sec > 59) {
        return 0;
    }
    y -= (m <= 2);
    s32 era = (y >= 0 ? y : y - 399) / 400;
    s32 yoe = y - era * 400;
    s32 doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    s32 doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return (era * 146097 + doe - 719468) * 86400 + hr * 3600 + min * 60 + sec;
}

/*
   cache operations
*/

static s32 cacheFlush(struct Fat *fat, s32 index) {
    struct FatCache *cache = &fat->cache[index];
    if (!cache->valid || !cache->dirty) {
        return 0;
    }
    if (fat->write(cache->loadLba, cache->nSect, cache->data)) {
        errno = EIO;
        return -1;
    }
    cache->dirty = FALSE;
    return 0;
}

static void *cachePrep(struct Fat *fat, s32 index, u32 lba, bool load) {
    struct FatCache *cache = &fat->cache[index];
    if (cache->valid && cache->loadLba <= lba && cache->loadLba + cache->nSect > lba) {
        cache->prepLba = lba;
        u32 offset = fat->nSectByte * (cache->prepLba - cache->loadLba);
        return &cache->data[offset];
    }
    if (cacheFlush(fat, index)) {
        return NULL;
    }
    if (load) {
        cache->nSect = FAT_MAX_CACHE_SECT;
        if (lba + cache->nSect > cache->maxLba) {
            cache->nSect = cache->maxLba - lba;
        }
        if (fat->read(lba, cache->nSect, cache->data)) {
            cache->valid = FALSE;
            errno = EIO;
            return NULL;
        }
    } else {
        cache->nSect = 1;
    }
    cache->valid = TRUE;
    cache->dirty = FALSE;
    cache->loadLba = lba;
    cache->prepLba = lba;
    return cache->data;
}

static void cacheDirty(struct Fat *fat, s32 index) {
    fat->cache[index].dirty = TRUE;
}

static void cacheInval(struct Fat *fat, s32 index) {
    fat->cache[index].valid = FALSE;
}

static void cacheRead(struct Fat *fat, s32 index, u32 offset, void *dst, u32 length) {
    struct FatCache *cache = &fat->cache[index];
    offset += fat->nSectByte * (cache->prepLba - cache->loadLba);
    if (dst) {
        memcpy(dst, &cache->data[offset], length);
    }
}

static void cacheWrite(struct Fat *fat, s32 index, u32 offset, const void *src, u32 length) {
    struct FatCache *cache = &fat->cache[index];
    offset += fat->nSectByte * (cache->prepLba - cache->loadLba);
    if (src) {
        memcpy(&cache->data[offset], src, length);
    } else {
        memset(&cache->data[offset], 0, length);
    }
    cache->dirty = TRUE;
}

static u32 getWord(const void *buf, u32 offset, s32 width) {
    const u8 *p = buf;
    u32 word = 0;
    for (s32 i = 0; i < width; ++i) {
        word |= (u32)p[offset + i] << (i * 8);
    }
    return word;
}

static void setWord(void *buf, u32 offset, s32 width, u32 value) {
    u8 *p = buf;
    for (s32 i = 0; i < width; ++i) {
        p[offset + i] = value >> (i * 8);
    }
}

/*
   cluster operations
*/

static s32 getClustFat12(struct Fat *fat, u32 clust, u32 *value) {
    u32 offset = clust / 2 * 3;
    u32 lba = offset / fat->nSectByte;
    offset %= fat->nSectByte;
    s32 n = 3;
    if (offset + n > fat->nSectByte) {
        n = fat->nSectByte - offset;
    }
    void *block = cachePrep(fat, FAT_CACHE_FAT, fat->fatLba + lba, TRUE);
    if (!block) {
        return -1;
    }
    u32 group = getWord(block, offset, n);
    if (n < 3) {
        block = cachePrep(fat, FAT_CACHE_FAT, fat->fatLba + lba + 1, TRUE);
        if (!block) {
            return -1;
        }
        group |= (getWord(block, 0, 3 - n) << (8 * n));
    }
    if (clust % 2 == 1) {
        group >>= 12;
    }
    *value = group & 0xFFF;
    if (*value >= 0xFF7) {
        *value |= 0x0FFFF000;
    }
    if (clust == fat->freeLb && *value != 0) {
        ++fat->freeLb;
    }
    return 0;
}

static s32 setClustFat12(struct Fat *fat, u32 clust, u32 value) {
    u32 offset = clust / 2 * 3;
    u32 lba = offset / fat->nSectByte;
    offset %= fat->nSectByte;
    u32 mask = 0xFFF;
    value &= mask;
    if (clust % 2 == 1) {
        value <<= 12;
        mask <<= 12;
    }
    s32 n = 3;
    if (offset + n > fat->nSectByte) {
        n = fat->nSectByte - offset;
    }
    void *block = cachePrep(fat, FAT_CACHE_FAT, fat->fatLba + lba, TRUE);
    if (!block) {
        return -1;
    }
    value |= (getWord(block, offset, n) & ~mask);
    setWord(block, offset, n, value);
    if (n < 3) {
        cacheDirty(fat, FAT_CACHE_FAT);
        block = cachePrep(fat, FAT_CACHE_FAT, fat->fatLba + lba + 1, TRUE);
        if (!block) {
            return -1;
        }
        value |= ((getWord(block, 0, 3 - n) << (8 * n)) & ~mask);
        setWord(block, offset, 3 - n, value >> (8 * n));
    }
    cacheDirty(fat, FAT_CACHE_FAT);
    if (clust < fat->freeLb && value == 0) {
        fat->freeLb = clust;
    } else if (clust == fat->freeLb && value != 0) {
        ++fat->freeLb;
    }
    return 0;
}

/* get the value of a cluster entry in the FAT */
static s32 getClust(struct Fat *fat, u32 clust, u32 *value) {
    if (clust >= fat->maxClust) {
        errno = EOVERFLOW;
        return -1;
    }
    if (fat->type == FAT12) {
        return getClustFat12(fat, clust, value);
    }
    u32 entSize = fat->type == FAT16 ? 2 : 4;
    u32 lba = clust / (fat->nSectByte / entSize);
    u32 offset = clust % (fat->nSectByte / entSize) * entSize;
    void *block = cachePrep(fat, FAT_CACHE_FAT, fat->fatLba + lba, TRUE);
    if (!block) {
        return -1;
    }
    if (fat->type == FAT16) {
        *value = getWord(block, offset, 2);
        if (*value >= 0xFFF7) {
            *value |= 0x0FFF0000;
        }
    } else {
        *value = getWord(block, offset, 4);
        *value &= 0x0FFFFFFF;
    }
    if (clust == fat->freeLb && *value != 0) {
        ++fat->freeLb;
    }
    return 0;
}

/* set the value of a cluster entry in the FAT */
static s32 setClust(struct Fat *fat, u32 clust, u32 value) {
    if (clust >= fat->maxClust) {
        errno = EOVERFLOW;
        return -1;
    }
    if (fat->type == FAT12) {
        return setClustFat12(fat, clust, value);
    }
    u32 entSize = fat->type == FAT16 ? 2 : 4;
    u32 lba = clust / (fat->nSectByte / entSize);
    u32 offset = clust % (fat->nSectByte / entSize) * entSize;
    void *block = cachePrep(fat, FAT_CACHE_FAT, fat->fatLba + lba, TRUE);
    if (!block) {
        return -1;
    }
    if (fat->type == FAT16) {
        value &= 0x0000FFFF;
        setWord(block, offset, 2, value);
    } else {
        value &= 0x0FFFFFFF;
        setWord(block, offset, 4, value);
    }
    cacheDirty(fat, FAT_CACHE_FAT);
    if (clust < fat->freeLb && value == 0) {
        fat->freeLb = clust;
    } else if (clust == fat->freeLb && value != 0) {
        ++fat->freeLb;
    }
    return 0;
}

/* get the next cluster in a cluster chain, returns 1 on success, 0 on eof,
   -1 on error */
static s32 advanceClust(struct Fat *fat, u32 *clust) {
    /* treat reserved clusters as the root directory */
    u32 current = *clust;
    if (current < 2) {
        if (fat->type == FAT32) {
            current = fat->rootClust;
        } else {
            return 0;
        }
    }
    /* get next cluster index */
    u32 next;
    if (getClust(fat, current, &next)) {
        return -1;
    }
    /* check for end of chain */
    if (next < 2 || next >= 0x0FFFFFF8 || next == current) {
        return 0;
    }
    *clust = next;
    return 1;
}

/* check the number of free clusters in a sequence from `clust`, up to `max` */
static u32 checkFreeChunkLength(struct Fat *fat, u32 clust, u32 max) {
    /* treat reserved clusters as the root directory */
    if (clust < 2) {
        if (fat->type == FAT32) {
            clust = fat->rootClust;
        } else {
            return 0;
        }
    }
    u32 length = 0;
    while (length < max && clust < fat->maxClust) {
        u32 value;
        if (getClust(fat, clust, &value)) {
            return 0;
        }
        if (value != 0) {
            break;
        }
        ++length;
        ++clust;
    }
    return length;
}
/* find the first free cluster after `clust` (inclusive),
   preferably with at most `pref_length` free clusters chunked in a sequence.
   returns the actual chunk length in `*length` if given. */
static u32 findFreeClust(struct Fat *fat, u32 clust, u32 prefLength, u32 *length) {
    if (clust < fat->freeLb) {
        clust = fat->freeLb;
    }
    u32 maxClust = 0;
    u32 maxLength = 0;
    while (maxLength < prefLength && clust < fat->maxClust) {
        u32 chunkLength = checkFreeChunkLength(fat, clust, prefLength);
        if (chunkLength > maxLength) {
            maxClust = clust;
            maxLength = chunkLength;
        }
        if (chunkLength > 0) {
            clust += chunkLength;
        } else {
            ++clust;
        }
    }
    if (maxLength == 0) {
        errno = ENOSPC;
    }
    if (length) {
        *length = maxLength;
    }
    return maxClust;
}

/* link `clust` to `next_clust`, mark as end-of-chain if `eoc`. */
static s32 linkClust(struct Fat *fat, u32 clust, u32 nextClust, bool eoc) {
    /* treat reserved clusters as the root directory */
    if (clust < 2) {
        if (fat->type == FAT32) {
            clust = fat->rootClust;
        } else {
            errno = EINVAL;
            return -1;
        }
    }
    /* link cluster */
    if (setClust(fat, clust, nextClust)) {
        return -1;
    }
    /* add end-of-chain marker */
    if (eoc) {
        if (setClust(fat, nextClust, 0x0FFFFFFF)) {
            return -1;
        }
    }
    return 0;
}

/* check if there are at least `needed` free clusters available */
static s32 checkFreeSpace(struct Fat *fat, u32 needed) {
    u32 nFree = 0;
    for (u32 i = fat->freeLb; i < fat->maxClust && nFree < needed; ++i) {
        u32 value;
        if (getClust(fat, i, &value)) {
            return -1;
        }
        if (value == 0x00000000) {
            ++nFree;
        }
    }
    if (nFree < needed) {
        errno = ENOSPC;
        return -1;
    }
    return 0;
}

/* resize the cluster chain that begins at `cluster` to length `n`,
   link and unlink clusters as needed. `chunk_length` is the precomputed
   number of free clusters in sequence at the end of the chain, or zero. */
static s32 resizeClustChain(struct Fat *fat, u32 clust, u32 n, u32 chunkLength) {
    /* treat reserved clusters as the root directory */
    if (clust < 2) {
        if (fat->type == FAT32) {
            clust = fat->rootClust;
        } else {
            errno = ENOSPC;
            return -1;
        }
    }
    /* walk cluster chain */
    bool eoc = FALSE;
    u32 nAlloc = 0;
    u32 newClust = 0;
    for (u32 i = 0; i < n || !eoc; ++i) {
        u32 value;
        if (getClust(fat, clust, &value)) {
            return -1;
        }
        if (!eoc && (value >= 0x0FFFFFF8 || value < 2)) {
            if (setClust(fat, clust, 0x0FFFFFFF)) {
                return -1;
            }
            if (n > i + 1) {
                /* check if there's enough space left for the
                   additional requested clusters */
                nAlloc = n - (i + 1);
                if (checkFreeSpace(fat, nAlloc)) {
                    return -1;
                }
            }
            eoc = TRUE;
        }
        if (i >= n) {
            if (setClust(fat, clust, 0x00000000)) {
                return -1;
            }
            clust = value;
        } else if (i == n - 1) {
            if (setClust(fat, clust, 0x0FFFFFFF)) {
                return -1;
            }
            clust = value;
        } else if (eoc) {
            if (chunkLength == 0) {
                newClust = findFreeClust(fat, newClust, nAlloc, &chunkLength);
                if (newClust == clust) {
                    newClust = findFreeClust(fat, clust + 1, nAlloc, &chunkLength);
                }
                if (newClust == 0) {
                    return -1;
                }
                nAlloc -= chunkLength;
            } else {
                newClust = clust + 1;
            }
            if (linkClust(fat, clust, newClust, 0)) {
                return -1;
            }
            --chunkLength;
            clust = newClust;
        } else {
            clust = value;
        }
    }
    return 0;
}

/*
   basic file operations
*/

/* prep or load the block pointed to by `file` */
static s32 fileSect(const struct FatFile *file, bool load) {
    struct Fat *fat = file->fat;
    u32 clustLba;
    /* treat reserved clusters as the root directory */
    if (file->pClust < 2) {
        if (fat->type == FAT32) {
            clustLba = fat->dataLba + (fat->rootClust - 2) * fat->nClustSect;
        } else {
            clustLba = fat->rootLba;
        }
    } else {
        clustLba = fat->dataLba + (file->pClust - 2) * fat->nClustSect;
    }
    if (!cachePrep(fat, FAT_CACHE_DATA, clustLba + file->pClustSect, load)) {
        return -1;
    }
    return 0;
}

/* return a pointer to the data cache at the file offset in `file` */
static void *fileData(const struct FatFile *file) {
    struct Fat *fat = file->fat;
    struct FatCache *cache = &fat->cache[FAT_CACHE_DATA];
    u32 offset = file->pSectOff;
    offset += fat->nSectByte * (cache->prepLba - cache->loadLba);
    return &cache->data[offset];
}

/* point `file` to the beginning of the root directory */
void fatRoot(struct Fat *fat, struct FatFile *file) {
    file->fat = fat;
    file->clust = 0;
    if (fat->type == FAT32) {
        file->size = 0;
    } else {
        file->size = fat->nEntry * 0x20;
    }
    file->isDir = TRUE;
    fatRewind(file);
}

/* point `file` to the beginning of the file represented by `entry` */
void fatBegin(struct FatEntry *entry, struct FatFile *file) {
    if ((entry->attrib & FAT_ATTRIB_DIRECTORY) && entry->clust < 2) {
        fatRoot(entry->fat, file);
    } else {
        file->fat = entry->fat;
        file->clust = entry->clust;
        file->size = entry->size;
        file->isDir = entry->attrib & FAT_ATTRIB_DIRECTORY;
        fatRewind(file);
    }
}

/* rewind `file` to file offset 0 */
void fatRewind(struct FatFile *file) {
    file->pOff = 0;
    file->pClust = file->clust;
    file->pClustSeq = 0;
    file->pClustSect = 0;
    file->pSectOff = 0;
}

/* advance a file pointer by `n_byte`, returns the number of bytes advanced.
   if eof is reached and `eof` is given, `*eof` is set to true. */
u32 fatAdvance(struct FatFile *file, u32 nByte, bool *eof) {
    struct Fat *fat = file->fat;
    bool ate = FALSE;
    u32 pOff = file->pOff;
    u32 oldOff = pOff;
    u32 newOff = pOff + nByte;
    /* do boundary check if size is known (i.e. non-zero size directory) */
    if (!(file->isDir && file->size == 0)) {
        if (newOff > file->size || newOff < pOff) {
            newOff = file->size;
            nByte = newOff - pOff;
            ate = TRUE;
        }
    }
    /* revert offset to start of current cluster */
    u32 pClustOff = file->pClustSect * fat->nSectByte;
    pClustOff += file->pSectOff;
    pOff -= pClustOff;
    nByte += pClustOff;
    /* advance through cluster chain
       (unless `file` is the FAT12/16 root directory,
       which is not in a cluster) */
    bool noClust = fat->type != FAT32 && file->clust < 2;
    if (!noClust) {
        /* compute current and target cluster sequence index */
        u32 clust = file->pClust;
        u32 clustSeq = file->pClustSeq;
        u32 newClustSeq = newOff / fat->nClustByte;
        /* walk cluster chain */
        while (clustSeq < newClustSeq) {
            s32 e = advanceClust(fat, &clust);
            if (e == -1) {
                if (eof) {
                    *eof = FALSE;
                }
                return 0;
            }
            /* if the end of the cluster chain is reached,
               advance to the end of the last cluster */
            if (e == 0) {
                nByte = fat->nClustByte;
                ate = 1;
                break;
            }
            pOff += fat->nClustByte;
            nByte -= fat->nClustByte;
            ++clustSeq;
        }
        file->pClust = clust;
        file->pClustSeq = clustSeq;
    }
    /* advance sector and offset */
    pOff += nByte;
    file->pOff = pOff;
    file->pClustSect = nByte / fat->nSectByte;
    file->pSectOff = nByte % fat->nSectByte;
    /* ensure that the sector is within the cluster range */
    if (!noClust) {
        if (file->pClustSect == fat->nClustSect) {
            --file->pClustSect;
            file->pSectOff += fat->nSectByte;
        }
    }
    if (eof) {
        *eof = ate;
    }
    return pOff - oldOff;
}

/* copy multiple clusters to or from a file and advance.
   assumes the file is pointed at the start of a cluster. */
static u32 clustRw(struct FatFile *file, enum FatRw rw, void *buf, u32 nClust, bool *eof) {
    struct Fat *fat = file->fat;
    char *p = buf;
    /* flush and invalidate data cache to prevent conflicts */
    if (cacheFlush(fat, FAT_CACHE_DATA)) {
        return 0;
    }
    cacheInval(fat, FAT_CACHE_DATA);
    /* treat reserved clusters as the root directory */
    u32 clust = file->pClust;
    if (clust < 2) {
        clust = fat->rootClust;
    }
    /* cluster loop */
    u32 nCopy = 0;
    while (nClust > 0) {
        u32 chunkStart = clust;
        u32 chunkLength = 1;
        /* compute consecutive cluster chunk length */
        u32 pClust = clust;
        while (1) {
            if (getClust(fat, clust, &clust)) {
                return nCopy;
            }
            if (clust >= 0x0FFFFFF7 || clust != pClust + 1 || chunkLength >= nClust) {
                break;
            }
            pClust = clust;
            ++chunkLength;
        }
        /* copy chunk */
        u32 lba = fat->dataLba + fat->nClustSect * (chunkStart - 2);
        u32 nBlock = fat->nClustSect * chunkLength;
        u32 nByte = nBlock * fat->nSectByte;
        s32 e;
        if (rw == FAT_READ) {
            e = fat->read(lba, nBlock, p);
        } else {
            e = fat->write(lba, nBlock, p);
        }
        if (e) {
            break;
        }
        nClust -= chunkLength;
        nCopy += chunkLength;
        file->pOff += nByte;
        file->pClustSeq += chunkLength;
        if (clust < 2 || clust >= 0x0FFFFFF7) {
            file->pClustSeq--;
            file->pClust = pClust;
            file->pClustSect = fat->nClustSect - 1;
            file->pSectOff = fat->nSectByte;
            if (eof) {
                *eof = TRUE;
            }
            break;
        } else {
            file->pClust = clust;
            p += nByte;
        }
    }
    return nCopy;
}

/* copy bytes to or from a file, returns the number of bytes copied.
   the updated file pointer is stored to `new_file` if given. */
u32 fatRw(struct FatFile *file, enum FatRw rw, void *buf, u32 nByte, struct FatFile *newFile, bool *eof) {
    if (nByte == 0) {
        if (eof) {
            *eof = FALSE;
        }
        return 0;
    }
    struct Fat *fat = file->fat;
    bool ate = TRUE;
    /* do boundary check if size is known (i.e. non-zero size directory) */
    if (!(file->isDir && file->size == 0)) {
        if (file->pOff >= file->size) {
            if (eof) {
                *eof = TRUE;
            }
            return 0;
        }
        u32 newOff = file->pOff + nByte;
        if (newOff > file->size || newOff < file->pOff) {
            nByte = file->size - file->pOff;
        }
    }
    bool noClust = fat->type != FAT32 && file->clust < 2;
    /* traverse file with a local copy of the file pointer */
    struct FatFile pos = *file;
    /* sector loop */
    char *p = buf;
    u32 nCopy = 0;
    while (nByte > 0) {
        /* write cluster chunks if possible */
        if (!noClust && nByte >= fat->nClustByte && pos.pClustSect == 0 && pos.pSectOff == 0) {
            u32 nClust = nByte / fat->nClustByte;
            u32 nCopyClust = clustRw(&pos, rw, p, nClust, &ate);
            u32 nByteClust = nCopyClust * fat->nClustByte;
            if (p) {
                p += nByteClust;
            }
            nByte -= nByteClust;
            nCopy += nByteClust;
            if (nCopyClust != nClust || ate) {
                break;
            } else {
                continue;
            }
        }
        /* compute chunk size */
        u32 chunkSize = fat->nSectByte - pos.pSectOff;
        if (chunkSize > nByte) {
            chunkSize = nByte;
        }
        /* prep or load sector */
        u32 pSectOff = pos.pSectOff;
        if (chunkSize > 0) {
            if (fileSect(&pos, rw == FAT_READ || chunkSize != fat->nSectByte)) {
                break;
            }
        }
        /* advance position pointer, clear errno to check for errors */
        s32 e = errno;
        errno = 0;
        u32 adv = fatAdvance(&pos, chunkSize, &ate);
        /* copy chunk */
        if (adv > 0) {
            if (rw == FAT_READ) {
                cacheRead(fat, FAT_CACHE_DATA, pSectOff, p, adv);
            } else {
                cacheWrite(fat, FAT_CACHE_DATA, pSectOff, p, adv);
            }
            if (p) {
                p += adv;
            }
            nByte -= adv;
            nCopy += adv;
        }
        /* restore errno */
        if (errno == 0) {
            errno = e;
        } else {
            break;
        }
        if (adv != chunkSize || ate) {
            break;
        }
    }
    if (newFile) {
        *newFile = pos;
    }
    if (eof) {
        *eof = ate;
    }
    return nCopy;
}

/*
   directory operations
*/

/* read the next entry in a directory file */
s32 fatDir(struct FatFile *dir, struct FatEntry *entry) {
    /* sanity check */
    if (!dir->isDir) {
        errno = ENOTDIR;
        return -1;
    }
    struct Fat *fat = dir->fat;
    /* lfn state */
    s32 lfnSeq = -1;
    struct FatFile lfnP;
    u8 lfnChecksum = 0;
    char lfnBuf[256];
    /* physical entry buffer */
    char entBuf[0x20];
    /* next entry pointer */
    struct FatFile dirNext;
    /* entry loop */
    while (fatRw(dir, FAT_READ, entBuf, 0x20, &dirNext, NULL) == 0x20) {
        /* get potential special entry marker */
        u8 mark = getWord(entBuf, 0x00, 1);
        /* store entry pointer advance directory file pointer */
        struct FatFile entP = *dir;
        *dir = dirNext;
        /* check for free entry */
        if (mark == 0x00 || mark == 0xE5) {
            lfnSeq = -1;
            continue;
        }
        /* check for lfn entry */
        u8 attrib = getWord(entBuf, 0x0B, 1);
        if (attrib == 0x0F) {
            u8 seq = mark & 0x1F;
            /* validate sequence number */
            if (seq < 0x01 || seq > 0x14) {
                lfnSeq = -1;
                continue;
            }
            u8 checksum = getWord(entBuf, 0x0D, 1);
            /* check for last lfn flag, indicating start of lfn entry chain */
            if (mark & 0x40) {
                lfnSeq = seq;
                lfnP = entP;
                lfnChecksum = checksum;
                memset(lfnBuf, 0, sizeof(lfnBuf));
            } else {
                /* validate sequence coherency */
                if (seq != lfnSeq - 1 || checksum != lfnChecksum) {
                    lfnSeq = -1;
                    continue;
                }
                lfnSeq = seq;
            }
            /* read lfn part (truncate wide characters) */
            s32 n = (lfnSeq - 1) * 13;
            for (s32 j = 0; j < 13 && n < 255; ++j) {
                u32 p = 1 + j * 2;
                if (j >= 5) {
                    p += 3;
                }
                if (j >= 11) {
                    p += 2;
                }
                u16 c = getWord(entBuf, p, 2);
                if (c > 0xFF) {
                    c = 0x7F;
                }
                lfnBuf[n++] = c;
            }
        }
        /* handle regular entry */
        else {
            /* check for lfn */
            bool haveLfn = FALSE;
            if (lfnSeq == 1 && lfnChecksum == computeLfnChecksum(entBuf)) {
                haveLfn = TRUE;
            }
            lfnSeq = -1;
            /* sanity check */
            if ((attrib & FAT_ATTRIB_DIRECTORY) && (attrib & FAT_ATTRIB_LABEL)) {
                continue;
            }
            /* validate name field */
            if (!validateSfn(&entBuf[0])) {
                continue;
            }
            /* get sfn, check for empty name */
            s32 nameL;
            s32 extL;
            if (getSfn(&entBuf[0], entry->shortName, &nameL, &extL) == 0) {
                continue;
            }
            /* check for 0xE5 escape character */
            if (entry->shortName[0] == '\x05') {
                entry->shortName[0] = '\xE5';
            }
            /* copy lfn to entry name */
            if (haveLfn) {
                strcpy(entry->name, lfnBuf);
                entry->first = lfnP;
            }
            /* use sfn as entry name if there's no lfn */
            else {
                strcpy(entry->name, entry->shortName);
                entry->first = entP;
                /* do case conversions */
                u8 cse = getWord(entBuf, 0x0C, 1);
                if (cse & 0x08) {
                    cvtLower(&entry->name[0], nameL);
                }
                if (cse & 0x10) {
                    cvtLower(&entry->name[nameL + 1], extL);
                }
            }
            entry->last = entP;
            /* insert metadata */
            entry->ctime = dos2unix(getWord(entBuf, 0x10, 2), getWord(entBuf, 0x0E, 2));
            entry->cms = getWord(entBuf, 0x0D, 1) * 10;
            entry->ctime += entry->cms / 1000;
            entry->cms %= 1000;
            entry->atime = dos2unix(getWord(entBuf, 0x12, 2), 0);
            entry->mtime = dos2unix(getWord(entBuf, 0x18, 2), getWord(entBuf, 0x16, 2));
            entry->attrib = attrib;
            if (entry->attrib & FAT_ATTRIB_LABEL) {
                entry->clust = 0;
            } else {
                entry->clust = getWord(entBuf, 0x1A, 2);
                if (fat->type == FAT32) {
                    entry->clust |= getWord(entBuf, 0x14, 2) << 16;
                    /* ensure that the root cluster is always presented as 0 to ensure
                       serial number consistency */
                    if (entry->clust == fat->rootClust) {
                        entry->clust = 0;
                    }
                }
                if (entry->clust == 1) {
                    entry->clust = 0;
                }
            }
            if (fat->type != FAT32 && (entry->attrib & FAT_ATTRIB_DIRECTORY) && entry->clust < 2) {
                entry->size = fat->nEntry * 0x20;
            } else if (entry->attrib & (FAT_ATTRIB_DIRECTORY | FAT_ATTRIB_LABEL)) {
                entry->size = 0;
            } else {
                entry->size = getWord(entBuf, 0x1C, 4);
            }
            /* success */
            entry->fat = dir->fat;
            return 0;
        }
    }
    return -1;
}

/* point `file` to the start of the directory at `cluster` */
static void beginDir(struct Fat *fat, struct FatFile *file, u32 clust) {
    if (clust < 2) {
        fatRoot(fat, file);
    } else {
        file->fat = fat;
        file->clust = clust;
        file->size = 0;
        file->isDir = TRUE;
        fatRewind(file);
    }
}

/* find a directory entry by name */
static s32 dirFind(struct Fat *fat, u32 clust, const char *name, struct FatEntry *entry) {
    struct FatFile pos;
    beginDir(fat, &pos, clust);
    bool isSfn = nameIsSfn(name, NULL, NULL);
    struct FatEntry ent;
    s32 e = errno;
    errno = 0;
    while (fatDir(&pos, &ent) == 0) {
        if (ent.attrib & FAT_ATTRIB_LABEL) {
            continue;
        }
        bool match;
        if (isSfn) {
            match = nameComp(name, ent.shortName);
        } else {
            match = nameComp(name, ent.name);
        }
        if (match) {
            if (entry) {
                *entry = ent;
            }
            errno = e;
            return 0;
        }
    }
    if (errno == 0) {
        errno = ENOENT;
    }
    return -1;
}

/* point an entry structure to the root directory */
static void makeRoot(struct Fat *fat, struct FatEntry *entry) {
    memset(entry, 0, sizeof(*entry));
    entry->fat = fat;
    entry->attrib = FAT_ATTRIB_DIRECTORY;
    if (fat->type != FAT32) {
        entry->size = fat->nEntry * 0x20;
    }
}

/* find the entry named by `path`, relative to `dir`.
   if `dir` is not given, `path` is relative to the root directory. */
s32 fatFind(struct Fat *fat, struct FatEntry *dir, const char *path, struct FatEntry *entry) {
    struct FatEntry ent;
    if (dir) {
        ent = *dir;
    } else {
        makeRoot(fat, &ent);
    }
    if (!path) {
        if (entry) {
            *entry = ent;
        }
        return 0;
    }
    /* substring loop */
    const char *p = path;
    while (*p) {
        if (!(ent.attrib & FAT_ATTRIB_DIRECTORY)) {
            errno = ENOTDIR;
            return -1;
        }
        /* extract substring */
        const char *s = p;
        const char *e = p;
        while (*p) {
            char c = *p++;
            if (c == '/' || c == '\\') {
                break;
            }
            ++e;
        }
        /* validate */
        size_t nameLength = nameTrim(s, e - s);
        if (nameLength == 0) {
            while (s[nameLength] == '.') {
                ++nameLength;
            }
        }
        if (nameLength > 255) {
            errno = ENAMETOOLONG;
            return -1;
        }
        if (nameLength == 0) {
            continue;
        }
        /* find entry */
        char name[256];
        memcpy(name, s, nameLength);
        name[nameLength] = 0;
        if (strcmp(name, ".") == 0) {
            continue;
        }
        if (dirFind(fat, ent.clust, name, &ent)) {
            return -1;
        }
    }
    if (entry) {
        *entry = ent;
    }
    return 0;
}

/* find the entry named by `path`, relative to the directory pointed to
   by `dir_fp`. if `dir_fp` is not given, `path` is relative to the root.
   returns a new fat_path pointing to the canonical location of the
   furthest sub-entry of `path` that can be found.
   if a sub-entry is not found and `tail` is given,
   `*tail` will point to the name of that entry in `path`. */
struct FatPath *fatPath(struct Fat *fat, struct FatPath *dirFp, const char *path, const char **tail) {
    struct FatPath *fp = malloc(sizeof(*fp));
    if (!fp) {
        errno = ENOMEM;
        return NULL;
    }
    list_init(&fp->entList, sizeof(struct FatEntry));
    if (dirFp) {
        for (struct FatEntry *lEnt = dirFp->entList.first; lEnt; lEnt = list_next(lEnt)) {
            if (!list_push_back(&fp->entList, lEnt)) {
                errno = ENOMEM;
                fatFree(fp);
                return NULL;
            }
        }
    } else {
        struct FatEntry *ent = list_push_back(&fp->entList, NULL);
        if (!ent) {
            errno = ENOMEM;
            fatFree(fp);
            return NULL;
        }
        makeRoot(fat, ent);
    }
    struct FatEntry *ent = fp->entList.last;
    /* substring loop */
    const char *p = path;
    while (*p) {
        if (!(ent->attrib & FAT_ATTRIB_DIRECTORY)) {
            errno = ENOTDIR;
            break;
        }
        /* extract substring */
        const char *s = p;
        const char *e = p;
        while (*p) {
            char c = *p++;
            if (c == '/' || c == '\\') {
                break;
            }
            ++e;
        }
        /* validate */
        size_t nameLength = nameTrim(s, e - s);
        if (nameLength == 0) {
            while (s[nameLength] == '.') {
                ++nameLength;
            }
        }
        if (nameLength > 255) {
            errno = ENAMETOOLONG;
            break;
        }
        if (nameLength == 0) {
            continue;
        }
        /* find entry */
        char name[256];
        memcpy(name, s, nameLength);
        name[nameLength] = 0;
        if (strcmp(name, ".") == 0) {
            continue;
        }
        struct FatEntry dEnt;
        if (dirFind(fat, ent->clust, name, &dEnt)) {
            if (errno == ENOENT && tail) {
                *tail = s;
            }
            break;
        }
        /* backtrack if entry exists in path */
        if (dEnt.attrib & FAT_ATTRIB_DIRECTORY) {
            bool exist = FALSE;
            for (struct FatEntry *lEnt = fp->entList.first; lEnt; lEnt = list_next(lEnt)) {
                if (exist) {
                    struct FatEntry *t = lEnt;
                    lEnt = list_prev(lEnt);
                    list_erase(&fp->entList, t);
                } else if ((lEnt->attrib & FAT_ATTRIB_DIRECTORY) && lEnt->clust == dEnt.clust) {
                    ent = lEnt;
                    exist = TRUE;
                }
            }
            if (exist) {
                continue;
            }
        }
        /* append to entry list in path */
        ent = list_push_back(&fp->entList, &dEnt);
        if (!ent) {
            errno = ENOMEM;
            break;
        }
    }
    return fp;
}

/* return the entry that a path points to */
struct FatEntry *fatPathTarget(struct FatPath *fp) {
    if (fp) {
        return fp->entList.last;
    } else {
        return NULL;
    }
}

/* return the directory entry which contains the target entry of a path */
struct FatEntry *fatPathDir(struct FatPath *fp) {
    return list_prev(fp->entList.last);
}

/* destroy and delete a path created by `fat_path` */
void fatFree(struct FatPath *ptr) {
    list_destroy(&ptr->entList);
    free(ptr);
}

/* generate a free sfn from an lfn */
static s32 generateSfn(struct Fat *fat, u32 clust, const char *lfn, char *sfn) {
    const char *name;
    const char *ext;
    s32 nameLength;
    s32 extLength;
    nameSplit(lfn, &name, &ext, &nameLength, &extLength);
    /* shorten name components */
    s32 sfnNameLength = cvtSfn(name, nameLength, &sfn[0], 8);
    s16 sfnExtLength = cvtSfn(ext, extLength, &sfn[8], 3);
    /* find a free short name */
    for (s32 i = 1; i < 1000000; ++i) {
        /* make discriminator */
        char sfnDisc[8];
        snprintf(sfnDisc, sizeof(sfnDisc), "%li", i);
        s32 sfnDiscLength = strlen(sfnDisc);
        /* make name */
        s16 sfnNdLength = 7 - sfnDiscLength;
        if (sfnNdLength > sfnNameLength) {
            sfnNdLength = sfnNameLength;
        }
        char nameBuf[13];
        snprintf(nameBuf, sizeof(nameBuf), "%.*s~%s.%.*s", sfnNdLength, &sfn[0], sfnDisc, sfnExtLength, &sfn[8]);
        /* check if exists */
        s32 e = errno;
        if (dirFind(fat, clust, nameBuf, NULL) == 0) {
            continue;
        } else if (errno != ENOENT) {
            return -1;
        }
        errno = e;
        /* return name with discriminator */
        sfn[sfnNdLength] = '~';
        memcpy(&sfn[sfnNdLength + 1], sfnDisc, sfnDiscLength);
        return 0;
    }
    errno = EEXIST;
    return -1;
}

/* basic vfat directory entry insertion */
static s32 dirInsert(struct Fat *fat, u32 dirClust, const char *name, time_t ctime, s32 cms, time_t atime, time_t mtime,
                     u8 attrib, u32 clust, u32 size, struct FatEntry *entry) {
    char lfnEntBuf[0x20];
    char sfnEntBuf[0x20];
    /* check name type and determine the required number of physical entries */
    s32 nameLength = strlen(name);
    s32 nPent = 1;
    {
        bool lowerName;
        bool lowerExt;
        if (nameIsSfn(name, &lowerName, &lowerExt)) {
            /* sfn characters */
            cvt83(name, &sfnEntBuf[0x00]);
            /* case info */
            u8 cse = 0x00;
            if (lowerName) {
                cse |= 0x08;
            }
            if (lowerExt) {
                cse |= 0x10;
            }
            setWord(sfnEntBuf, 0x0C, 1, cse);
        } else {
            nPent += (nameLength + 12) / 13;
            /* sfn characters */
            if (generateSfn(fat, dirClust, name, &sfnEntBuf[0x00])) {
                return -1;
            }
            /* case info */
            setWord(sfnEntBuf, 0x0C, 1, 0x00);
            /* attrib */
            setWord(lfnEntBuf, 0x0B, 1, 0x0F);
            /* type */
            setWord(lfnEntBuf, 0x0C, 1, 0x00);
            /* checksum */
            setWord(lfnEntBuf, 0x0D, 1, computeLfnChecksum(&sfnEntBuf[0x00]));
            /* cluster */
            setWord(lfnEntBuf, 0x1A, 2, 0x0000);
        }
    }
    /* initialize search position */
    struct FatFile start;
    struct FatFile pos;
    beginDir(fat, &pos, dirClust);
    /* find a free entry sequence of the required length */
    for (s32 nFree = 0; nFree < nPent;) {
        /* check for eof */
        struct FatFile entP = pos;
        bool ate;
        if (fatAdvance(&pos, 0x20, &ate) != 0x20) {
            if (!ate) {
                return -1;
            }
            pos = entP;
            /* expand directory file */
            u32 newClust = findFreeClust(fat, 0, 1, NULL);
            if (newClust == 0) {
                return -1;
            }
            if (linkClust(fat, pos.pClust, newClust, 1)) {
                return -1;
            }
            struct FatFile clustPos;
            beginDir(fat, &clustPos, newClust);
            if (fatRw(&clustPos, FAT_WRITE, NULL, fat->nClustByte, NULL, NULL) != fat->nClustByte) {
                return -1;
            }
            continue;
        }
        /* check entry */
        u8 mark;
        if (fatRw(&entP, FAT_READ, &mark, 1, NULL, &ate) != 1) {
            if (ate) {
                errno = EINVAL;
            }
            return -1;
        }
        /* increase sequence length, or start a new sequence */
        if (mark == 0x00 || mark == 0xE5) {
            if (nFree == 0) {
                start = entP;
            }
            ++nFree;
        }
        /* break the current sequence if the entry is taken */
        else {
            nFree = 0;
        }
    }
    pos = start;
    /* insert lfn entries */
    for (s32 i = 0; i < nPent - 1; ++i) {
        /* sequence */
        u8 seq = nPent - 1 - i;
        if (i == 0) {
            seq |= 0x40;
        }
        setWord(lfnEntBuf, 0x00, 1, seq);
        /* name characters */
        for (s32 j = 0; j < 13; ++j) {
            u32 p = 1 + j * 2;
            if (j >= 5) {
                p += 3;
            }
            if (j >= 11) {
                p += 2;
            }
            s32 n = 13 * (nPent - 2 - i) + j;
            u16 c;
            if (n > nameLength) {
                c = 0xFFFF;
            } else if (n == nameLength) {
                c = 0x0000;
            } else {
                c = (u8)name[n];
            }
            setWord(lfnEntBuf, p, 2, c);
        }
        bool ate;
        if (fatRw(&pos, FAT_WRITE, lfnEntBuf, 0x20, &pos, &ate) != 0x20) {
            if (ate) {
                errno = EINVAL;
            }
            return -1;
        }
    }
    /* attribute */
    setWord(sfnEntBuf, 0x0B, 1, attrib);
    /* ctime */
    cms += (ctime % 2) * 1000;
    ctime -= ctime % 2;
    ctime += cms / 2000;
    cms %= 2000;
    u16 dosCdate;
    u16 dosCtime;
    unix2dos(ctime, &dosCdate, &dosCtime);
    setWord(sfnEntBuf, 0x0D, 1, cms / 10);
    setWord(sfnEntBuf, 0x0E, 2, dosCtime);
    setWord(sfnEntBuf, 0x10, 2, dosCdate);
    /* atime */
    u16 dosAdate;
    unix2dos(atime, &dosAdate, NULL);
    setWord(sfnEntBuf, 0x12, 2, dosAdate);
    /* cluster */
    setWord(sfnEntBuf, 0x14, 2, clust >> 16);
    setWord(sfnEntBuf, 0x1A, 2, clust);
    /* mtime */
    u16 dosMdate;
    u16 dosMtime;
    unix2dos(ctime, &dosMdate, &dosMtime);
    setWord(sfnEntBuf, 0x16, 2, dosMtime);
    setWord(sfnEntBuf, 0x18, 2, dosMdate);
    /* size */
    setWord(sfnEntBuf, 0x1C, 4, size);
    /* insert sfn entry */
    struct FatFile eotPos;
    {
        bool ate;
        if (fatRw(&pos, FAT_WRITE, sfnEntBuf, 0x20, &eotPos, &ate) != 0x20) {
            if (ate) {
                errno = EINVAL;
            }
            return -1;
        }
    }
    /* return entry details */
    if (entry) {
        getSfn(&sfnEntBuf[0], entry->shortName, NULL, NULL);
        memcpy(entry->name, name, nameLength);
        entry->name[nameLength] = 0;
        entry->fat = fat;
        entry->first = start;
        entry->last = pos;
        entry->ctime = dos2unix(dosCdate, dosCtime);
        entry->cms = cms;
        entry->atime = dos2unix(dosAdate, 0);
        entry->mtime = dos2unix(dosMdate, dosMtime);
        entry->attrib = attrib;
        entry->clust = clust;
        entry->size = size;
    }
    return 0;
}

/* basic vfat directory entry removal */
static s32 dirRemove(struct FatEntry *entry) {
    struct Fat *fat = entry->fat;
    struct FatFile pos = entry->first;
    while (pos.pOff <= entry->last.pOff) {
        if (fileSect(&pos, TRUE)) {
            return -1;
        }
        void *data = fileData(&pos);
        setWord(data, 0x0D, 1, getWord(data, 0x00, 1));
        setWord(data, 0x00, 1, 0xE5);
        cacheDirty(fat, FAT_CACHE_DATA);
        if (pos.pOff == entry->last.pOff) {
            break;
        }
        bool ate;
        if (fatAdvance(&pos, 0x20, &ate) != 0x20) {
            if (ate) {
                errno = EINVAL;
            }
            return -1;
        }
    }
    return 0;
}

/* create an entry named by `path`, relative to `dir`.
   if `dir` is not given, `path` is relative to the root directory.
   if `attrib` has FAT_ATTRIB_DIRECTORY, a cluster is allocated for the
   directory and dot entries are inserted.
   the entry must not exist. */
s32 fatCreate(struct Fat *fat, struct FatEntry *dir, const char *path, u8 attrib, struct FatEntry *entry) {
    bool isDir = attrib & FAT_ATTRIB_DIRECTORY;
    bool isLabel = attrib & FAT_ATTRIB_LABEL;
    /* sanity check */
    if (isDir && isLabel) {
        errno = EINVAL;
        return -1;
    }
    /* split the path into directory path and file name */
    const char *dirS;
    size_t dirL;
    const char *fileS;
    size_t fileL;
    {
        const char *end = path + nameTrim(path, strlen(path));
        const char *slash = NULL;
        for (const char *p = end; p >= path; --p) {
            if (*p == '\\' || *p == '/') {
                slash = p;
                break;
            }
        }
        if (slash) {
            dirS = path;
            dirL = slash - dirS;
            fileS = slash + 1;
        } else {
            dirS = NULL;
            dirL = 0;
            fileS = path;
        }
        fileL = end - fileS;
        /* validate name */
        if (fileL > 255) {
            errno = ENAMETOOLONG;
            return -1;
        }
        if (fileL == 0) {
            errno = EINVAL;
            return -1;
        }
    }
    /* navigate to directory */
    struct FatEntry dirEnt;
    if (dirL > 0) {
        char *dirPath = malloc(dirL + 1);
        if (!dirPath) {
            errno = ENOMEM;
            return -1;
        }
        memcpy(dirPath, dirS, dirL);
        dirPath[dirL] = 0;
        s32 e = fatFind(fat, dir, dirPath, &dirEnt);
        free(dirPath);
        if (e) {
            return -1;
        }
    } else {
        if (fatFind(fat, dir, NULL, &dirEnt)) {
            return -1;
        }
    }
    /* check if the file exists */
    {
        s32 e = errno;
        if (fatFind(fat, &dirEnt, fileS, NULL) == 0) {
            errno = EEXIST;
            return -1;
        }
        if (errno != ENOENT) {
            return -1;
        }
        errno = e;
    }
    /* allocate directory cluster */
    u32 clust = 0;
    if (isDir) {
        clust = findFreeClust(fat, 0, 1, NULL);
        if (clust < 2) {
            return -1;
        }
        /* end cluster chain */
        if (setClust(fat, clust, 0x0FFFFFFF)) {
            return -1;
        }
    }
    /* insert entry */
    u32 dirClust = dirEnt.clust;
    time_t t = time(NULL);
    {
        char name[256];
        memcpy(name, fileS, fileL);
        name[fileL] = 0;
        if (dirInsert(fat, dirClust, name, t, 0, t, t, attrib, clust, 0, entry)) {
            return -1;
        }
    }
    if (isDir) {
        /* clear directory cluster */
        struct FatFile clustPos;
        beginDir(fat, &clustPos, clust);
        if (fatRw(&clustPos, FAT_WRITE, NULL, fat->nClustByte, NULL, NULL) != fat->nClustByte) {
            return -1;
        }
        /* insert dot entries */
        s32 d = dirInsert(fat, clust, ".", t, 0, t, t, FAT_ATTRIB_DIRECTORY, clust, 0, NULL);
        s32 dd = dirInsert(fat, clust, "..", t, 0, t, t, FAT_ATTRIB_DIRECTORY, dirClust, 0, NULL);
        if (d || dd) {
            return -1;
        }
    }
    return 0;
}

/* same as `fat_create`, except return a path to the new entry */
struct FatPath *fatCreatePath(struct Fat *fat, struct FatPath *dirFp, const char *path, u8 attrib) {
    /* seek destination */
    s32 e = errno;
    errno = 0;
    const char *tail;
    struct FatPath *destFp = fatPath(fat, dirFp, path, &tail);
    if (errno == 0) {
        errno = EEXIST;
        goto error;
    } else {
        if (errno == ENOENT && strlen(tail) > 0 && !strchr(tail, '/') && !strchr(tail, '\\')) {
            errno = e;
        } else {
            goto error;
        }
    }
    /* create entry and insert into path */
    struct FatEntry entry;
    if (fatCreate(fat, fatPathTarget(destFp), tail, attrib, &entry)) {
        goto error;
    }
    if (!list_push_back(&destFp->entList, &entry)) {
        errno = ENOMEM;
        goto error;
    }
    return destFp;
error:
    if (destFp) {
        fatFree(destFp);
    }
    return NULL;
}

/* resize `entry` to `size` (must be a file). `file`, if given, should
   be a valid pointer within `entry`. it will be put into a valid but
   unspecified state after the operation is completed. */
s32 fatResize(struct FatEntry *entry, u32 size, struct FatFile *file) {
    struct Fat *fat = entry->fat;
    /* sanity check */
    if (entry->attrib & FAT_ATTRIB_DIRECTORY) {
        errno = EISDIR;
        return -1;
    }
    if (entry->attrib & FAT_ATTRIB_LABEL) {
        errno = ENOENT;
        return -1;
    }
    if (size == entry->size) {
        return 0;
    }
    /* allocate a cluster if the file is empty */
    u32 nClust = (size + fat->nClustByte - 1) / fat->nClustByte;
    u32 clust = entry->clust;
    u32 chunkLength = 0;
    if (size > 0 && clust < 2) {
        clust = findFreeClust(fat, 0, nClust, &chunkLength);
        if (clust == 0) {
            return -1;
        }
        --chunkLength;
    }
    /* resize cluster chain */
    if (clust >= 2) {
        if (resizeClustChain(fat, clust, nClust, chunkLength)) {
            return -1;
        }
    }
    if (nClust == 0) {
        clust = 0;
    }
    /* update entry */
    entry->clust = clust;
    entry->size = size;
    /* write entry to directory file */
    if (fileSect(&entry->last, TRUE)) {
        return -1;
    }
    void *data = fileData(&entry->last);
    if (fat->type == FAT32) {
        setWord(data, 0x14, 2, entry->clust >> 16);
    }
    setWord(data, 0x1A, 2, entry->clust);
    setWord(data, 0x1C, 4, entry->size);
    cacheDirty(fat, FAT_CACHE_DATA);
    /* update file pointer */
    if (file) {
        file->size = entry->size;
        if (file->size == 0 || file->clust < 2) {
            file->clust = entry->clust;
            fatRewind(file);
        } else if (file->pOff > size || file->pClustSeq >= nClust) {
            fatRewind(file);
        }
    }
    return 0;
}

/* check if a directory is empty */
s32 fatEmpty(struct Fat *fat, struct FatEntry *dir) {
    if (!(dir->attrib & FAT_ATTRIB_DIRECTORY)) {
        errno = ENOTDIR;
        return -1;
    }
    struct FatFile pos;
    if (dir) {
        fatBegin(dir, &pos);
    } else {
        fatRoot(fat, &pos);
    }
    struct FatEntry ent;
    s32 e = errno;
    errno = 0;
    while (fatDir(&pos, &ent) == 0) {
        if (ent.attrib & FAT_ATTRIB_LABEL) {
            continue;
        }
        if (strcmp(ent.name, ".") == 0 || strcmp(ent.name, "..") == 0) {
            continue;
        }
        errno = ENOTEMPTY;
        return -1;
    }
    if (errno != 0) {
        return -1;
    }
    errno = e;
    return 0;
}

/* check if an entry is allowed to be modified */
static s32 entryMod(struct FatEntry *entry) {
    struct Fat *fat = entry->fat;
    /* check for dot entry */
    if (strcmp(entry->name, ".") == 0 || strcmp(entry->name, "..") == 0) {
        errno = EINVAL;
        return -1;
    }
    /* check for root directory */
    if ((entry->attrib & FAT_ATTRIB_DIRECTORY) &&
        (entry->clust < 2 || (fat->type == FAT32 && entry->clust == fat->rootClust))) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

/* rename/move a directory entry. canonical paths must be given for the
   pertinent entry and the directory to which `path` is relative.
   `dir_fp` can be null for root.
   `entry_fp` will be invalidated on success and should not be used.
   the new entry is stored in `*new_entry`, if given. */
s32 fatRename(struct Fat *fat, struct FatPath *entryFp, struct FatPath *dirFp, const char *path,
              struct FatEntry *newEntry) {
    struct FatEntry *entry = fatPathTarget(entryFp);
    /* sanity check */
    if (entryMod(entry)) {
        return -1;
    }
    /* seek destination */
    s32 e = errno;
    errno = 0;
    const char *tail;
    struct FatPath *destFp = fatPath(fat, dirFp, path, &tail);
    if (errno == 0) {
        /* check for no-op */
        struct FatEntry *destEntry = fatPathTarget(destFp);
        if (destEntry->last.clust == entry->last.clust && destEntry->last.pOff == entry->last.pOff) {
            errno = e;
            goto exit;
        } else {
            errno = EEXIST;
            goto error;
        }
    } else {
        if (errno == ENOENT && strlen(tail) > 0 && !strchr(tail, '/') && !strchr(tail, '\\')) {
            errno = e;
        } else {
            goto error;
        }
    }
    /* check for directory recursion */
    if (entry->attrib & FAT_ATTRIB_DIRECTORY) {
        for (struct FatEntry *lEnt = destFp->entList.first; lEnt; lEnt = list_next(lEnt)) {
            if ((lEnt->attrib & FAT_ATTRIB_DIRECTORY) && lEnt->clust == entry->clust) {
                errno = EINVAL;
                goto error;
            }
        }
    }
    /* validate name */
    size_t nameL = nameTrim(tail, strlen(tail));
    if (nameL == 0) {
        errno = EINVAL;
        goto error;
    }
    if (nameL > 255) {
        errno = ENAMETOOLONG;
        goto error;
    }
    char name[256];
    memcpy(name, tail, nameL);
    name[nameL] = 0;
    /* insert new entry, remove old entry */
    if (dirInsert(fat, fatPathTarget(destFp)->clust, name, entry->ctime, entry->cms, entry->atime, entry->mtime,
                  entry->attrib, entry->clust, entry->size, newEntry) ||
        dirRemove(entry)) {
        goto error;
    }
exit:
    fatFree(destFp);
    return 0;
error:
    if (destFp) {
        fatFree(destFp);
    }
    return -1;
}

/* remove a directory entry */
s32 fatRemove(struct FatEntry *entry) {
    struct Fat *fat = entry->fat;
    /* sanity check */
    if (entryMod(entry)) {
        return -1;
    }
    /* do not remove non-empty directories */
    if ((entry->attrib & FAT_ATTRIB_DIRECTORY) && fatEmpty(fat, entry)) {
        return -1;
    }
    /* free cluster chain */
    if (entry->clust >= 2) {
        if (resizeClustChain(fat, entry->clust, 0, 0)) {
            return -1;
        }
    }
    /* remove physical entries */
    if (dirRemove(entry)) {
        return -1;
    }
    return 0;
}

/* modify the attribute of an entry */
s32 fatAttrib(struct FatEntry *entry, u8 attrib) {
    struct Fat *fat = entry->fat;
    /* sanity check */
    if (entryMod(entry)) {
        return -1;
    }
    u8 eType = entry->attrib & (FAT_ATTRIB_DIRECTORY | FAT_ATTRIB_LABEL);
    u8 aType = attrib & (FAT_ATTRIB_DIRECTORY | FAT_ATTRIB_LABEL);
    if (aType != eType) {
        errno = EINVAL;
        return -1;
    }
    /* edit attribute */
    if (fileSect(&entry->last, TRUE)) {
        return -1;
    }
    void *data = fileData(&entry->last);
    if (!data) {
        return -1;
    }
    entry->attrib = attrib;
    setWord(data, 0x0B, 1, attrib);
    cacheDirty(fat, FAT_CACHE_DATA);
    return 0;
}

/* update the access time of an entry */
s32 fatAtime(struct FatEntry *entry, time_t timeval) {
    struct Fat *fat = entry->fat;
    /* sanity check */
    if (entryMod(entry)) {
        return -1;
    }
    if (fileSect(&entry->last, TRUE)) {
        return -1;
    }
    void *data = fileData(&entry->last);
    if (!data) {
        return -1;
    }
    u16 dosAdate;
    entry->atime = timeval;
    unix2dos(entry->atime, &dosAdate, NULL);
    setWord(data, 0x12, 2, dosAdate);
    cacheDirty(fat, FAT_CACHE_DATA);
    return 0;
}

/* update the modified time of an entry */
s32 fatMtime(struct FatEntry *entry, time_t timeval) {
    struct Fat *fat = entry->fat;
    /* sanity check */
    if (entryMod(entry)) {
        return -1;
    }
    if (fileSect(&entry->last, TRUE)) {
        return -1;
    }
    void *data = fileData(&entry->last);
    if (!data) {
        return -1;
    }
    u16 dosMdate;
    u16 dosMtime;
    entry->mtime = timeval;
    unix2dos(entry->mtime, &dosMdate, &dosMtime);
    setWord(data, 0x16, 2, dosMtime);
    setWord(data, 0x18, 2, dosMdate);
    cacheDirty(fat, FAT_CACHE_DATA);
    return 0;
}

/*
   file system operations
*/

static s32 checkRec(struct Fat *fat, u32 recLba, s32 part) {
    if (part >= 4) {
        errno = ENOENT;
        return -1;
    }
    /* load partition record */
    void *pr = cachePrep(fat, FAT_CACHE_DATA, recLba, TRUE);
    if (!pr) {
        return -1;
    }
    /* check signature */
    if (getWord(pr, 0x1FE, 2) != 0xAA55) {
        errno = ENOENT;
        return -1;
    }
    /* check partition type */
    s32 partEntry = 0x1BE + 0x10 * part;
    u8 partType = getWord(pr, partEntry + 0x4, 1);
    if (partType != 0x01 && partType != 0x04 && partType != 0x06 && partType != 0x0E && partType != 0x0B &&
        partType != 0x0C) {
        errno = ENOENT;
        return -1;
    }
    /* get partition address */
    fat->partLba = getWord(pr, partEntry + 0x8, 4);
    fat->nPartSect = getWord(pr, partEntry + 0xC, 4);
    if (fat->partLba == 0 || fat->partLba == recLba || fat->nPartSect == 0) {
        errno = ENOENT;
        return -1;
    }
    return 0;
}

s32 fatInit(struct Fat *fat, FatRdProc read, FatWrProc write, u32 recLba, s32 part) {
    /* initialize cache */
    fat->read = read;
    fat->write = write;
    for (s32 i = 0; i < FAT_CACHE_MAX; ++i) {
        fat->cache[i].valid = FALSE;
        fat->cache[i].maxLba = 0xFFFFFFFF;
    }
    /* check partition record for compatible partition */
    if (checkRec(fat, recLba, part)) {
        /* no partition found, treat as logical volume */
        fat->partLba = 0;
        fat->nPartSect = 0;
    }
    /* load partition boot record */
    void *pbr = cachePrep(fat, FAT_CACHE_DATA, fat->partLba, TRUE);
    if (!pbr) {
        return -1;
    }
    /* get file system geometry info */
    fat->nSectByte = getWord(pbr, 0x00B, 2);
    fat->nClustSect = getWord(pbr, 0x00D, 1);
    fat->nResvSect = getWord(pbr, 0x00E, 2);
    fat->nFat = getWord(pbr, 0x010, 1);
    fat->nEntry = getWord(pbr, 0x011, 2);
    fat->nFsSect = getWord(pbr, 0x013, 2);
    if (fat->nFsSect == 0) {
        fat->nFsSect = getWord(pbr, 0x020, 4);
    }
    if (fat->nFsSect == 0) {
        fat->nFsSect = fat->nPartSect;
    }
    fat->nFatSect = getWord(pbr, 0x016, 2);
    if (fat->nFatSect == 0) {
        fat->nFatSect = getWord(pbr, 0x024, 4);
    }
    /* do sanity checks */
    if (fat->nSectByte != 0x200 || fat->nClustSect == 0 || fat->nResvSect == 0 || fat->nFat == 0 || fat->nFsSect == 0 ||
        (fat->nPartSect != 0 && fat->nFsSect > fat->nPartSect) || fat->nFatSect == 0) {
        errno = ENOENT;
        return -1;
    }
    /* compute addresses and limits */
    fat->fatLba = fat->partLba + fat->nResvSect;
    fat->rootLba = fat->fatLba + fat->nFat * fat->nFatSect;
    fat->dataLba = fat->rootLba + (fat->nEntry * 0x20 + fat->nSectByte - 1) / fat->nSectByte;
    fat->cache[FAT_CACHE_FAT].maxLba = fat->fatLba + fat->nFatSect;
    fat->cache[FAT_CACHE_DATA].maxLba = fat->partLba + fat->nFsSect;
    /* more sanity checks */
    u32 maxLba = fat->partLba + fat->nFsSect;
    if (maxLba < fat->partLba || fat->fatLba < fat->partLba || fat->fatLba >= maxLba || fat->rootLba < fat->partLba ||
        fat->rootLba >= maxLba || fat->dataLba < fat->partLba || fat->dataLba >= maxLba) {
        errno = ENOENT;
        return -1;
    }
    fat->nClustByte = fat->nClustSect * fat->nSectByte;
    fat->maxClust = 2 + (fat->nFsSect - fat->dataLba) / fat->nClustSect;
    fat->freeLb = 2;
    if (fat->maxClust < 0xFF7) {
        fat->type = FAT12;
    } else if (fat->maxClust < 0xFFF7) {
        fat->type = FAT16;
    } else {
        fat->type = FAT32;
        if (fat->maxClust > 0x0FFFFFF7) {
            fat->maxClust = 0x0FFFFFF7;
        }
    }
    u32 nFatClust = fat->nFatSect * fat->nSectByte;
    if (fat->type == FAT12) {
        nFatClust = nFatClust / 3 * 2;
    } else if (fat->type == FAT16) {
        nFatClust /= 2;
    } else {
        nFatClust /= 4;
    }
    if (fat->maxClust > nFatClust) {
        fat->maxClust = nFatClust;
    }
    /* get fat32 info */
    if (fat->type == FAT32) {
        fat->rootClust = getWord(pbr, 0x02C, 4);
        fat->fsisLba = getWord(pbr, 0x030, 2);
    } else {
        fat->rootClust = 0;
        fat->fsisLba = 0;
    }
    /* even more sanity checks */
    if ((fat->type != FAT32 && fat->nEntry == 0) || (fat->type == FAT32 && fat->rootClust < 2)) {
        errno = ENOENT;
        return -1;
    }
    return 0;
}

s32 fatFlush(struct Fat *fat) {
    for (s32 i = 0; i < FAT_CACHE_MAX; ++i) {
        if (cacheFlush(fat, i)) {
            return -1;
        }
    }
    return 0;
}
