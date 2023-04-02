/* KallistiOS ##version##

   timespec_get.c
   Copyright (C) 2023 Lawrence Sebald
*/

#include <time.h>
#include <arch/timer.h>
#include <arch/rtc.h>

int timespec_get(struct timespec *ts, int base) {
    if(base == TIME_UTC && ts) {
        uint32 s, ns;

        timer_ns_gettime(&s, &ns);
        ts->tv_sec = rtc_boot_time() + s;
        ts->tv_nsec = ns;

        return base;
    }

    return 0;
}
