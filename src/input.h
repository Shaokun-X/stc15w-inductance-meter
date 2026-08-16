#ifndef	__INPUT_H
#define	__INPUT_H

#include "config.h"
#include "gpio.h"
#include "exti.h"
#include "pca.h"

void int1_isr (void) __interrupt (INT1_VECTOR);
void pca_isr(void) __interrupt (PCA_VECTOR);

void input_init(void);

enum Range {
    RANGE_BELOW_10U, // outbound
    RANGE_10U_TO_100U, // comparator range
    RANGE_100U_TO_1M,
    RANGE_1M_TO_10M,
    RANGE_10M_TO_100M,
    RANGE_100M_TO_1H,
    RANGE_ABOVE_1H, // outbound
    RANGE_COUNT,
};

enum Mode {
    MODE_AUTO,
    MODE_10U_TO_100U,
    MODE_100U_TO_1M,
    MODE_1M_TO_10M,
    MODE_10M_TO_100M,
    MODE_100M_TO_1H,
    MODE_COUNT,
};

extern volatile enum Mode mode;

#define PCA_TIMER_HZ    100UL
#define PCA_TIMER_TICKS ((u16)(MAIN_Fosc / 12UL / PCA_TIMER_HZ)) // PCA_Clock_12T

#define BUTTON_PIN P33

#endif