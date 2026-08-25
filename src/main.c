#include "config.h"
#include "delay.h"
#include "gpio.h"
#include "stc15.h"

#include "mode.h"
#include "debug.h"
#include "range.h"
#include "measure.h"


void main(void)
{
    EA = 0;

    // led
    GPIO_INIT(P3, GPIO_Pin_2, GPIO_OUT_PP);

    debug_init();
    mode_init();
    range_init();
    measure_init();

    EA = 1;

    switch_range(RANGE_1M_TO_10M);
    Result r;

    while (true)
    {
        delay_ms(500);
        measure_once(&r);
        // P32 = !P32;
        // log("%d\n", r.data >> 8);
        // log("%d\n", r.status);
        // log("test\n");
    }
}


