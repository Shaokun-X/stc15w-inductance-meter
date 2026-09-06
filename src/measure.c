#include "measure.h"
#include "adc.h"
#include "gpio.h"
#include "pca.h"
#include "timer.h"
#include "cmp.h"
#include "debug.h"

// must match ADC_PIN_PIN
#define ADC_CHANNEL ADC_CH3

// must match ADC_CHANNEL, the PIN needs to be init as high z
#define ADC_PIN_PIN P13
#define ECI_PIN P12

// Timer clock = 36 MHz / 12 = 3 MHz, 60,000 timer ticks = 20ms, 20ms * 5 = 100ms
#define TIMER_VALUE_20MS 5536
#define TIMER_OVERFLOW_COUNT_100MS 5

#define PULSE_COUNT_OVERFLOW_THRESHOLD 100

// must be ADC_RES_H2L8
#define ADC_GET_RESULT() (((unsigned int)(ADC_RES & 0x03) << 8) | ADC_RESL)
#define ADC_CONTR_IDLE_VALUE (0x80 | ADC_90T | ADC_CHANNEL)      /* 0xE1 */
#define ADC_CONTR_START_VALUE (ADC_CONTR_IDLE_VALUE | ADC_START) /* 0xE9 */

#define ADC_CLEAR_FLAG() (ADC_CONTR = ADC_CONTR_IDLE_VALUE)

#define ADC_START_MEASUREMENT() (ADC_CONTR = ADC_CONTR_START_VALUE)

#define ADC_WAIT_FOR_RESULT()                                                                      \
    do                                                                                             \
    {                                                                                              \
    } while (!(ADC_CONTR & ADC_FLAG))
#define TIMER_START()                                                                              \
    do                                                                                             \
    {                                                                                              \
        TH0 = (u8)((TIMER_VALUE_20MS) >> 8);                                                       \
        TL0 = (u8)(TIMER_VALUE_20MS);                                                              \
        TF0 = 0;                                                                                   \
        TR0 = 1;                                                                                   \
    } while (0)
#define TIMER_GET_VALUE(VALUE)                                                                     \
    do                                                                                             \
    {                                                                                              \
        unsigned char timer_high;                                                                  \
        do                                                                                         \
        {                                                                                          \
            timer_high = TH0;                                                                      \
            (VALUE) = TL0;                                                                         \
        } while (timer_high != TH0);                                                               \
        (VALUE) |= (unsigned int)timer_high << 8;                                                  \
    } while (0)
#define TIMER_STOP() (TR0 = 0)

#define PCA_COUNTER_START()                                                                        \
    do                                                                                             \
    {                                                                                              \
        CF = 0;                                                                                    \
        CL = 0;                                                                                    \
        CH = 0;                                                                                    \
        CR = 1;                                                                                    \
    } while (0)
#define PCA_COUNTER_STOP() (CR = 0)

// 100 ms soft timer helper variables
static volatile __data unsigned char timer_overflow_count = 0;
static volatile __data unsigned int pulse_count = 0;
static volatile __data bool timer_flag = false;

void measure_init(void)
{
    // ADC & CCP, must match macro definitions
    GPIO_INIT(P1, GPIO_Pin_1 | GPIO_Pin_3, GPIO_HighZ);
    // CMP positive input
    GPIO_INIT(P5, GPIO_Pin_5, GPIO_HighZ);
    // ECI & CMPO
    GPIO_INIT(P1, GPIO_Pin_2, GPIO_PullUp);

    ADC_INIT(ADC_P11, ADC_90T, ENABLE, ADC_RES_H2L8, DISABLE, PriorityLow);

    TIMER0_INIT(TIM_16BitAutoReload, PriorityHigh, ENABLE, TIM_CLOCK_12T, DISABLE, TIMER_VALUE_20MS,
                DISABLE);
    CMP_INIT(CMP_POSITIVE_P55, CMP_NEGATIVE_BANDGAP, CMP_INTERRUPT_NONE, CMP_OUTPUT_ENABLE,
             CMP_OUTPUT_NORMAL, CMP_FILTER_ENABLE, 0);
    // PCA0_INIT(PCA_Mode_Capture, PCA_PWM_8bit, DISABLE, 0);
    PCA_COUNTER_INIT(PCA_P12_P11_P10_P37, PCA_Clock_ECI, DISABLE, PriorityLow);

    // delay_ms(1);
}

void measure_once(Result *result)
{
    TIMER_START();
    PCA_COUNTER_START();
    pulse_count = 0;
    timer_flag = 0;
    timer_overflow_count = 0;

    while (!timer_flag)
    {
    }

    PCA_COUNTER_STOP();

    // if there is not enough pulse the accuracy worsens
    if (pulse_count <= PULSE_COUNT_OVERFLOW_THRESHOLD)
    {
        result->status = OVERFLOW;
        result->data = 0;
        return;
    }

    if (CF)
    {
        result->status = UNDERFLOW;
        result->data = 0;
        return;
    }

    result->status = OK;
    result->data = pulse_count;
}

// void adc_isr(void) __interrupt(ADC_VECTOR)
// {
//     // log("%d\n", adc_result);
// }

void timer0_isr(void) __interrupt(TIMER0_VECTOR)
{
    timer_overflow_count++;
    if (timer_overflow_count >= TIMER_OVERFLOW_COUNT_100MS)
    {
        pulse_count = CL;
        pulse_count += ((unsigned int)CH << 8);
        TIMER_STOP();
        timer_flag = true;
    }
}

// void cmp_isr(void) __interrupt(COMPARATOR_VECTOR)
// {
//     CMP_CLEAR_INTERRUPT_FLAG();
//     P32 = !P32;
// }
