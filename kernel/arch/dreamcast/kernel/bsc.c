#include <arch/bsc.h>
#include <arch/irq.h>

#include <stdbool.h>

// ROVI - refresh couner overflow interrupt
//RCMI: compare-match interrupt
//interval timer interrupt

#define BSC(o, t)   (*((volatile t *)(BSC_BASE + o)))
#define BSC16(o)    BSC(uint16_t, o)

#define BSC_BASE    0xf80001c

/* Special Function Registers */
#define RTCSR       0x0 /* Refresh Timer Control/Status Register */ 
#define RTCNT       0x4 /* Refresh Timer Counter: Upon matching RTCOR, RTCSR.CMF=>1 && RTCNT=>0*/
#define RTCOR       0x8 /* Refresh Time Constant Register: Comparator register to RTCNT for setting RTCSR.CMF */
#define RFCR        0xc /* Refresh Counter Register: 10-bit counter counting number of matches between RTCOR and RTCNT values. If value > RTCSR.LMTS, RTCSR.OVF is set, RFCR is cleared */

/* High byte magic for SFR writes */
#define RTC_HIGH    0xa5  /* Write data high byte for RTCSR, RTCNT, RTCOR */
#define RFCR_HIGH   0x29  /* Write data high 6 bits for RFCR */

/* RTSCR Fields */
#define CMF         (1 << 7) /* RTCNT == RTCOR: 0=> 0 written to CMF. Writing 1 maintains original value */ 
#define CMIE        (1 << 6) /* Compare-Match Interrupt Enable => Enables interrupts when CMF == 1*/ 
#define CKS         (7 << 3) /* Clock Select Bits */
#define OVF         (1 << 2) /* Refresh Overflow flag: RFCR has overflowed the count limit indicated by LMTS (writing 1 retains value) */
#define OVIE        (1 << 1) /* Refresh Count Overflow Interrupt Enable: Enables interrupts when OVF flag is set to 1 */
#define LMTS        (1 << 0) /* Bit 0—Refresh Count Overflow Limit Select (LMTS): Specifies the count limit to be compared with the refresh count indicated by the refresh count register (RFCR). If the RFCR register value exceeds the value specified by LMTS, the OVF flag is set. 0=>count limit is 1024. 1=>count limit is 512*/

typedef struct rtcsr {
    union {
        uint16_t     bytes;
        struct {
            uint16_t high : 8;
            uint16_t cmf  : 1;
            uint16_t cmie : 1;
            uint16_t cks  : 3;
            uint16_t ovf  : 1;
            uint16_t ovie : 1;
            uint16_t lmts : 1; 
        };
    };
} rtcsr_t;

typedef enum CLK_SELECT {
    CLK_DISABLED,
    CLK_DIV_4,
    CLK_DIV_16,
    CLK_DIV_64,
    CLK_DIV_256,
    CLK_DIV_1024,
    CLK_DIV_2048,
    CLK_DIV_4096
} CLK_SELECT;

/* Interrupt Priority Register access */
#define IPR(o)          (*((volatile uint16_t *)(IPR_BASE + o)))
#define IPR_BASE        0xffd00004          /* Base Address */
#define IPRB            0x4                 /* Interrupt Priority Register B offset */
#define IPRB_REF_POS    8                   /* Mask for IRB WDT IRQ priority field */
#define IPRB_REF        (7 << IPRB_REF_POS) /* IRB WDT IRQ priority field (3 bits) */




static void *compare_match_userdata = NULL;
static bsc_callback_t compare_match_isr = NULL;
static void *overflow_userdata = NULL;
static bsc_callback_t overflow_isr = NULL;

static void bsc_compare_match_isr_wrapper(void) {
    (*compare_match_isr)(compare_match_userdata);
}

static void bsc_overflow_isr_wrapper(void) { 
    (*overflow_isr)(overflow_userdata);
}

#define EXC_REF_RCMI    0x0580  /**< \brief Memory refresh compare-match interrupt */
#define EXC_REF_ROVI    0x05a0  /**< \brief Memory refresh counter overflow interrupt */

void bsc_set_isrs(uint8_t priority, 
                  bsc_callback_t comp_match_callback, void *comp_match_data,
                  bsc_callback_t overflow_callback, void *overflow_data) {

    compare_match_userdata = comp_match_data;
    overflow_userdata      = overflow_data;

    compare_match_isr = comp_match_callback;
    overflow_isr      = overflow_callback;

    /* Register our interrupt handlers */
    irq_set_handler(EXC_REF_RCMI, bsc_compare_match_isr_wrapper);
    irq_set_handler(EXC_REF_ROVI, bsc_overflow_isr_wrapper);
    
    /* Unmask the WDTIT interrupt, giving it a new priority */
    IPR(IPRB) = IPR(IPRB) | ((priority << IPRB_REF_POS) & IPRB_REF);

    const rtcsr_t prev_rtcsr = (rtcsr_t)BSC16(RTCSR);

    BSC16(RTCSR) = *(uint16_t*)&(rtcsr_t) { 
        .high = RTC_HIGH,
        .cmf = false,
        .cmie = true,
        .cks = CLK_DIV_4, 
        .ovf = false,
        .ovie = true,
        .lmts = 1
    };
}



void bsc_init(void) {}

void bsc_shutdown(void) {

}


