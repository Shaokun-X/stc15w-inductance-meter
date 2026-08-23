#include "config.h"
#include "delay.h"
#include "gpio.h"
#include "stc15.h"
#include "timer.h"

#include "mode.h"
#include "debug.h"
#include "range.h"
#include "measure.h"


void main(void)
{
    EA = 0;

    // led
    GPIO_INIT(P3, GPIO_Pin_2, GPIO_OUT_PP);
    P32 = 1;

    

    debug_init();
    mode_init();
    range_init();
    measure_init();

    EA = 1;
    TR0 = 1;

    
    while (true)
    {
        delay_ms(500);
        // P32 = !P32;
        log("%d\n", TF0);
        // TR0 = 1;
        log("%d\n", TF0);
        log("%u\n", ((unsigned int)TH0 << 8) | TL0);
        // TF0 = 0;
    }
}


