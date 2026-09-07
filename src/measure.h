#ifndef	__MEASURE_H
#define	__MEASURE_H

#include "config.h"
#include "filter.h"

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
void measure_once_with_filter(Result *result, KalmanFilter *filter);

void timer0_isr(void) __interrupt(TIMER0_VECTOR);
// void adc_isr (void) __interrupt (ADC_VECTOR);
// void cmp_isr(void) __interrupt(COMPARATOR_VECTOR);

#endif
