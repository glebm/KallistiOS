/* KallistiOS ##version##

   fenv.c
   Copyright (C) 2024 Falco Girgis

*/

#include <fenv.h>


#define FPSCR            __builtin_sh_get_fpscr()
#define FPSCR_SET(value) __builtin_sh_set_fpscr((value))

static fenv_t __fe_dfl_env = 0;
const fenv_t *_fe_dfl_env = &__fe_dfl_env;

static femode_t __fe_dfl_mode = 0;
const femode_t *_fe_dfl_mode = &__fe_dfl_mode;

#define FENV_MODE_MASK  0x3c0000
#define FENV_MODE_SHIFT 18

int fegetmode(femode_t *mode) {
    *mode = (FPSCR & FENV_MODE_MASK) >> FENV_MODE_SHIFT;
}

int fesetmode(const femode_t *mode) {
    FPSCR_SET((*mode << FENV_MODE_SHIFT) & FENV_MODE_MASK);
}

#define FENV_EXCEPT_MASK  0x1f
#define FENV_ENABLE_SHIFT 7

/* GNU Extensions */
int feenableexcept(int excepts) {
    FPSCR_SET(FPSCR | ((excepts & FENV_EXCEPT_MASK) << FENV_ENABLE_SHIFT));
    return 0;
}

int fedisableexcept(int excepts) {
    FPSCR_SET(FPSCR & ~((excepts & FENV_EXCEPT_MASK) << FENV_ENABLE_SHIFT));
    return 0;
}

#define FENV_FLAG_SHIFT 2

int fegetexcept(void) {
    return (FPSCR >> FENV_FLAG_SHIFT) & FENV_EXCEPT_MASK;
}