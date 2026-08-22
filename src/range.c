#include "range.h"
#include "config.h"
#include "gpio.h"
#include "delay.h"

volatile enum Range range = COMPARATOR_RANGE;

void range_init(void)
{
    GPIO_InitTypeDef        GPIO_InitStructure;

    GPIO_InitStructure.Pin  = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.Mode = GPIO_OUT_PP;
    GPIO_Inilize(GPIO_P1, &GPIO_InitStructure);

    // PMOS & NMOS both off, low excitation voltage
    SW_5V = HIGH;
    SW_DISCHARGE = LOW;
}

enum Range switch_range(enum Range target)
{
    if (range == target)
        return range;

    if (range == COMPARATOR_RANGE)
    {
        // pull up to 5v, turn on pull-up PMOS
        SW_5V = LOW;
        delay_us(CHARGE_TIME);
    }
    else if (target == COMPARATOR_RANGE)
    {
        // pull down to 0.5v
        // turn off pull-up PMOS
        SW_5V = HIGH;
        // prevent shoot through
        delay_us(SHOOT_THROUGH_DEADZONE);
        // turn on discharge path
        SW_DISCHARGE = HIGH;
        // wait for discharge
        delay_us(DISCHARGE_TIME);
        // finally turn off discharge NMOS
        SW_DISCHARGE = LOW;
    }
    range = target;
    return range;
}


enum Range auto_range()
{
    return range;
}