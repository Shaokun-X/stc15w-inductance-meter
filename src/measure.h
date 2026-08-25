#ifndef	__MEASURE_H
#define	__MEASURE_H

#include "config.h"

enum ResultStatus {
    OK,
    OVERFLOW,
    UNDERFLOW,
};

typedef struct {
    unsigned long data;
    enum ResultStatus status;
} Result;

void measure_init(void);
void measure_once(Result *result);

// void adc_isr (void) __interrupt (ADC_VECTOR);
// void timer0_isr(void) __interrupt(TIMER0_VECTOR);

#endif
