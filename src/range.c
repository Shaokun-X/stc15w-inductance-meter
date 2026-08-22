#include "range.h"
#include "config.h"
#include "gpio.h"
#include "delay.h"

#define SW_5V P12
#define SW_DISCHARGE P13
#define COMPARATOR_RANGE RANGE_10U_TO_100U
// pessimistic time needed for the capacitor bank to fully discharge to low excitation voltage, in us
#define DISCHARGE_TIME 100
// time needed for the source to charge up the capacitors, in us
#define CHARGE_TIME 100
// delay time before switching 2 MOSFETs on the same path, in us
#define SHOOT_THROUGH_DEADZONE 20

volatile enum Range range = COMPARATOR_RANGE;

void range_init(void)
{
    // PMOS & NMOS both off, low excitation voltage
    SW_5V = HIGH;
    SW_DISCHARGE = LOW;

    GPIO_InitTypeDef        GPIO_InitStructure;

    GPIO_InitStructure.Pin  = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.Mode = GPIO_OUT_PP;
    GPIO_Inilize(GPIO_P1, &GPIO_InitStructure);
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