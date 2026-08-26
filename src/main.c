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
    // GPIO_INIT(P3, GPIO_Pin_2, GPIO_OUT_PP);
    // P32 = 0;

    debug_init();
    mode_init();
    range_init();
    measure_init();

    EA = 1;

    switch_range(RANGE_100U_TO_1M);
    Result r;

    while (true)
    {
        delay_ms(500);
        measure_once(&r);
        // P32 = !P32;
        log("result %lu, status %d\n", r.data, r.status);
        // log("%d\n", r.status);
        // log("test\n");
    }
}


