#include "config.h"
#include "filter.h"
#include "gpio.h"
#include "debug.h"
#include "measure.h"


void main(void)
{
    EA = 0;

    // led
    // GPIO_INIT(P3, GPIO_Pin_2, GPIO_OUT_PP);
    // P32 = 0;

    debug_init();
    measure_init();

    EA = 1;

    Result r = {0, 0};
    KalmanFilter f = {0, 0};

    while (true)
    {
        if (r.status != OK) {
            f.uncertainty = 0;
            f.prediction = 0;
        }
        measure_once_with_filter(&r, &f);
        log("%d %lu\n", r.status, r.data);
    }
}


