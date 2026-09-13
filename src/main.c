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

// measure_with_filter() takes approximately 100 ms per measurement
#define AUTO_SHUTDOWN_UNCHANGED_LOOP_COUNT 1800U // 3min
#define AUTO_SHUTDOWN_CONFIRM_LOOP_COUNT 50U // 5s

enum PowerState
{
    RUNNING,
    CONFIRMING,
    POWER_OFF,
};

enum AutoShutdownState
{
    AUTO_SHUTDOWN_TRACKING,
    AUTO_SHUTDOWN_WARNING,
    AUTO_SHUTDOWN_POWER_OFF,
};

static __data Result r = {0, 0};
static __data KalmanFilter f = {0, 0};
static __data Result previous_result = {0, 0};
static __data unsigned int btn_high_loop_count = 0;
static __data enum PowerState power_state = RUNNING;
static __data unsigned int unchanged_loop_count = 0;
static __data unsigned char auto_shutdown_confirm_loop_count = 0;
static __data bool has_previous_measurement = false;
static __data enum AutoShutdownState auto_shutdown_state = AUTO_SHUTDOWN_TRACKING;

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

static void reset_auto_shutdown(void)
{
    unchanged_loop_count = 0;
    auto_shutdown_confirm_loop_count = 0;
    auto_shutdown_state = AUTO_SHUTDOWN_TRACKING;
}

static void cancel_auto_shutdown_on_button_press(void)
{
    if (auto_shutdown_state == AUTO_SHUTDOWN_WARNING && !BTN_PIN)
    {
        reset_auto_shutdown();
    }
}

static void update_auto_shutdown(const Result *result)
{
    bool measurement_changed;

    measurement_changed = !has_previous_measurement || result->status != previous_result.status ||
                          result->data != previous_result.data;
    previous_result = *result;
    has_previous_measurement = true;

    if (measurement_changed)
    {
        reset_auto_shutdown();
    }
    else if (auto_shutdown_state == AUTO_SHUTDOWN_WARNING)
    {
        auto_shutdown_confirm_loop_count++;
        if (auto_shutdown_confirm_loop_count >= AUTO_SHUTDOWN_CONFIRM_LOOP_COUNT)
        {
            auto_shutdown_state = AUTO_SHUTDOWN_POWER_OFF;
        }
    }
    else
    {
        unchanged_loop_count++;
        if (unchanged_loop_count >= AUTO_SHUTDOWN_UNCHANGED_LOOP_COUNT)
        {
            auto_shutdown_confirm_loop_count = 0;
            auto_shutdown_state = AUTO_SHUTDOWN_WARNING;
        }
    }
}

static void power_off(void)
{
    display_at_row(1, "Powering off...", 0);
    delay_ms(SHUTDOWN_BUTTON_RELEASE_WAIT_TIME);
    display_clear();
    LATCH_PIN = LOW;
    while (1)
    {
    }
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

    while (true)
    {
        power_state = handle_long_press_shutdown();
        cancel_auto_shutdown_on_button_press();
        if (auto_shutdown_state == AUTO_SHUTDOWN_POWER_OFF)
        {
            power_state = POWER_OFF;
        }

        if (power_state == POWER_OFF)
        {
            power_off();
        }
        else if (power_state == CONFIRMING)
        {
            display_at_row(1, "Hold to power off", 0);
            // each loop should take roughly 100ms
            delay_ms(100);
        }
        else
        {
            if (r.status != OK)
            {
                f.uncertainty = 0;
                f.prediction = 0;
            }
            measure_with_filter(&r, &f);
            update_auto_shutdown(&r);

            if (auto_shutdown_state != AUTO_SHUTDOWN_TRACKING)
            {
                display_at_row(1, "Power off in 5s", 0);
            }
            else if (r.status == UNDERFLOW)
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
    }
}
