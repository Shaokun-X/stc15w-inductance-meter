#include "measure.h"
#include "math.h"
#include "adc.h"
#include "clangd_compat.h"
#include "gpio.h"
#include "stc15.h"
#include "timer.h"
#include "delay.h"
#include "debug.h"
#include "range.h"
#include <stdbool.h>

#define ADC_CHANNEL ADC_CH1
// above which adc result we consider that the LR circuit is stable
#define STABLE_VOLTAGE_THRESHOLD 1020

// time below which the inductor must fully charge, other we consider the current range is too low,
// in ms
#define CHARGE_TIMEOUT 2
// time that ensures the full discharge of the inductor, in ms
#define DISCHARGE_DEADZONE 10

#define RANGE_10U_TO_100U_PIN P14
#define RANGE_100U_TO_1M_PIN P15
#define RANGE_1M_TO_10M_PIN P54
#define RANGE_10M_TO_1H_PIN P55
#define ADC_PIN_PIN P11

// must be ADC_RES_H2L8
#define ADC_GET_RESULT() (((unsigned int)(ADC_RES & 0x03) << 8) | ADC_RESL)
#define ADC_CLEAR_FLAG() (ADC_CONTR &= ~ADC_FLAG)
#define ADC_WAIT_FOR_RESULT()                                                                      \
    do                                                                                             \
    {                                                                                              \
    } while (!(ADC_CONTR & ADC_FLAG))
#define TIMER_START()                                                                              \
    do                                                                                             \
    {                                                                                              \
        TL0 = 0;                                                                                   \
        TH0 = 0;                                                                                   \
        TF0 = 0;                                                                                   \
        TR0 = 1;                                                                                   \
    } while (0)
#define TIMER_GET_VALUE() (TL0 | ((unsigned int)TH0 << 8))
#define TIMER_STOP() (TR0 = 0)

#define THRESHOLDS_COUNT 12

static const __code unsigned int RESISTANCE_MAPPING[RANGE_COUNT] = {2, 10, 100, 1000};

// from 0.2 to 0.75, with 0.05 step
static __data unsigned int thresholds[THRESHOLDS_COUNT + 1];
static __data unsigned int stable_voltage;
static __data unsigned int adc_result;
static __data unsigned int voltage_buffer[THRESHOLDS_COUNT];
static unsigned int time_buffer[THRESHOLDS_COUNT];

void measure_init(void)
{
    GPIO_INIT(P1, GPIO_Pin_1, GPIO_HighZ);
    GPIO_INIT(P1, GPIO_Pin_4 | GPIO_Pin_5, GPIO_OUT_PP);
    GPIO_INIT(P5, GPIO_Pin_4 | GPIO_Pin_5, GPIO_OUT_PP);
    RANGE_10U_TO_100U_PIN = LOW;
    RANGE_100U_TO_1M_PIN = LOW;
    RANGE_1M_TO_10M_PIN = LOW;
    RANGE_10M_TO_1H_PIN = LOW;
    ADC_INIT(ADC_P11, ADC_90T, ENABLE, ADC_RES_H2L8, DISABLE, PriorityHigh);
    TIMER0_INIT(TIM_16Bit, PriorityHigh, DISABLE, TIM_CLOCK_1T, DISABLE, 0, DISABLE);
    // delay_ms(1);
}

static inline void excite(void)
{
    switch (range)
    {
    case RANGE_10U_TO_100U:
        RANGE_10U_TO_100U_PIN = HIGH;
        break;
    case RANGE_100U_TO_1M:
        RANGE_100U_TO_1M_PIN = HIGH;
        break;
    case RANGE_1M_TO_10M:
        RANGE_1M_TO_10M_PIN = HIGH;
        break;
    case RANGE_10M_TO_1H:
        RANGE_10M_TO_1H_PIN = HIGH;
        break;
    default:
        break;
    }
}

static inline void deexcite(void)
{
    RANGE_10U_TO_100U_PIN = LOW;
    RANGE_100U_TO_1M_PIN = LOW;
    RANGE_1M_TO_10M_PIN = LOW;
    RANGE_10M_TO_1H_PIN = LOW;
}

static inline void initialize_thresholds(void)
{
    // de-excite and delay to make sure the inductor is fully discharged
    deexcite();
    delay_ms(DISCHARGE_DEADZONE);
    // excite and delay to make sure the inductor is fully charged
    excite();
    delay_ms(CHARGE_TIMEOUT);
    // measure the result detect if it is outbound
    ADC_START_CONVERSION(ADC_CHANNEL);
    ADC_WAIT_FOR_RESULT();
    stable_voltage = ADC_GET_RESULT();
    ADC_CLEAR_FLAG();

    deexcite();
    delay_ms(DISCHARGE_DEADZONE);

    log("stable %d\n", stable_voltage);

    // calculate thresholds
    for (char i = 0; i < THRESHOLDS_COUNT + 1; i++)
    {
        thresholds[i] = (unsigned int)(((unsigned long)stable_voltage * (i + 4) + 10) / 20);
    }
}

static void measure_with_adc(Result *result)
{
    __data unsigned char next_threshold_i = 0;
    __data unsigned int time_point;
    unsigned char i;

    adc_result = 0;
    for (i = 0; i < THRESHOLDS_COUNT; i++)
    {
        voltage_buffer[i] = 0;
        time_buffer[i] = 0;
    }

    excite();
    TIMER_START();
    ADC_START_CONVERSION(ADC_CHANNEL);

    while (TR0 && !TF0 && !voltage_buffer[THRESHOLDS_COUNT - 1])
    {
        ADC_WAIT_FOR_RESULT();
        adc_result = ADC_GET_RESULT();
        time_point = TIMER_GET_VALUE();
        // adc is much slower than this loop body
        ADC_START_CONVERSION(ADC_CHANNEL);
        // log("adc %d\n", adc_result);

        if (adc_result > thresholds[next_threshold_i] && !voltage_buffer[next_threshold_i])
        {
            while (next_threshold_i < THRESHOLDS_COUNT &&
                   adc_result > thresholds[next_threshold_i + 1])
            {
                next_threshold_i++;
            }

            if (next_threshold_i == THRESHOLDS_COUNT)
                break;
            voltage_buffer[next_threshold_i] = adc_result;
            time_buffer[next_threshold_i] = time_point;
            next_threshold_i++;
        }
    }

    TIMER_STOP();
    deexcite();

    // timer overflows, time constant is larger than 2ms, consider out of range
    if (TF0)
    {
        TF0 = 0;
        result->data = 0;
        result->status = OVERFLOW;
        return;
    }

    // if there are less than 2 points, consider underflow
    unsigned char point_count = 0;
    for (i = 0; i < THRESHOLDS_COUNT; i++)
    {
        if (voltage_buffer[i])
        {
            // log("[%d]voltage %d, time %d\n", i, voltage_buffer[i], time_buffer[i]);
            point_count++;
        }
    }
    if (point_count < 2)
    {
        result->data = 0;
        result->status = UNDERFLOW;
        return;
    }

    unsigned long slope_q16 =
        calculate_voltage_slope_q16(time_buffer, voltage_buffer, THRESHOLDS_COUNT, stable_voltage);

    // regression result is 0
    if (slope_q16 == 0)
    {
        result->data = 0;
        result->status = UNDERFLOW;
        return;
    }

    unsigned int resistance =
        (((unsigned long)RESISTANCE_MAPPING[range] << 10) + stable_voltage / 2U) / stable_voltage;

    unsigned long inductance = (slope_q16 >> 16) * resistance;
    inductance += (((slope_q16 & 0xffffUL) * resistance) + 0x8000UL) >> 16;
    inductance = (inductance + MAIN_Fosc / 2000000UL) / (MAIN_Fosc / 1000000UL);

    result->data = inductance;
    result->status = OK;
    log("result %lu\n", result->data);
}

static void measure_with_comparator(Result *result) {}

void measure_once(Result *result)
{
    initialize_thresholds();
    if (range == COMPARATOR_RANGE)
    {
        measure_with_comparator(result);
    }
    else
    {
        measure_with_adc(result);
    }
    delay_ms(DISCHARGE_DEADZONE);
}

// void adc_isr(void) __interrupt(ADC_VECTOR)
// {
//     // log("%d\n", adc_result);
// }

// void timer0_isr(void) __interrupt(TIMER0_VECTOR)
// {
//     log("isr %d\n", TF0);
//     TR0 = 0;
// }
