#include "config.h"
#include "delay.h"
#include "gpio.h"
#include "adc.h"
#include "timer.h"

#include "mode.h"
#include "debug.h"
#include "range.h"

#define ADC_CHANNEL ADC_CH1


volatile u16 adc_result;
volatile bool adc_result_ready;

void main(void)
{
    EA = 0;

    // led
    GPIO_INIT(P3, GPIO_Pin_2, GPIO_OUT_PP);

    // adc
    GPIO_INIT(P1, GPIO_Pin_1, GPIO_HighZ);

    ADC_INIT(ADC_P11, ADC_90T, DISABLE, ADC_RES_H2L8, ENABLE,
             PriorityLow);

    // timer
    TIMER0_INIT(TIM_16BitAutoReload, PriorityLow, DISABLE,
                TIM_CLOCK_1T, DISABLE, 0, DISABLE);

    mode_init();
    range_init();
    debug_init();

    EA = 1;
    ADC_PowerControl(ENABLE);
    delay_ms(2);
    ADC_StartConversion(ADC_CH1);

    TR0 = 1;
    
    while (true)
    {
        delay_ms(500);
        // P32 = !P32;
        // log("%d\n", adc_result);
        log("%u\n", ((unsigned int)TH0 << 8) | TL0);
        ADC_StartConversion(ADC_CH1);
    }
}

void adc_isr (void) __interrupt (ADC_VECTOR)
{
    // must be ADC_RES_H2L8
    adc_result = ((u16)(ADC_RES & 0x03) << 8) | ADC_RESL;
    ADC_CONTR &= ~ADC_FLAG;
    adc_result_ready = true;
}
