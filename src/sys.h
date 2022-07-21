#ifndef SYS_H
#define SYS_H
#include <stddef.h>
#include <sys/stat.h>

typedef void *DIR;

struct Dirent {
    ino_t dIno;
    char dName[256];
    /* extensions */
    mode_t mode;
    time_t ctime;
    time_t mtime;
    off_t size;
};

int open(const char *path, int oflags, ...);
int creat(const char *path, mode_t mode);
int fstat(int fildes, struct stat *buf);
int fstatat(int fildes, const char *path, struct stat *buf, int flag);
int isatty(int fildes);
off_t lseek(int fildes, off_t offset, int whence);
int close(int fildes);
int read(int fildes, void *buf, unsigned int nbyte);
int write(int fildes, void *buf, unsigned int nbyte);
int truncate(const char *path, off_t length);
int rename(const char *oldPath, const char *newPath);
int chmod(const char *path, mode_t mode);
int unlink(const char *path);
DIR *opendir(const char *dirname);
int closedir(DIR *dirp);
struct Dirent *readdir(DIR *dirp);
void seekdir(DIR *dirp, long loc);
long telldir(DIR *dirp);
void rewinddir(DIR *dirp);
int mkdir(const char *path, mode_t mode);
int rmdir(const char *path);
int stat(const char *path, struct stat *buf);
int lstat(const char *path, struct stat *buf);
int chdir(const char *path);
char *getcwd(char *buf, size_t size);
time_t time(time_t *tloc);
void sysReset(void);

#endif
