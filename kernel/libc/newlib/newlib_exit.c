/* KallistiOS ##version##

   newlib_exit.c
   Copyright (C)2004 Megan Potter

*/

#include <arch/arch.h>
#include <stdbool.h>

extern void arch_exit_handler(int ret_code) __noreturn;

static int ret_code;

void kos_shutdown(void)
{
    arch_exit_handler(ret_code);

    __builtin_unreachable();
}

KOS_INIT_FLAG_WEAK(kos_shutdown, true);

void _exit(int code) {
    ret_code = code;

    KOS_INIT_FLAG_CALL(kos_shutdown);
}
