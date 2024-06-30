/* KallistiOS ##version##

   openat.c
   Copyright (C) 2024 Falco Girgis
*/

#include <kos/fs.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>

/* 
    1) If pathname is relative, it's relative to dirfd
        a. if dirfd is AT_FDCWD it's relative to working dir
    2) if pathname is absolute, ignore dirfd
*/

static inline bool is_relative(const char* pathname) {
    return pathname[0] != '/';
}

int openat(int dirfd, const char *path_name, int flags, ...) {
    char new_path[PATH_MAX] = { '\0' };
    va_list args;
    mode_t mode;

    va_start(args, flags);
    mode = va_arg(args, mode_t);
    va_end(args);

    if(is_relative(path_name)) {
        if(dirfd == AT_FDCWD)
            strcat(new_path, fs_getwd());
        else
            assert(false);

        strcat(new_path, "/");
    }

    strcat(new_path, path_name);

	return open(new_path, flags, mode);
}