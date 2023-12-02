#include <arch/bsc.h>
#include <arch/irq.h>

#include <stdbool.h>

// ROVI - refresh couner overflow interrupt
//RCMI: compare-match interrupt
//interval timer interrupt

#if 0
Bit 2—Refresh Control (RFSH): Specifies refresh control. Selects whether refreshing is
performed for DRAM and synchronous DRAM. When the refresh function is no
#endif

#define BSC(o)      (*((volatile uint16_t *)(BSC_BASE + o)))

#define BSC_BASE    0xff80001c

/* Special Function Registers */
#define RTCSR       0x0 /* Refresh Timer Control/Status Register */ 
#define RTCNT       0x4 /* Refresh Timer Counter: Upon matching RTCOR, RTCSR.CMF=>1 && RTCNT=>0*/
#define RTCOR       0x8 /* Refresh Time Constant Register: Comparator register to RTCNT for setting RTCSR.CMF */
#define RFCR        0xc /* Refresh Counter Register: 10-bit counter counting number of matches between RTCOR and RTCNT values. If value > RTCSR.LMTS, RTCSR.OVF is set, RFCR is cleared */

/* High byte magic for SFR writes */
#define RTC_MAGIC    0xa500  /* Write data high byte for RTCSR, RTCNT, RTCOR */
#define RFCR_MAGIC   0xa400  /* Write data high 6 bits for RFCR */

/* RTSCR Fields */
#define CMF         (1 << 7) /* RTCNT == RTCOR: 0=> 0 written to CMF. Writing 1 maintains original value */ 
#define CMIE        (1 << 6) /* Compare-Match Interrupt Enable => Enables interrupts when CMF == 1*/ 
#define CKS         (7 << 3) /* Clock Select Bits */
#define OVF         (1 << 2) /* Refresh Overflow flag: RFCR has overflowed the count limit indicated by LMTS (writing 1 retains value) */
#define OVIE        (1 << 1) /* Refresh Count Overflow Interrupt Enable: Enables interrupts when OVF flag is set to 1 */
#define LMTS        (1 << 0) /* Bit 0—Refresh Count Overflow Limit Select (LMTS): Specifies the count limit to be compared with the refresh count indicated by the refresh count register (RFCR). If the RFCR register value exceeds the value specified by LMTS, the OVF flag is set. 0=>count limit is 1024. 1=>count limit is 512*/

typedef enum CLK_SELECT {
    CLK_DISABLED    = (0 << 3),
    CLK_DIV_4       = (1 << 3),
    CLK_DIV_16      = (2 << 3),
    CLK_DIV_64      = (3 << 3),
    CLK_DIV_256     = (4 << 3),
    CLK_DIV_1024    = (5 << 3),
    CLK_DIV_2048    = (6 << 3),
    CLK_DIV_4096    = (7 << 3)
} CLK_SELECT;

#if 0
DC BIOS default values
RTCSR: 94
RTCNT: 5d
RTCOR: 5e
RFCR: 232
#endif

/* Interrupt Priority Register access */
#define IPR(o)          (*((volatile uint16_t *)(IPR_BASE + o)))
#define IPR_BASE        0xffd00004          /* Base Address */
#define IPRB            0x4                 /* Interrupt Priority Register B offset */
#define IPRB_REF_BIT    8                   /* Mask for IRB WDT IRQ priority field */
#define IPRB_REF        (0xf << IPRB_REF_BIT) /* IRB WDT IRQ priority field (3 bits) */


static void *compare_match_userdata = NULL;
static bsc_callback_t compare_match_isr = NULL;
static void *overflow_userdata = NULL;
static bsc_callback_t overflow_isr = NULL;

static void bsc_compare_match_isr_wrapper(irq_t, irq_context_t *) {
    compare_match_isr(compare_match_userdata);
    BSC(RTCSR) = RTC_MAGIC | (BSC(RTCSR) & ~CMF);
}

static void bsc_overflow_isr_wrapper(irq_t, irq_context_t *) { 
    overflow_isr(overflow_userdata);
    BSC(RTCSR) = RTC_MAGIC | (BSC(RTCSR) & ~OVF);
}

void bsc_set_isrs(uint8_t priority, 
                  bsc_callback_t comp_match_callback, void *comp_match_data,
                  bsc_callback_t overflow_callback, void *overflow_data) {

    printf("Initializing!\n");

    printf("RTCSR: %x\n", BSC(RTCSR));
    printf("RTCNT: %x\n", BSC(RTCNT));
    printf("RTCOR: %x\n", BSC(RTCOR));
    printf("RFCR: %x\n", BSC(RFCR));

    compare_match_userdata = comp_match_data;
    overflow_userdata      = overflow_data;

    compare_match_isr = comp_match_callback;
    overflow_isr      = overflow_callback;

    /* Register our interrupt handlers */
    irq_set_handler(EXC_REF_RCMI, bsc_compare_match_isr_wrapper);
    irq_set_handler(EXC_REF_ROVI, bsc_overflow_isr_wrapper);

    BSC(RTCNT) = RTC_MAGIC;
    BSC(RTCOR) = RTC_MAGIC | 255;
    BSC(RFCR)  = RFCR_MAGIC;

    printf("Set RTCNT, RTCOR, RFCR\n");

    /* Unmask the WDTIT interrupt, giving it a new priority */
    IPR(IPRB) = IPR(IPRB) | ((priority & 0xf) << IPRB_REF_BIT);

    printf("Set IPRB!\n");

#if 1
    BSC(RTCSR) = RTC_MAGIC |
                    OVIE | CMIE |
                    CLK_DIV_4;
#else 
    BSC(RTCSR) = rtcsr.bytes;
#endif

for(unsigned i = 0; i < 500; ++i) { 
    printf("RTCSR: %x\n", BSC(RTCSR));
    printf("RTCNT: %x\n", BSC(RTCNT));
   // printf("RTCOR: %x\n", BSC(RTCOR));
    printf("RFCR: %x\n", BSC(RFCR));
}
}



void bsc_init(void) {}

void bsc_shutdown(void) {
    BSC(RTCSR) = RTC_MAGIC | CLK_DISABLED;
    IPR(IPRB) = 0;
    irq_set_handler(EXC_REF_RCMI, NULL);
    irq_set_handler(EXC_REF_ROVI, NULL);
    BSC(RTCNT) = RTC_MAGIC;
    BSC(RFCR)  = RFCR_MAGIC;
}


