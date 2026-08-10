#include "STC15Fxxxx.h"
#include "config.h"
#include "delay.h"
#include "GPIO.h"

void main(void)
{

    GPIO_InitTypeDef        GPIO_InitStructure;
    GPIO_InitStructure.Pin  = GPIO_Pin_2;
    GPIO_InitStructure.Mode = GPIO_OUT_PP;
    GPIO_Inilize(GPIO_P3, &GPIO_InitStructure);

    while (true)
    {
        delay_ms(500);
        P32 = !P32;
    }

}
