#include "mode.h"
#include "gpio.h"
#include "exti.h"
#include "pca.h"
#include "range.h"

#define PCA_TIMER_HZ    100UL
#define PCA_TIMER_TICKS ((u16)(MAIN_Fosc / 12UL / PCA_TIMER_HZ)) // PCA_Clock_12T

#define BUTTON_PIN P33

volatile __data enum Mode mode = MODE_AUTO;

static inline void pca_timer_restart(void);
static inline void pca_timer_stop(void);
static enum Mode cycle_mode(void);

void mode_init(void)
{
    GPIO_INIT(P3, GPIO_Pin_3, GPIO_HighZ);
    EXTI1_INIT(EXT_MODE_RiseFall, PriorityLow, ENABLE);

    /* Configure PCA0 as a 16-bit software timer. */
    PCA0_INIT(PCA_Mode_SoftTimer, PCA_PWM_8bit, ENABLE,
              PCA_TIMER_TICKS);
    CR = 0;
    /*
    * Reset and start the shared PCA counter.
    * DISABLE applies only to the counter-overflow interrupt.
    */
    PCA_COUNTER_INIT(PCA_P12_P11_P10_P37, PCA_Clock_12T, DISABLE,
                     PriorityLow);
}

static enum Mode cycle_mode(void)
{
    mode++;

    if (mode >= MODE_COUNT) {
        mode = MODE_AUTO;
    }

    switch (mode) {
    case MODE_10U_TO_100U:
        switch_range(RANGE_10U_TO_100U);
        break;
    case MODE_100U_TO_1M:
        switch_range(RANGE_100U_TO_1M);
        break;
    case MODE_1M_TO_10M:
        switch_range(RANGE_1M_TO_10M);
        break;
    case MODE_10M_TO_1H:
        switch_range(RANGE_10M_TO_1H);
        break;
    case MODE_AUTO:
        auto_range();
        break;
    default:
        break;
    }

    return mode;
}

static inline void pca_timer_stop(void)
{
    CR = 0;
}

static inline void pca_timer_restart(void)
{
    CR = 0;

    CCF0 = 0;
    CF = 0;

    CH = 0;
    CL = 0;

    CCAP0_tmp = PCA_Timer0;
    CCAP0L = (u8)CCAP0_tmp;
    CCAP0H = (u8)(CCAP0_tmp >> 8);

    CR = 1;
}

void int1_isr (void) __interrupt (INT1_VECTOR)		//进中断时已经清除标志
{
    // rising edge only
    if (BUTTON_PIN)
    {
        pca_timer_restart();
    }
}

void pca_isr(void) __interrupt (PCA_VECTOR)
{
    if (CCF0)
    {
        CCF0 = 0;

        /* Schedule the next interrupt 10 ms after the previous deadline. */
        CCAP0_tmp += PCA_Timer0;
        CCAP0L = (u8)CCAP0_tmp;
        CCAP0H = (u8)(CCAP0_tmp >> 8);
        if (BUTTON_PIN)
        {
            // P32 = !P32;
            cycle_mode();
            pca_timer_stop();
        }
    }
}
