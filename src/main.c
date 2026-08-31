#include "config.h"
#include "delay.h"
#include "gpio.h"
#include "cmp.h"

#include "debug.h"
// #include "mode.h"
// #include "range.h"
// #include "measure.h"


void main(void)
{
    EA = 0;

    // led
    GPIO_INIT(P3, GPIO_Pin_2, GPIO_OUT_PP);
    GPIO_INIT(P3, GPIO_Pin_3, GPIO_OUT_PP);
    GPIO_INIT(P5, GPIO_Pin_5, GPIO_HighZ);
    P32 = 0;
    P33 = 0;

    CMP_INIT(CMP_POSITIVE_P55, CMP_NEGATIVE_BANDGAP, CMP_INTERRUPT_BOTH, CMP_OUTPUT_DISABLE, CMP_OUTPUT_NORMAL, CMP_FILTER_ENABLE, 0);

    debug_init();
    // mode_init();
    // range_init();
    // measure_init();

    EA = 1;

    // switch_range(RANGE_100U_TO_1M);
    // Result r;

    while (true)
    {
        P33 = !P33;
        /* 1 + 15 + 2 clocks per half-period at MAIN_Fosc = 36 MHz. */
        NOP7();
    }
}


void cmp_isr(void) __interrupt(COMPARATOR_VECTOR)
{
    CMP_CLEAR_INTERRUPT_FLAG();
    P32 = !P32;
}
