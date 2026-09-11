#include "config.h"
#include "filter.h"
#include "display.h"
#include "format.h"
#include "measure.h"
#include "debug.h"
#include "wdt.h"

void main(void)
{
    EA = 0;

    // led
    // GPIO_INIT(P3, GPIO_Pin_2, GPIO_OUT_PP);
    // P32 = 0;

    debug_init();
    measure_init();
    display_init();
    WDT_INIT(WDT_STOP_IN_IDLE, WDT_PRESCALER_32);

    EA = 1;

    Result r = {0, 0};
    KalmanFilter f = {0, 0};

    while (true)
    {
        if (r.status != OK)
        {
            f.uncertainty = 0;
            f.prediction = 0;
        }
        measure_with_filter(&r, &f);
        if (r.status == UNDERFLOW)
        {
            display_at_row(1, "Underflow (<10" DISPLAY_MU "H)", 0);
        }
        // else if (r.status == OVERFLOW)
        // {
        //     display_at_row(1, "Overflow", 0);
        // }
        else
        {
            display_at_row(1, format_inductance(r.data), 0);
        }
        // display_at_row(1, "Underflow (<10" DISPLAY_MU "H)", 0);
        // log("%d %lu\n", r.status, r.data);
        WDT_FEED();
    }
}
