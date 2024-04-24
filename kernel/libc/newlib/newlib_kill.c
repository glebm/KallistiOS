/* KallistiOS ##version##

   newlib_kill.c
   Copyright (C) 2004 Megan Potter

*/

#include <kos/thread.h>

#include <sys/reent.h>

#include <signal.h>
#include <errno.h>

int _kill_r(struct _reent *reent, int pid, int sig) {
    if(getpid() != KOS_PID) {
        reent->_errno = EINVAL;
        return -1;
    }

    printf("KILLING: %d", sig);

    return raise(sig);
}
