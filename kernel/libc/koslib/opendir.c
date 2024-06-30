/* KallistiOS ##version##

   opendir.c
   Copyright (C) 2004 Megan Potter

*/

#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <kos/fs.h>

DIR *opendir(const char * name) {
    file_t fd = fs_open(name, O_DIR | O_RDONLY);

    if(fd < 0) {
        // VFS will set errno
        return NULL;
    }

    return fdopendir(fd);
}

DIR *fdopendir(int fd) {
    DIR *newd;

    // Ok, got it. Alloc a struct to return.
    newd = calloc(1, sizeof(DIR));

    if(!newd) {
        errno = ENOMEM;
        return NULL;
    }

    newd->fd = fd;

    return newd;
}
