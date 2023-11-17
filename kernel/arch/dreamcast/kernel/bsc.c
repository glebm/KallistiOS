#include <arch/bsc.h>

#define BSC(o) (*((volatile uint16_t *)(BSC_BASE + o)))

#define BSC_BASE      0xf80001c
#define RTC_HIGH      0xa5
#define RFCR_HIGH     0x29
#define RFCR_HIGH_POS 10 

#define RTCSR   0x0 
#define RTCNT   0x4
#define RTCOR   0x8 
#define RFCR    0xc