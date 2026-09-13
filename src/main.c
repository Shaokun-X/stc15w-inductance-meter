#include "config.h"
#include "stc15.h"
#include "gpio.h"
#include "delay.h"
#include "filter.h"
#include "display.h"
#include "format.h"
#include "measure.h"
#include "debug.h"

#define BTN_PIN P33
#define LATCH_PIN P32

#define SHUTDOWN_CONFIRM_LOOP_COUNT 7
#define SHUTDOWN_TRIGGER_LOOP_COUNT 12
// in ms, the period when the button must be released to finally power off
#define SHUTDOWN_BUTTON_RELEASE_WAIT_TIME 600U

enum PowerState
{
    RUNNING,
    CONFIRMING,
    POWER_OFF,
};

__data unsigned int btn_high_loop_count = 0;
__data enum PowerState power_state = RUNNING;

enum PowerState handle_long_press_shutdown(void)
{
    if (BTN_PIN)
    {
        btn_high_loop_count = 0;
        return RUNNING;
    }

    btn_high_loop_count++;

    if (btn_high_loop_count >= SHUTDOWN_CONFIRM_LOOP_COUNT &&
        btn_high_loop_count < SHUTDOWN_TRIGGER_LOOP_COUNT)
    {
        return CONFIRMING;
    }
    if (btn_high_loop_count >= SHUTDOWN_TRIGGER_LOOP_COUNT)
    {
        return POWER_OFF;
    }

    return RUNNING;
}

void main(void)
{
    LATCH_PIN = HIGH;
    GPIO_INIT(P3, GPIO_Pin_2, GPIO_OUT_PP);
    GPIO_INIT(P3, GPIO_Pin_3, GPIO_HighZ);

    EA = 0;

    debug_init();
    measure_init();
    display_init();

    EA = 1;

    Result r = {0, 0};
    KalmanFilter f = {0, 0};

    while (true)
    {
        power_state = handle_long_press_shutdown();
        if (power_state == RUNNING)
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
            else
            {
                display_at_row(1, format_inductance(r.data), 0);
            }
            // display_at_row(1, "Underflow (<10" DISPLAY_MU "H)", 0);
            // log("%d %lu\n", r.status, r.data);
        }
        else if (power_state == CONFIRMING)
        {
            display_at_row(1, "Hold to power off", 0);
            // each loop should take roughly 100ms
            delay_ms(100);
        }
        else
        {
            display_at_row(1, "Powering off...", 0);
            delay_ms(SHUTDOWN_BUTTON_RELEASE_WAIT_TIME);
            display_clear();
            LATCH_PIN = LOW;
            // MCU_POWER_DOWN();
            while (1)
            {
            }
        }

        // WDT_FEED();
    }
}
