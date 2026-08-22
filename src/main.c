#include "config.h"
#include "delay.h"
#include "gpio.h"
#include "adc.h"
// #include "timer.h"

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
    GPIO_InitTypeDef        GPIO_InitStructure;
    GPIO_InitStructure.Pin  = GPIO_Pin_2;
    GPIO_InitStructure.Mode = GPIO_OUT_PP;
    GPIO_Init(GPIO_P3, &GPIO_InitStructure);

    // adc
    GPIO_InitStructure.Pin  = GPIO_Pin_1;
    GPIO_InitStructure.Mode = GPIO_HighZ;
    GPIO_Init(GPIO_P1, &GPIO_InitStructure);

    ADC_InitTypeDef         ADC_InitStructure;              //结构定义
    ADC_InitStructure.ADC_Speed     = ADC_90T;               //设置 ADC 工作时钟频率 ADC_SPEED_2X1T~ADC_SPEED_2X16T
    ADC_InitStructure.ADC_Px        = ADC_P11;
    ADC_InitStructure.ADC_AdjResult = ADC_RES_H2L8;  //ADC结果调整,  ADC_LEFT_JUSTIFIED,ADC_RIGHT_JUSTIFIED
    ADC_InitStructure.ADC_Interrupt = ENABLE;
    ADC_InitStructure.ADC_Priority    = PriorityLow;
    ADC_InitStructure.ADC_Power     = DISABLE;
    ADC_Init(&ADC_InitStructure);                //初始化

    // timer
    // TIM_InitTypeDef     TIM_InitStructure;
    // TIM_InitStructure.TIM_Mode      = TIM_16BitAutoReload;
    // TIM_InitStructure.TIM_Interrupt = DISABLE;
    // TIM_InitStructure.TIM_Value     = 0;
    // TIM_InitStructure.TIM_ClkSource = TIM_CLOCK_1T;
    // TIM_InitStructure.TIM_ClkOut = DISABLE;
    // TIM_InitStructure.TIM_Run = DISABLE;
    // Timer_Init(Timer0, &TIM_InitStructure);

    // mode_init();
    // range_init();
    debug_init();

    EA = 1;
    ADC_PowerControl(ENABLE);
    delay_ms(2);
    ADC_StartConversion(ADC_CH1);

    // TR0 = 1;
    
    while (true)
    {
        delay_ms(500);
        // P32 = !P32;
        log("%d\n", adc_result);
        // log("%u\n", ((unsigned int)TH0 << 8) | TL0);
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

