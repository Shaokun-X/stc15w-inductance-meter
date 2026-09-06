#include "config.h"
#include "delay.h"
#include "gpio.h"
#include "debug.h"
#include "measure.h"


void main(void)
{
    EA = 0;

    // led
    GPIO_INIT(P3, GPIO_Pin_2, GPIO_OUT_PP);
    P32 = 0;

    debug_init();
    measure_init();
    log("init \n");

    EA = 1;

    Result r = {0, 0};

    while (true)
    {
        delay_ms(200);
        measure_once(&r);
        log("%d %lu\n", r.status, r.data);
    }
}



