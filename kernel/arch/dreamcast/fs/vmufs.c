/* KallistiOS ##version##

   vmufs.c
   Copyright (C) 2003 Megan Potter
   Copyright (C) 2023 Falco Girgis

*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <kos/mutex.h>
#include <kos/numeric.h>
#include <dc/vmufs.h>
#include <dc/maple.h>
#include <dc/maple/vmu.h>

/*

This is a whole new module that sits between the fs_vmu module and the maple
VMU driver. It's based loosely on the stuff in the old fs_vmu, but it's been
rewritten and reworked to be clearer, more clean, use threads better, etc.

Unlike the fs_vmu module, this code is stateless. You make a call and you get
back data (or have written it). There are no handles involved or anything
else like that. The new fs_vmu sits on top of this and provides a (mostly)
nice VFS interface similar to the old fs_vmu.

This module tends to do more work than it really needs to for some
functions (like reading a named file) but it does it that way to have very
clear, concise code that can be audited for bugs more easily. It's not
like you load and save on the VMU every frame or something. ;) But
the user may never give your program another frame of time if it corrupts
their save games! If you want better control to save loading and saving
stuff for a big batch of changes, then use the low-level funcs.

Function comments located in vmufs.h.

*/


/* ****************** Low level functions ******************** */


/* We need some sort of access control here for threads. This is somewhat
   less than optimal (one mutex for all VMUs) but I doubt it'll really
   be much of an issue :) */
static mutex_t mutex = MUTEX_INITIALIZER;

static void vmufs_timestamp_to_tm(const vmu_timestamp_t *timestamp, struct tm *bt) {
    tm.tm_year = bcd_to_dec(timestamp->cent - 0x19) * 100 +
                 bcd_to_dec(timestamp->year) + 100;
    tm.tm_mon  = bcd_to_dec(timestamp->mon) - 1;
    tm.tm_mday = bcd_to_dec(timestamp->day);
    tm.tm_hour = bcd_to_dec(timestamp->hour);
    tm.tm_min  = bcd_to_dec(timestamp->min);
    tm.tm_sec  = bcd_to_dec(timestamp->sec);
    tm.tm_wday = (bcd_to_dec(timestamp->dow) + 1) % 7;
}

static void vmufs_timestamp_from_tm(vmu_timestamp_t *timestamp, const struct tm *bt) {
    timestamp->cent  = bcd_from_dec(tm.tm_year / 100) + 0x19;
    timestamp->year  = bcd_from_dec(tm.tm_year - 100);
    timestamp->month = bcd_from_dec(tm.tm_mon + 1);
    timestamp->day   = bcd_from_dec(tm.tm_mday);
    timestamp->hour  = bcd_from_dec(tm.tm_hour);
    timestamp->min   = bcd_from_dec(tm.tm_min);
    timestamp->sec   = bcd_from_dec(tm.tm_sec);
    timestamp->dow   = bcd_from_dec((tm.tm_wday - 1) % 7);
}

time_t vmufs_timestamp_to_unux(const vmu_timestamp_t *timestamp) { 
    struct tm bt;

    vmufs_timestamp_to_tm(timestamp, &bt);

    return mktime(&bt);
}

int vmufs_timestamp_from_unix(vmu_timestamp_t *timestamp, time_t unix) { 
    struct tm bt;

    if(!localtime_r(&unix, &bt))
        return -1;

    vmufs_timestamp_from_tm(timestamp, &bt);

    return 0;
}

time_t vmufs_dir_fill_time(vmu_dir_t *d) {
    time_t t;
    struct tm tm;

    /* Get the time */
    if((t = time(NULL)) == -1)
        return -1;

    if(!localtime_r(&t, &tm))
        return -1;

    /* Fill in the struct, converting to BCD */
    vmufs_timestamp_from_tm(&d->timestamp, &tm);

    return t;
}

int vmufs_root_read(maple_device_t* dev, vmu_root_t* root_buf) {
    const vmu_media_info_t* media_info = NULL;

    if(vmu_media_info(dev, &media_info) != 0) {
        dbglog(DBG_ERROR, "vmufs_root_read: can't get media info on device %c%c\n",
               dev->port + 'A', dev->unit + '0');
        return -1;
    }

    /* XXX: Assume root is at 255.. is there some way to figure this out dynamically? */
    if(vmu_block_read(dev, media_info->root_loc, (uint8 *)root_buf) != 0) {
        dbglog(DBG_ERROR, "vmufs_root_read: can't read block %d on device %c%c\n",
               media_info->root_loc, dev->port + 'A', dev->unit + '0');
        return -1;
    }

    return 0;
}

int vmufs_root_write(maple_device_t *dev, const vmu_root_t *root_buf) {
    const vmu_media_info_t* media_info = NULL;

    if(vmu_media_info(dev, &media_info) != 0) {
        dbglog(DBG_ERROR, "vmufs_root_write: can't get media info on device %c%c\n",
               dev->port + 'A', dev->unit + '0');
        return -1;
    }

    if(vmu_block_write(dev, media_info->root_loc, (uint8 *)root_buf) != 0) {
        dbglog(DBG_ERROR, "vmufs_root_write: can't write block %d on device %c%c\n",
               media_info->root_loc, dev->port + 'A', dev->unit + '0');
        return -1;
    }
    else
        return 0;
}

size_t vmufs_dir_bytes(maple_device_t *dev, const vmu_root_t *root_buf) {
    const vmu_storage_info_t* storage_info = NULL;

    if(vmu_storage_info(dev, &storage_info) != 0) {
        dbglog(DBG_ERROR, "vmufs_dir_bytes: can't get storage info on device %c%c\n",
               dev->port + 'A', dev->unit + '0');
        return 0;
    }

    return root_buf->dir_size * storage_info->block_size * 32;
}

size_t vmufs_fat_bytes(const vmu_root_t *root_buf) {
    const vmu_storage_info_t* storage_info = NULL;

    if(vmu_storage_info(dev, &storage_info) != 0) {
        dbglog(DBG_ERROR, "vmufs_fat_bytes: can't get storage info on device %c%c\n",
               dev->port + 'A', dev->unit + '0');
        return 0;
    }

    return root_buf->fat_size * storage_info->block_size * 32;
}

/* Common code for both dir_read and dir_write */
static int vmufs_dir_ops(maple_device_t *dev, vmu_root_t *root, vmu_dir_t *dir_buf, int write) {
    uint16_t dir_block, dir_size;
    unsigned int i;
    int needsop, rv;
    const vmu_storage_info_t* storage_info = NULL;

    /* Find the directory starting block and length */
    dir_block = root->dir_loc;
    dir_size = root->dir_size;

    if(vmu_storage_info(dev, &storage_info) != 0) {
        dbglog(DBG_ERROR, "vmufs_dir_ops: can't get storage info on device %c%c\n",
               dev->port + 'A', dev->unit + '0');
        return -1;
    }

    /* The dir is stored backwards, so we start at the end and go back. */
    while(dir_size > 0) {
        if(write) {
            /* Scan this block for changes */
            for(i = 0, needsop = 0; i < storage_info->block_size * 32 / sizeof(vmu_dir_t); i++) {
                if(dir_buf[i].dirty) {
                    needsop = 1;
                }

                dir_buf[i].dirty = 0;
            }
        }
        else
            needsop = 1;

        if(needsop) {
            if(!write)
                rv = vmu_block_read(dev, dir_block, (uint8_t *)dir_buf);
            else
                rv = vmu_block_write(dev, dir_block, (uint8_t *)dir_buf);

            if(rv != 0) {
                dbglog(DBG_ERROR, "vmufs_dir_%s: can't %s block %d on device %c%c\n",
                       write ? "write" : "read",
                       write ? "write" : "read",
                       (int)dir_block, dev->port + 'A', dev->unit + '0');
                return -1;
            }
        }

        dir_block--;
        dir_size--;
        dir_buf += storage_info->block_size * 32 / sizeof(vmu_dir_t); /* == 16 */
    }

    return 0;
}

int vmufs_dir_read(maple_device_t *dev, const vmu_root_t *root, vmu_dir_t *dir_buf) {
    return vmufs_dir_ops(dev, root, dir_buf, 0);
}

int vmufs_dir_write(maple_device_t *dev, vmu_root_t *root, vmu_dir_t *dir_buf) {
    return vmufs_dir_ops(dev, root, dir_buf, 1);
}

/* Common code for both fat_read and fat_write */
static int vmufs_fat_ops(maple_device_t *dev, const vmu_root_t *root, vmu_block_t *fat_buf, int write) {
    vmu_block_t fat_block, fat_size;
    int rv;

    /* Find the FAT starting block and length */
    fat_block = root->fat_loc;
    fat_size = root->fat_size;

    /* We can't reliably handle VMUs with a larger FAT... */
    if(fat_size > 1) {
        dbglog(DBG_ERROR, "vmufs_fat_%s: VMU has >1 (%d) FAT blocks on device %c%c\n",
               write ? "write" : "read",
               (int)fat_size, dev->port + 'A', dev->unit + '0');
        return -1;
    }

    if(!write)
        rv = vmu_block_read(dev, fat_block, (uint8 *)fat_buf);
    else
        rv = vmu_block_write(dev, fat_block, (uint8 *)fat_buf);

    if(rv != 0) {
        dbglog(DBG_ERROR, "vmufs_fat_%s: can't %s block %d on device %c%c (error %d)\n",
               write ? "write" : "read",
               write ? "write" : "read",
               (int)fat_block, dev->port + 'A', dev->unit + '0', rv);
        return -2;
    }

    return 0;
}

int vmufs_fat_read(maple_device_t *dev, const vmu_root_t *root, vmu_block_t *fat_buf) {
    return vmufs_fat_ops(dev, root, fat_buf, 0);
}

int vmufs_fat_write(maple_device_t *dev, const vmu_root_t *root, const vmu_block_t* fat_buf) {
    return vmufs_fat_ops(dev, root, (vmu_block_t *)fat_buf, 1);
}

int vmufs_dir_find(maple_device_t *dev, const vmu_root_t *root, vmu_dir_t *dir, const char *fn) {
    int i;
    int dcnt;

    const vmu_storage_info_t* storage_info = NULL;

    if(vmu_storage_info(dev, &storage_info) != 0) {
        dbglog(DBG_ERROR, "vmufs_dir_find: can't get storage info on device %c%c\n",
               dev->port + 'A', dev->unit + '0');
        return -2;
    }

    dcnt = root->dir_size * storage_info->block_size * 32 / sizeof(vmu_dir_t);

    for(i = 0; i < dcnt; i++) {
        /* Not a file -> skip it */
        if(dir[i].filetype == VMU_FILE_NONE)
            continue;

        /* Check the filename */
        if(!strncmp(fn, dir[i].filename, VMU_FILENAME_SIZE))
            return i;
    }

    /* Didn't find anything */
    return -1;
}

// is this order right?
int vmufs_dir_add(maple_device_t *dev, const vmu_root_t *root, vmu_dir_t *dir, const vmu_dir_t *newdirent) {
    int i;
    int dcnt;

    const vmu_storage_info_t* storage_info = NULL;

    if(vmu_storage_info(dev, &storage_info) != 0) {
        dbglog(DBG_ERROR, "vmufs_dir_add: can't get storage info on device %c%c\n",
               dev->port + 'A', dev->unit + '0');
        return -2;
    }

    dcnt = root->dir_size * storage_info->block_size * 32 / sizeof(vmu_dir_t);

    for(i = 0; i < dcnt; i++) {
        /* A file -> skip it */
        if(dir[i].filetype != VMU_FILE_NONE)
            continue;

        /* Copy in the entry */
        memcpy(dir + i, newdirent, sizeof(vmu_dir_t));

        /* Set this entry dirty so its dir block will get written out */
        dir[i].dirty = 1;

        return 0;
    }

    /* Didn't find any open spaces */
    return -1;
}

int vmufs_file_read(maple_device_t *dev, const vmu_block_t *fat, const vmu_dir_t *dirent, void *outbuf) {
    vmu_block_t curblk, blkleft, 
    int         rv;
    uint8_t     *out;

    const vmu_storage_info_t* storage_info = NULL;

    if(vmu_storage_info(dev, &storage_info) != 0) {
        dbglog(DBG_ERROR, "vmufs_file_read: can't get storage info on device %c%c\n",
               dev->port + 'A', dev->unit + '0');
        return -4;
    }

    out = (uint8_t *)outbuf;

    /* Find the first block */
    curblk = dirent->firstblk;

    /* And the blocks remaining */
    blkleft = dirent->filesize;

    /* While we've got stuff remaining... */
    while(blkleft > 0) {
        /* Make sure the FAT matches up with the directory */
        if(curblk == VMUFS_FAT_UNALLOCATED || curblk == VMUFS_FAT_LAST_IN_FILE) {
            char fn[VMU_FILENAME_SIZE + 1] = { 0 };
            memcpy(fn, dirent->filename, VMU_FILENAME_SIZE);
            dbglog(DBG_ERROR, "vmufs_file_read: file '%s' ends prematurely in fat on device %c%c\n",
                   fn, dev->port + 'A', dev->unit + '0');
            return -1;
        }

        /* Read the block */
        rv = vmu_block_read(dev, curblk, (uint8_t *)out);

        if(rv != 0) {
            dbglog(DBG_ERROR, "vmufs_file_read: can't read block %d on device %c%c (error %d)\n",
                   curblk, dev->port + 'A', dev->unit + '0', rv);
            return -2;
        }

        /* Scoot our counters */
        curblk = fat[curblk];
        blkleft--;
        out += storage_info->block_size * 32;
    }

    /* Make sure the FAT matches up with the directory */
    if(curblk != VMUFS_FAT_LAST_IN_FILE) {
        char fn[VMU_FILENAME_SIZE + 1] = { 0 };
        memcpy(fn, dirent->filename, VMU_FILENAME_SIZE);
        dbglog(DBG_ERROR, "vmufs_file_read: file '%s' is sized shorter than in the FAT on device %c%c\n",
               fn, dev->port + 'A', dev->unit + '0');
        return -3;
    }

    return 0;
}

void vmu_media_info_user_region(const vmu_media_info_t *info, vmu_block_t *user_loc, vmu_block_t *user_size) { 
    if(info->hidden_size)
        *user_loc  = info->hidden_loc - info->hidden_size;
    else
        *user_loc = info->dir_loc - info->dir_size;

     *user_size = *user_loc - 0; // first block
}

/* Find an open block for writing in the FAT */
static int vmufs_find_block(maple_device_t *dev, const vmu_root_t *root, const vmu_block_t *fat, const vmu_dir_t *dirent) {
    int i;
    vmu_block_t user_loc, user_size;
    size_t fat_entries;
    const vmu_storage_info_t* storage_info = NULL;

    if(vmu_storage_info(dev, &storage_info) != 0) {
        dbglog(DBG_ERROR, "vmufs_find_block: can't get storage info on device %c%c\n",
               dev->port + 'A', dev->unit + '0');
        return -3;
    }

    vmu_media_info_user_region(&root->media_info, &user_loc, &user_size);

    if(dirent->filetype == VMU_FILE_DATA) {
        /* Data files -- count down from top */
        for(i = user_loc - 1; i >= 0; i--) {
            if(fat[i] == VMUFS_FAT_UNALLOCATED)
                return i;
        }
    }
    else if(dirent->filetype == VMU_FILE_GAME) {
        /* VMU games -- count up from bottom */
        for(i = 0; i < user_loc; i++) {
            if(fat[i] == VMUFS_FAT_UNALLOCATED)
                return i;
        }
    }
    else {
        /* Dunno what this is! */
        char fn[VMU_FILENAME_SIZE + 1] = { 0 };
        memcpy(fn, dirent->filename, VMU_FILENAME_SIZE);
        dbglog(DBG_ERROR, "vmufs_find_block: file '%s' has unknown type %d\n", fn, dirent->filetype);
        return -1;
    }

    /* No free blocks left */
    {
        char fn[VMU_FILENAME_SIZE + 1] = { 0 };
        memcpy(fn, dirent->filename, VMU_FILENAME_SIZE);
        dbglog(DBG_ERROR, "vmufs_find_block: can't find any more free blocks for file '%s'\n", fn);
    }
    return -2;
}






int vmufs_file_write(maple_device_t * dev, vmu_root_t * root, uint16 * fat,
                     vmu_dir_t * dir, vmu_dir_t * newdirent, void * filebuf, int size) {
    vmu_block_t curblk, blkleft;
    int         rv, vmuspaceleft;
    uint8       *out;

    /* Files must be at least one block long */
    if(size <= 0) {
        char fn[VMU_FILENAME_SIZE + 1] = {0};
        memcpy(fn, newdirent->filename, VMU_FILENAME_SIZE);
        dbglog(DBG_ERROR, "vmufs_file_write: file '%s' is too short (%d blocks)\n", fn, size);
        return -3;
    }

    /* Make sure this file isn't already in the directory */
    if(vmufs_dir_find(root, dir, newdirent->filename) >= 0) {
        char fn[VMU_FILENAME_SIZE + 1] = {0};
        memcpy(fn, newdirent->filename, VMU_FILENAME_SIZE);
        dbglog(DBG_ERROR, "vmufs_file_write: file '%s' is already in the dir on device %c%c\n",
               fn, dev->port + 'A', dev->unit + '0');
        return -4;
    }

    out = (uint8 *)filebuf;

    /* Don't even start if there isn't enough room to write the whole file */
    vmuspaceleft = vmufs_fat_free(root, fat);

    if(vmuspaceleft < size) {
        dbglog(DBG_INFO, "vmufs_file_write: not enough space for file. Need %d blocks, have %d\n", size, vmuspaceleft);
        return -2;  /* Same error as is returned if a block can not be found below */
    }

    /* Find ourselves an open slot for the first block */
    curblk = newdirent->firstblk = vmufs_find_block(root, fat, newdirent);

    if(curblk < 0)
        return curblk;

    /* And the blocks remaining */
    blkleft = newdirent->filesize = size;

    /* While we've got stuff remaining... */
    while(blkleft > 0) {
        /* Write the block */
        rv = vmu_block_write(dev, curblk, (uint8 *)out);

        if(rv != 0) {
            dbglog(DBG_ERROR, "vmufs_file_write: can't write block %d on device %c%c (error %d)\n",
                   curblk, dev->port + 'A', dev->unit + '0', rv);
            return -5;
        }

        /* Scoot our counters */
        blkleft--;
        out += VMU_BLOCK_SIZE;

        /* If we have blocks left, find another free block. Otherwise,
           write out a terminator. */
        if(blkleft) {
            // Set the pointer to the terminator just in case:
            // a) vmufs_find_block() fails to find a block, AND
            // b) the calling code for some reason writes the FAT back out anyway.
            // This may render the save game unusable but at least we won't link
            // into some other file (or worse, a game!)
            fat[curblk] = VMUFS_FAT_LAST_IN_FILE;
            rv = vmufs_find_block(root, fat, newdirent);

            if(rv < 0)
                return rv;

            fat[curblk] = rv;
            curblk = rv;
        }
        else {
            fat[curblk] = VMUFS_FAT_LAST_IN_FILE;
        }
    }

    /* Add the entry to the directory */
    if(vmufs_dir_add(root, dir, newdirent) < 0) {
        dbglog(DBG_ERROR, "vmufs_file_write: can't find an open dirent on device %c%c\n",
               dev->port + 'A', dev->unit + '0');
        return -6;
    }

    return 0;
}

int vmufs_file_delete(vmu_root_t *root, vmu_block_t *fat, vmu_dir_t *dir, const char *fn) {
    ssize_t idx;
    vmu_block_t blk, nextblk;

    /* Find the file */
    idx = vmufs_dir_find(root, dir, fn);

    if(idx < 0) {
        dbglog(DBG_ERROR, "vmufs_file_delete: can't find file '%s'\n", fn);
        return -1;
    }

    /* Find its first block, and go through clearing FAT blocks. */
    blk = dir[idx].firstblk;

    while(blk != VMUFS_FAT_LAST_IN_FILE) {
        if(blk == VMUFS_FAT_UNALLOCATED || blk > root->blk_cnt) {
            dbglog(DBG_ERROR, "vmufs_file_delete: inconsistency -- corrupt FAT or dir\n");
            return -2;
        }

        /* Free it */
        nextblk = fat[blk];
        fat[blk] = VMUFS_FAT_UNALLOCATED;

        /* Move to the next one */
        blk = nextblk;
    }

    /* Now clear out its dirent also */
    memset(dir + idx, 0, sizeof(vmu_dir_t));

    /* Set it dirty so it'll be flushed out */
    dir[idx].dirty = 1;

    return 0;
}

/* hee hee :) */
size_t vmufs_fat_free(const vmu_root_t *root, const vmu_block_t *fat) {
    size_t i, freeblocks;

    freeblocks = 0;

    for(i = 0; i < root->blk_cnt; i++) { /* only count user blocks */
        if(fat[i] == VMUFS_FAT_UNALLOCATED)
            freeblocks++;
    }

    return freeblocks;
}

size_t vmufs_dir_free(vmu_root_t * root, vmu_dir_t * dir) {
    unsigned int i;
    int freeblocks;

    freeblocks = 0;

    for(i = 0; i < root->dir_size * VMU_BLOCK_SIZE / sizeof(vmu_dir_t); i++) {
        if(dir[i].filetype == VMU_FILE_NONE)
            freeblocks++;
    }

    return freeblocks;
}

int vmufs_mutex_lock(void) {
    return mutex_lock(&mutex);
}

int vmufs_mutex_unlock(void) {
    return mutex_unlock(&mutex);
}

/* ****************** Higher level functions ******************** */

/* Internal function gets everything setup for you */
static int vmufs_setup(maple_device_t * dev, vmu_root_t * root, vmu_dir_t ** dir, int * dirsize,
                       uint16 ** fat, int * fatsize) {
    /* Check to make sure this is a valid device right now */
    if(!dev || !(dev->info.functions & MAPLE_FUNC_MEMCARD)) {
        if(!dev)
            dbglog(DBG_ERROR, "vmufs_setup: device is invalid\n");
        else
            dbglog(DBG_ERROR, "vmufs_setup: device %c%c is not a memory card\n",
                   dev->port + 'A', dev->unit + '0');

        return -1;
    }

    vmufs_mutex_lock();

    /* Read its root block */
    if(!root || vmufs_root_read(dev, root) < 0)
        goto dead;

    if(dir) {
        /* Alloc enough space for the whole dir */
        *dirsize = vmufs_dir_blocks(root);
        *dir = (vmu_dir_t *)malloc(*dirsize);

        if(!*dir) {
            dbglog(DBG_ERROR, "vmufs_setup: can't alloc %d bytes for dir on device %c%c\n",
                   *dirsize, dev->port + 'A', dev->unit + '0');
            goto dead;
        }

        /* Read it */
        if(vmufs_dir_read(dev, root, *dir) < 0) {
            free(*dir);
            *dir = NULL;
            goto dead;
        }
    }

    if(fat) {
        /* Alloc enough space for the fat */
        *fatsize = vmufs_fat_blocks(root);
        *fat = (uint16 *)malloc(*fatsize);

        if(!*fat) {
            dbglog(DBG_ERROR, "vmufs_setup: can't alloc %d bytes for FAT on device %c%c\n",
                   *fatsize, dev->port + 'A', dev->unit + '0');
            if(dir)
                free(*dir);
            goto dead;
        }

        /* Read it */
        if(vmufs_fat_read(dev, root, *fat) < 0) {
            free(*fat);
            if(dir)
                free(*dir);
            goto dead;
        }
    }

    /* Ok, everything's cool */
    return 0;

dead:
    vmufs_mutex_unlock();
    return -1;
}

/* Internal function to tear everything down for you */
static void vmufs_teardown(vmu_dir_t * dir, uint16 * fat) {
    if(dir)
        free(dir);

    if(fat)
        free(fat);

    vmufs_mutex_unlock();
}

int vmufs_readdir(maple_device_t * dev, vmu_dir_t ** outbuf, int * outcnt) {
    vmu_root_t root;
    vmu_dir_t *dir;
    int dircnt, dirsize, rv = 0;
    unsigned int i, j;

    *outbuf = NULL;
    *outcnt = 0;

    /* Init everything */
    if(vmufs_setup(dev, &root, &dir, &dirsize, NULL, NULL) < 0)
        return -1;

    /* Go through and move all entries to the lowest-numbered spots. */
    dircnt = 0;

    for(i = 0; i < dirsize / sizeof(vmu_dir_t); i++) {
        /* Skip blanks */
        if(dir[i].filetype == VMU_FILE_NONE)
            continue;

        /* Not a blank -- look for an earlier slot that's empty. If
           we don't find one, just leave it alone. */
        for(j = 0; j < i; j++) {
            if(dir[j].filetype == VMU_FILE_NONE) {
                memcpy(dir + j, dir + i, sizeof(vmu_dir_t));
                dir[i].filetype = VMU_FILE_NONE;
                break;
            }
        }

        /* Update the entry count */
        dircnt++;
    }

    /* Resize the buffer to match the number of entries */
    *outcnt = dircnt;
    *outbuf = (vmu_dir_t *)realloc(dir, dircnt * sizeof(vmu_dir_t));

    if(!*outbuf && dircnt) {
        dbglog(DBG_ERROR, "vmufs_readdir: can't realloc %d bytes for dir on device %c%c\n",
               dircnt * sizeof(vmu_dir_t), dev->port + 'A', dev->unit + '0');
        free(dir);
        rv = -2;
        goto ex;
    }

ex:
    vmufs_teardown(NULL, NULL);
    return rv;
}

/* Shared code between read/read_dirent */
static int vmufs_read_common(maple_device_t * dev, vmu_dir_t * dirent, uint16 * fat, void ** outbuf, int * outsize) {
    /* Allocate the output space */
    *outsize = dirent->filesize * VMU_FILENAME_SIZE;
    *outbuf = malloc(*outsize);

    if(!*outbuf) {
        dbglog(DBG_ERROR, "vmufs_read: can't alloc %d bytes for reading a file  on device %c%c\n",
               *outsize, dev->port + 'A', dev->unit + '0');
        return -1;
    }

    /* Ok, go ahead and read it */
    if(vmufs_file_read(dev, fat, dirent, *outbuf) < 0) {
        free(*outbuf);
        *outbuf = NULL;
        *outsize = 0;
        return -1;
    }

    return 0;
}

int vmufs_read(maple_device_t *dev, const char *fn, void **outbuf, size_t* outsize) {
    vmu_root_t  root;
    vmu_dir_t   *dir = NULL;
    vmu_block_t *fat = NULL;
    int     fatsize, dirsize, idx, rv = 0;

    *outbuf = NULL;
    *outsize = 0;

    /* Init everything */
    if(vmufs_setup(dev, &root, &dir, &dirsize, &fat, &fatsize) < 0)
        return -1;

    /* Look for the file we want */
    idx = vmufs_dir_find(&root, dir, fn);

    if(idx < 0) {
        //dbglog(DBG_ERROR, "vmufs_read: can't find file '%s' on device %c%c\n",
        //  fn, dev->port+'A', dev->unit+'0');
        rv = -2;
        goto ex;
    }

    if(vmufs_read_common(dev, dir + idx, fat, outbuf, outsize) < 0) {
        rv = -3;
        goto ex;
    }

ex:
    vmufs_teardown(dir, fat);
    return rv;
}

int vmufs_read_dirent(maple_device_t * dev, vmu_dir_t * dirent, void ** outbuf, int * outsize) {
    vmu_root_t  root;
    uint16      * fat = NULL;
    int     fatsize, rv = 0;

    *outbuf = NULL;
    *outsize = 0;

    /* Init everything */
    if(vmufs_setup(dev, &root, NULL, NULL, &fat, &fatsize) < 0)
        return -1;

    if(vmufs_read_common(dev, dirent, fat, outbuf, outsize) < 0)
        rv = -2;

    vmufs_teardown(NULL, fat);
    return rv;
}

/* Returns 0 for success, -7 for 'not enough space', and other values for other errors. :-)  */
int vmufs_write(maple_device_t *dev, const char *fn, const void *inbuf, size_t insize, unsigned flags) {
    vmu_root_t      root;
    vmu_dir_t       *dir = NULL, nd;
    vmu_fat_block_t *fat = NULL;
    int             oldinsize, fatsize, dirsize, idx, rv = 0, st, fnlength;

    /* Round up the size if necessary */
    oldinsize = insize;
    insize = round_up_to_pow2(insize, VMU_BLOCK_SIZE);

    if(insize == 0) insize = VMU_BLOCK_SIZE;

    if(oldinsize != insize) {
        dbglog(DBG_WARNING, "vmufs_write: padded file '%s' from %d to %d bytes\n",
               fn, oldinsize, insize);
    }

    /* Init everything */
    if(vmufs_setup(dev, &root, &dir, &dirsize, &fat, &fatsize) < 0)
        return -1;

    /* Check if the file already exists */
    idx = vmufs_dir_find(&root, dir, fn);

    if(idx >= 0) {
        if(!(flags & VMUFS_OVERWRITE)) {
            dbglog(DBG_ERROR, "vmufs_write: file '%s' already exists on device %c%c\n",
                   fn, dev->port + 'A', dev->unit + '0');
            rv = -2;
            goto ex;
        }
        else {
            if(vmufs_file_delete(&root, fat, dir, fn) < 0) {
                dbglog(DBG_ERROR, "vmufs_write: can't delete old file '%s' on device %c%c\n",
                       fn, dev->port + 'A', dev->unit + '0');
                rv = -3;
                goto ex;
            }
        }
    }

    /* Fill out a new dirent for this file */
    memset(&nd, 0, sizeof(nd));
    nd.filetype = (flags & VMUFS_VMUGAME) ? VMU_FILE_GAME : VMU_FILE_DATA;
    nd.copyprotect = (flags & VMUFS_NOCOPY) ? VMU_FILE_PROTECTED : VMU_FILE_COPYABLE;
    nd.firstblk = 0;

    fnlength = strlen(fn);
    fnlength = fnlength > VMU_FILENAME_SIZE ? VMU_FILENAME_SIZE : fnlength;
    memcpy(nd.filename, fn, fnlength);
    if (fnlength < VMU_FILENAME_SIZE) {
        memset(nd.filename + fnlength, '\0', VMU_FILENAME_SIZE - fnlength);
    }

    vmufs_dir_fill_time(&nd);
    nd.filesize = insize / VMU_BLOCK_SIZE;
    nd.hdroff = (flags & VMUFS_VMUGAME) ? 1 : 0;
    nd.dirty = 1;

    // If any of these fail, the action to take can be decided by the caller.

    /* Write out the data and update our structs */
    if((st = vmufs_file_write(dev, &root, fat, dir, &nd, inbuf, insize / VMU_BLOCK_SIZE)) < 0) {
        if(st == -2)
            rv = -7;
        else
            rv = -4;
        goto ex;
    }

    /* Ok, everything's looking good so far.. update the FAT */
    if(vmufs_fat_write(dev, &root, fat) < 0) {
        rv = -5;
        goto ex;
    }

    /* This is the critical point. If the dir doesn't save correctly, then
       we may have an unusable card (until it's reformatted) or leaked
       blocks not attached to a file. Cross your fingers! */
    if(vmufs_dir_write(dev, &root, dir) < 0) {
        /* doh! */
        dbglog(DBG_ERROR, "vmufs_write: warning, card may be corrupted or leaking blocks!\n");
        rv = -6;
        goto ex;
    }

    /* Looks like everything was good */
ex:
    vmufs_teardown(dir, fat);
    return rv;
}

int vmufs_delete(maple_device_t * dev, const char * fn) {
    vmu_root_t  root;
    vmu_dir_t   * dir = NULL;
    uint16      * fat = NULL;
    int     fatsize, dirsize, rv = 0;

    /* Init everything */
    if(vmufs_setup(dev, &root, &dir, &dirsize, &fat, &fatsize) < 0)
        return -2;

    /* Ok, try to delete the file */
    rv = vmufs_file_delete(&root, fat, dir, fn);

    if(rv < 0) goto ex;

    /* If we succeeded, write back the dir and fat */
    if(vmufs_dir_write(dev, &root, dir) < 0) {
        rv = -2;
        goto ex;
    }

    /* This is the critical point. If the fat doesn't save correctly, then
       we may have an unusable card (until it's reformatted) or leaked
       blocks not attached to a file. Cross your fingers! */
    if(vmufs_fat_write(dev, &root, fat) < 0) {
        /* doh! */
        dbglog(DBG_ERROR, "vmufs_delete: warning, card may be corrupted or leaking blocks!\n");
        rv = -2;
        goto ex;
    }

    /* Looks like everything was good */
ex:
    vmufs_teardown(dir, fat);
    return rv;
}

int vmufs_free_blocks(maple_device_t * dev) {
    vmu_root_t  root;
    uint16      * fat = NULL;
    int     fatsize, rv = 0;

    /* Init everything */
    if(vmufs_setup(dev, &root, NULL, NULL, &fat, &fatsize) < 0)
        return -1;

    rv = vmufs_fat_free(&root, fat);

    vmufs_teardown(NULL, fat);
    return rv;
}




int vmufs_init(void) {
    mutex_init(&mutex, MUTEX_TYPE_NORMAL);
    return 0;
}

int vmufs_shutdown(void) {
    mutex_destroy(&mutex);
    return 0;
}
