/* KallistiOS ##version##

   readdir.c
   Copyright (C) 2004 Megan Potter
   Copyright (C) 2024 Falco Girgis
*/

#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include <kos/fs.h>

struct dirent *readdir(DIR *dir) {
    struct stat stat_buf;
    dirent_t *d;
    size_t len;

    if(!dir) {
        errno = EBADF;
        return NULL;
    }

    d = fs_readdir(dir->fd);

    if(!d) {
        /* errno should be set by VFS */
        return NULL;
    }

    if(fstat(dir->fd, &stat_buf) < 0) {
        /* errno should be set by Newlib hook or VFS */
        return NULL;
    }

    len = strnlen(d->name, sizeof(dir->d_name) - 1);
    strncpy(dir->d_ent.d_name, d->name, len);
    dir->d_ent.d_name[len] = '\0';

    dir->d_ent.d_ino = stat_buf.st_ino;
    dir->d_ent.d_off = 0;
    dir->d_ent.d_reclen = sizeof(struct dirent) + len;
    dir->d_ent.d_namlen = len;

    dir->d_ent.d_type = DT_UNKNOWN;
    if(S_ISBLK (stat_buf.st_mode)) dir->d_ent.d_type |= DT_BLK;
    if(S_ISCHR (stat_buf.st_mode)) dir->d_ent.d_type |= DT_CHR;
    if(S_ISDIR (stat_buf.st_mode)) dir->d_ent.d_type |= DT_DIR;
    if(S_ISFIFO(stat_buf.st_mode)) dir->d_ent.d_type |= DT_FIFO;
    if(S_ISREG (stat_buf.st_mode)) dir->d_ent.d_type |= DT_REG;
    if(S_ISLNK (stat_buf.st_mode)) dir->d_ent.d_type |= DT_LNK;
    if(S_ISSOCK(stat_buf.st_mode)) dir->d_ent.d_type |= DT_SOCK;

    return &dir->d_ent;
}
